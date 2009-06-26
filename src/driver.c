#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "driver.h"
#include "config.h"

grn_hash *mrn_hash;
grn_obj *mrn_db, *mrn_lexicon;
pthread_mutex_t *mrn_lock;
const char *mrn_logfile_name=MRN_LOG_FILE_NAME;
FILE *mrn_logfile = NULL;

uint mrn_hash_counter=0;

grn_logger_info mrn_logger_info = {
  GRN_LOG_DUMP,
  GRN_LOG_TIME|GRN_LOG_MESSAGE,
  mrn_logger_func,
  NULL
};

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

int mrn_flush_logs(grn_ctx *ctx)
{
  pthread_mutex_lock(mrn_lock);
  GRN_LOG(ctx, GRN_LOG_NOTICE, "flush logfile");
  if (fflush(mrn_logfile) || fclose(mrn_logfile)
      || (mrn_logfile = fopen(mrn_logfile_name, "a")) == NULL)
  {
    GRN_LOG(ctx, GRN_LOG_ERROR, "cannot flush logfile");
    return -1;
  }
  pthread_mutex_unlock(mrn_lock);
  return 0;
}

int mrn_init()
{
  grn_ctx ctx;

  // init groonga
  if (grn_init() != GRN_SUCCESS)
  {
    goto err;
  }

  grn_ctx_init(&ctx,0);

  // init log, and then we can do logging
  if (!(mrn_logfile = fopen(mrn_logfile_name, "a")))
  {
    goto err;
  }

  grn_logger_info_set(&ctx, &mrn_logger_info);
  GRN_LOG(&ctx, GRN_LOG_NOTICE, "%s start", PACKAGE_STRING);

  // init database
  struct stat dummy;
  if ((stat(MRN_DB_FILE_PATH, &dummy)))
  {
    GRN_LOG(&ctx, GRN_LOG_NOTICE, "database not exists");
    if ((mrn_db = grn_db_create(&ctx, MRN_DB_FILE_PATH, NULL)))
    {
      GRN_LOG(&ctx, GRN_LOG_NOTICE, "database created");
    }
    else
    {
      GRN_LOG(&ctx, GRN_LOG_ERROR, "cannot create database, exiting");
      goto err;
    }
  }
  else
  {
    mrn_db = grn_db_open(&ctx, MRN_DB_FILE_PATH);
  }
  grn_ctx_use(&ctx, mrn_db);


  // init lexicon table
  if (!(mrn_lexicon = grn_ctx_get(&ctx,"lexicon",7)))
  {
    GRN_LOG(&ctx, GRN_LOG_NOTICE, "lexicon table not exists");
    if ((mrn_lexicon = grn_table_create(&ctx, MRN_LEXICON_TABLE_NAME,
                                        strlen(MRN_LEXICON_TABLE_NAME), NULL,
                                        GRN_OBJ_TABLE_PAT_KEY|GRN_OBJ_PERSISTENT,
                                        grn_ctx_at(&ctx,GRN_DB_SHORTTEXT), 0)))
    {
      GRN_LOG(&ctx, GRN_LOG_NOTICE, "database created");
    }
    else
    {
      GRN_LOG(&ctx, GRN_LOG_ERROR, "cannot create lexicon table, exiting");
      goto err;
    }
  }

  // init hash
  if (!(mrn_hash = grn_hash_create(&ctx,NULL,
                                   MRN_MAX_KEY_LEN,sizeof(size_t),
                                   GRN_OBJ_KEY_VAR_SIZE)))
  {
    GRN_LOG(&ctx, GRN_LOG_ERROR, "cannot init hash, exiting");
    goto err;
  }

  // init lock
  mrn_lock = malloc(sizeof(pthread_mutex_t));
  if ((mrn_lock == NULL) || (pthread_mutex_init(mrn_lock, NULL) != 0))
  {
    goto err;
  }

  grn_ctx_fin(&ctx);
  return 0;

err:
  // TODO: report more detail error to mysql
  grn_ctx_fin(&ctx);
  return -1;
}

int mrn_deinit()
{
  grn_ctx ctx;
  grn_ctx_init(&ctx,0);
  grn_ctx_use(&ctx, mrn_db);
  GRN_LOG(&ctx, GRN_LOG_NOTICE, "shutdown");

  pthread_mutex_destroy(mrn_lock);
  free(mrn_lock);
  mrn_lock = NULL;

  grn_hash_close(&ctx, mrn_hash);
  mrn_hash = NULL;

  grn_obj_close(&ctx, mrn_lexicon);
  mrn_lexicon = NULL;

  grn_db_close(&ctx, mrn_db);
  mrn_db = NULL;

  fclose(mrn_logfile);
  mrn_logfile = NULL;

  grn_ctx_fin(&ctx);
  grn_fin();

  return 0;
}

/**
 *   0 success
 *  -1 duplicated
 */
int mrn_hash_put(grn_ctx *ctx, const char *key, void *value)
{
  int added, res=0;
  void *buf;
  pthread_mutex_lock(mrn_lock);
  grn_hash_add(ctx, mrn_hash, (const char*) key, strlen(key), &buf, &added);
  // duplicate check
  if (added == 0)
  {
    GRN_LOG(ctx, GRN_LOG_WARNING, "hash put duplicated (key=%s)", key);
    res = -1;
  } else {
    memcpy(buf, value, sizeof(value));
    mrn_hash_counter++;
  }
  pthread_mutex_unlock(mrn_lock);
  return res;
}

/**
 *   0 success
 *  -1 not found
 */
int mrn_hash_get(grn_ctx *ctx, const char *key, void **value)
{
  int res = 0;
  grn_id id;
  grn_search_flags flags = 0;
  pthread_mutex_lock(mrn_lock);
  id = grn_hash_lookup(ctx, mrn_hash, (const char*) key, strlen(key), value, &flags);
  // key not found
  if (id == GRN_ID_NIL)
  {
    GRN_LOG(ctx, GRN_LOG_WARNING, "hash get not found (key=%s)", key);
    res = -1;
  }
  pthread_mutex_unlock(mrn_lock);
  return res;
}

/**
 *   0 success
 *  -1 error
 */
int mrn_hash_remove(grn_ctx *ctx, const char *key)
{
  int res = 0;
  grn_rc rc;
  grn_id id;
  grn_search_flags flags = 0;
  pthread_mutex_lock(mrn_lock);
  id = grn_hash_lookup(ctx, mrn_hash, (const char*) key, strlen(key), NULL, &flags);
  if (id == GRN_ID_NIL)
  {
    GRN_LOG(ctx, GRN_LOG_WARNING, "hash remove not found (key=%s)", key);
    res = -1;
  } else {
    rc = grn_hash_delete_by_id(ctx, mrn_hash, id, NULL);
    if (rc != GRN_SUCCESS) {
      GRN_LOG(ctx, GRN_LOG_ERROR, "hash remove error (key=%s)", key);
      res = -1;
    } else {
      mrn_hash_counter--;
    }
  }
  pthread_mutex_unlock(mrn_lock);
  return res;
}

void mrn_share_put(grn_ctx *ctx, mrn_table *share)
{
  void *value;
  grn_search_flags flags = GRN_TABLE_ADD;
  grn_hash_lookup(ctx, mrn_hash, share->name,
		  strlen(share->name), &value, &flags);
  memcpy(value, share, sizeof(share));
}

mrn_table *mrn_share_get(grn_ctx *ctx, const char *name)
{
  void *value;
  grn_search_flags flags = 0;
  grn_id rid = grn_hash_lookup(ctx, mrn_hash, name,
			       strlen(name), &value, &flags);
  if (rid == 0) {
    return NULL;
  } else {
    return (mrn_table*) value;
  }
}

void mrn_share_remove(grn_ctx *ctx, mrn_table *share)
{
  grn_hash_delete(ctx, mrn_hash, share->name,
		  strlen(share->name), NULL);
}

void mrn_share_remove_all()
{
}


