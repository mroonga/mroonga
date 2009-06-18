#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "driver.h"

/* TLS variables */
__thread grn_ctx *mrn_ctx_tls;


/* static variables */
grn_hash *mrn_hash_sys;
grn_obj *mrn_db_sys, *mrn_lexicon_sys;
pthread_mutex_t *mrn_mutex_sys;
const char *mrn_logfile_name=MRN_LOG_FILE_NAME;
FILE *mrn_logfile = NULL;
uint mrn_ctx_counter = 0;

grn_logger_info mrn_logger_info = {
  GRN_LOG_DUMP,
  GRN_LOG_TIME|GRN_LOG_MESSAGE,
  mrn_logger_func,
  NULL
};

/* additional functions */
int mrn_flush_logs()
{
  MRN_TRACE;
  pthread_mutex_lock(mrn_mutex_sys);
  MRN_LOG(GRN_LOG_NOTICE, "logfile closed by FLUSH LOGS");
  fflush(mrn_logfile);
  fclose(mrn_logfile); /* reopen logfile for rotation */
  mrn_logfile = fopen(mrn_logfile_name, "a");
  MRN_LOG(GRN_LOG_NOTICE, "-------------------------------");
  MRN_LOG(GRN_LOG_NOTICE, "logfile re-opened by FLUSH LOGS");
  pthread_mutex_unlock(mrn_mutex_sys);
  return 0;
}

int mrn_init()
{
  grn_ctx ctx;

  /* libgroonga init */
  if (grn_init() != GRN_SUCCESS) {
    return -1;
  }
  grn_ctx_init(ctx,0);

  /* log init */
  if (!(mrn_logfile = fopen(mrn_logfile_name, "a"))) {
    return -1;
  }
  grn_logger_info_set(mrn_ctx_tls, &mrn_logger_info);
  GRN_LOG(ctx, GRN_LOG_NOTICE, "++++++ starting mroonga ++++++");

  /* init meta-data repository */
  mrn_hash_sys = grn_hash_create(mrn_ctx_tls,NULL,
				 MRN_MAX_KEY_LEN,sizeof(size_t),
				 GRN_OBJ_KEY_VAR_SIZE);
  mrn_db_sys = mrn_db_open_or_create();

  /* mutex init */
  mrn_mutex_sys = (pthread_mutex_t*) MRN_MALLOC(sizeof(pthread_mutex_t));
  // TODO: FIX THIS 
  //pthread_mutex_init(mrn_mutex_sys, PTHREAD_MUTEX_INITIALIZER);

  grn_ctx_fin(ctx);
  return 0;
}

/*
  TODO: release all grn_obj in global hash
*/
int mrn_deinit()
{
  mrn_ctx_init();
  MRN_TRACE;

  MRN_LOG(GRN_LOG_DEBUG, "-> grn_obj_close: '%s'", MRN_DB_FILE_PATH);
  grn_obj_close(mrn_ctx_tls, mrn_db_sys);

  /* mutex deinit*/
  pthread_mutex_destroy(mrn_mutex_sys);
  MRN_FREE(mrn_mutex_sys);

  /* log deinit */
  MRN_LOG(GRN_LOG_NOTICE, "------ stopping mroonga ------");
  fclose(mrn_logfile);
  mrn_logfile = NULL;

  /* hash deinit */
  grn_hash_close(mrn_ctx_tls, mrn_hash_sys);

  /* libgroonga deinit */
  grn_fin();

  return 0;
}

void mrn_logger_func(int level, const char *time, const char *title,
		     const char *msg, const char *location, void *func_arg)
{
  const char slev[] = " EACewnid-";
  if ((mrn_logfile)) {
    fprintf(mrn_logfile, "%s|%c|%u|%s\n", time,
	    *(slev + level), (uint)pthread_self(), msg);
    fflush(mrn_logfile);
  }
}


void mrn_ctx_init()
{
  if (mrn_ctx_tls == NULL) {
    mrn_ctx_tls = (grn_ctx*) MRN_MALLOC(sizeof(grn_ctx));
    grn_ctx_init(mrn_ctx_tls, 0);
    if ((mrn_db_sys))
      grn_ctx_use(mrn_ctx_tls, mrn_db_sys);
  }
}

grn_obj *mrn_db_open_or_create()
{
  grn_obj *obj;
  struct stat dummy;
  if ((stat(MRN_DB_FILE_PATH, &dummy))) { // check if file not exists
    MRN_LOG(GRN_LOG_DEBUG, "-> grn_db_create: '%s'", MRN_DB_FILE_PATH);
    obj = grn_db_create(mrn_ctx_tls, MRN_DB_FILE_PATH, NULL);
    /* create global lexicon table */
    mrn_lexicon_sys = grn_table_create(mrn_ctx_tls, "lexicon", 7, MRN_LEXICON_FILE_PATH,
				       GRN_OBJ_TABLE_PAT_KEY|GRN_OBJ_PERSISTENT, 
				       grn_ctx_at(mrn_ctx_tls,GRN_DB_SHORTTEXT), 0);
    grn_obj_set_info(mrn_ctx_tls, mrn_lexicon_sys, GRN_INFO_DEFAULT_TOKENIZER,
		     grn_ctx_at(mrn_ctx_tls, GRN_DB_BIGRAM));
  } else {
    MRN_LOG(GRN_LOG_DEBUG, "-> grn_db_open: '%s'", MRN_DB_FILE_PATH);
    obj = grn_db_open(mrn_ctx_tls, MRN_DB_FILE_PATH);
    /* open global lexicon table */
    mrn_lexicon_sys = grn_table_open(mrn_ctx_tls, "lexicon", 7, MRN_LEXICON_FILE_PATH);
  }
  return obj;
}

void mrn_share_put(mrn_share *share)
{
  void *value;
  grn_search_flags flags = GRN_TABLE_ADD;
  /* TODO: check duplication */
  MRN_LOG(GRN_LOG_DEBUG,"-> grn_hash_lookup(put): name='%s'", share->name);
  grn_hash_lookup(mrn_ctx_tls, mrn_hash_sys, share->name,
		  strlen(share->name), &value, &flags);
  memcpy(value, share, sizeof(share));
}

/* returns NULL if specified obj_name is not found in grn_hash */
mrn_share *mrn_share_get(const char *name)
{
  void *value;
  grn_search_flags flags = 0;
  MRN_LOG(GRN_LOG_DEBUG,"-> grn_hash_lookup(get): name='%s'", name);
  grn_id rid = grn_hash_lookup(mrn_ctx_tls, mrn_hash_sys, name,
			       strlen(name), &value, &flags);
  if (rid == 0) {
    return NULL;
  } else {
    return (mrn_share*) value;
  }
}

void mrn_share_remove(mrn_share *share)
{
  /* TODO: check return value */
  MRN_LOG(GRN_LOG_DEBUG, "-> grn_hash_delete: name='%s'", share->name);
  grn_hash_delete(mrn_ctx_tls, mrn_hash_sys, share->name,
		  strlen(share->name), NULL);
}

void mrn_share_remove_all()
{
  /* TODO: implement this function by using GRN_HASH_EACH */
}

/*
#define LOG_FIELD(x) MRN_LOG(GRN_LOG_DEBUG, "-> %s %s", field->field_name, x); break;

void mrn_print_field_type(Field *field)
{
  switch (field->type()) {
  case MYSQL_TYPE_DECIMAL:
    LOG_FIELD("MYSQL_TYPE_DECIMAL");
  case MYSQL_TYPE_TINY:
    LOG_FIELD("MYSQL_TYPE_TINY");
  case MYSQL_TYPE_SHORT:
    LOG_FIELD("MYSQL_TYPE_SHORT");
  case MYSQL_TYPE_LONG:
    LOG_FIELD("MYSQL_TYPE_LONG");
  case MYSQL_TYPE_FLOAT:
    LOG_FIELD("MYSQL_TYPE_FLOAT");
  case MYSQL_TYPE_DOUBLE:
    LOG_FIELD("MYSQL_TYPE_DOUBLE");
  case MYSQL_TYPE_NULL:
    LOG_FIELD("MYSQL_TYPE_NULL");
  case MYSQL_TYPE_TIMESTAMP:
    LOG_FIELD("MYSQL_TYPE_TIMESTAMP");
  case MYSQL_TYPE_LONGLONG:
    LOG_FIELD("MYSQL_TYPE_LONGLONG");
  case MYSQL_TYPE_INT24:
    LOG_FIELD("MYSQL_TYPE_INT24");
  case MYSQL_TYPE_DATE:
    LOG_FIELD("MYSQL_TYPE_DATE");
  case MYSQL_TYPE_TIME:
    LOG_FIELD("MYSQL_TYPE_TIME");
  case MYSQL_TYPE_DATETIME:
    LOG_FIELD("MYSQL_TYPE_DATETIME");
  case MYSQL_TYPE_YEAR:
    LOG_FIELD("MYSQL_TYPE_YEAR");
  case MYSQL_TYPE_NEWDATE:
    LOG_FIELD("MYSQL_TYPE_NEWDATE");
  case MYSQL_TYPE_VARCHAR:
    LOG_FIELD("MYSQL_TYPE_VARCHAR");
  case MYSQL_TYPE_BIT:
    LOG_FIELD("MYSQL_TYPE_BIT");
  case MYSQL_TYPE_NEWDECIMAL:
    LOG_FIELD("MYSQL_TYPE_NEWDECIMAL");
  case MYSQL_TYPE_ENUM:
    LOG_FIELD("MYSQL_TYPE_ENUM");
  case MYSQL_TYPE_SET:
    LOG_FIELD("MYSQL_TYPE_SET");
  case MYSQL_TYPE_TINY_BLOB:
    LOG_FIELD("MYSQL_TYPE_TINY_BLOB");
  case MYSQL_TYPE_MEDIUM_BLOB:
    LOG_FIELD("MYSQL_TYPE_MEDIUM_BLOB");
  case MYSQL_TYPE_LONG_BLOB:
    LOG_FIELD("MYSQL_TYPE_LONG_BLOB");
  case MYSQL_TYPE_BLOB:
    LOG_FIELD("MYSQL_TYPE_BLOB");
  case MYSQL_TYPE_VAR_STRING:
    LOG_FIELD("MYSQL_TYPE_VAR_STRING");
  case MYSQL_TYPE_STRING:
    LOG_FIELD("MYSQL_TYPE_STRING");
  case MYSQL_TYPE_GEOMETRY:
    LOG_FIELD("MYSQL_TYPE_GEOMETRY");
  }
}
*/
