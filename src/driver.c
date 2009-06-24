#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "driver.h"
#include "config.h"

/* TLS variables */
__thread grn_ctx *mrn_ctx_tls;


/* static variables */
grn_hash *mrn_hash;
grn_obj *mrn_db, *mrn_lexicon;
pthread_mutex_t mrn_lock;
const char *mrn_logfile_name=MRN_LOG_FILE_NAME;
FILE *mrn_logfile = NULL;

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
  pthread_mutex_lock(&mrn_lock);
  MRN_LOG(GRN_LOG_NOTICE, "logfile closed by FLUSH LOGS");
  fflush(mrn_logfile);
  fclose(mrn_logfile); /* reopen logfile for rotation */
  mrn_logfile = fopen(mrn_logfile_name, "a");
  MRN_LOG(GRN_LOG_NOTICE, "-------------------------------");
  MRN_LOG(GRN_LOG_NOTICE, "logfile re-opened by FLUSH LOGS");
  pthread_mutex_unlock(&mrn_lock);
  return 0;
}

int mrn_init()
{
  grn_ctx ctx;

  // init groonga
  if (grn_init() != GRN_SUCCESS)
    goto err;

  grn_ctx_init(&ctx,0);

  // init log, and then we can do logging
  if (!(mrn_logfile = fopen(mrn_logfile_name, "a")))
    goto err;

  grn_logger_info_set(mrn_ctx_tls, &mrn_logger_info);
  GRN_LOG(&ctx, GRN_LOG_NOTICE, "%s start", PACKAGE_STRING);

  // init hash
  if (!(mrn_hash = grn_hash_create(&ctx,NULL,
                                   MRN_MAX_KEY_LEN,sizeof(size_t),
                                   GRN_OBJ_KEY_VAR_SIZE)))
  {
    GRN_LOG(&ctx, GRN_LOG_ERROR, "cannot init hash, exiting");
    goto err;
  }

  // init database
  struct stat dummy;
  if ((stat(MRN_DB_FILE_PATH, &dummy)))
  {
    GRN_LOG(&ctx, GRN_LOG_NOTICE, "database not exists");
    if ((mrn_db = grn_db_create(&ctx, MRN_DB_FILE_PATH, NULL)))
      GRN_LOG(&ctx, GRN_LOG_NOTICE, "database created");
    else
    {
      GRN_LOG(&ctx, GRN_LOG_ERROR, "cannot create database, exiting");
      goto err;
    }
  }
  else
    mrn_db = grn_db_open(&ctx, MRN_DB_FILE_PATH);

  if (!(mrn_lexicon = grn_ctx_get(&ctx,"lexicon",7)))
  {
    GRN_LOG(&ctx, GRN_LOG_NOTICE, "lexicon table not exists");
    if ((mrn_lexicon = grn_table_create(&ctx, MRN_LEXICON_TABLE_NAME,
                                        strlen(MRN_LEXICON_TABLE_NAME), NULL,
                                        GRN_OBJ_TABLE_PAT_KEY|GRN_OBJ_PERSISTENT,
                                        grn_ctx_at(&ctx,GRN_DB_SHORTTEXT), 0)))
      GRN_LOG(&ctx, GRN_LOG_NOTICE, "database created");
    else
    {
      GRN_LOG(&ctx, GRN_LOG_ERROR, "cannot create lexicon table, exiting");
      goto err;
    }
  }

  // init lock
  pthread_mutex_init(&mrn_lock, NULL);

  grn_ctx_fin(&ctx);
  return 0;

err:
  // TODO: report more detail error to mysql
  return -1;
}

/*
  TODO: release all grn_obj in global hash
*/
int mrn_deinit()
{
  grn_ctx ctx;
  grn_ctx_init(&ctx,0);
  grn_obj_close(&ctx, mrn_lexicon);
  grn_obj_close(&ctx, mrn_db);

  /* mutex deinit*/
  pthread_mutex_destroy(&mrn_lock);

  /* log deinit */
  GRN_LOG(&ctx, GRN_LOG_NOTICE, "------ stopping mroonga ------");
  fclose(mrn_logfile);
  mrn_logfile = NULL;

  /* hash deinit */
  grn_hash_close(&ctx, mrn_hash);

  /* libgroonga deinit */
  grn_ctx_fin(&ctx);
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
    if ((mrn_db))
      grn_ctx_use(mrn_ctx_tls, mrn_db);
  }
}

grn_obj *mrn_db_open_or_create(grn_ctx *ctx)
{

}

void mrn_share_put(mrn_table *share)
{
  void *value;
  grn_search_flags flags = GRN_TABLE_ADD;
  /* TODO: check duplication */
  MRN_LOG(GRN_LOG_DEBUG,"-> grn_hash_lookup(put): name='%s'", share->name);
  grn_hash_lookup(mrn_ctx_tls, mrn_hash, share->name,
		  strlen(share->name), &value, &flags);
  memcpy(value, share, sizeof(share));
}

/* returns NULL if specified obj_name is not found in grn_hash */
mrn_table *mrn_share_get(const char *name)
{
  void *value;
  grn_search_flags flags = 0;
  MRN_LOG(GRN_LOG_DEBUG,"-> grn_hash_lookup(get): name='%s'", name);
  grn_id rid = grn_hash_lookup(mrn_ctx_tls, mrn_hash, name,
			       strlen(name), &value, &flags);
  if (rid == 0) {
    return NULL;
  } else {
    return (mrn_table*) value;
  }
}

void mrn_share_remove(mrn_table *share)
{
  /* TODO: check return value */
  MRN_LOG(GRN_LOG_DEBUG, "-> grn_hash_delete: name='%s'", share->name);
  grn_hash_delete(mrn_ctx_tls, mrn_hash, share->name,
		  strlen(share->name), NULL);
}

void mrn_share_remove_all()
{
  /* TODO: implement this function by using GRN_HASH_EACH */
}


