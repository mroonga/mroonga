#ifdef USE_PRAGMA_IMPLEMENTATION
#pragma implementation
#endif

#define MYSQL_SERVER 1

#include <mysql_priv.h>
#include <mysql/plugin.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "ha_mroonga.h"

#ifdef __cplusplus
extern "C" {
#endif

/* global variables */
grn_obj *mrn_db;
grn_hash *mrn_hash;
pthread_mutex_t db_mutex;

/* logging */
const char *mrn_logfile_name=MRN_LOG_FILE_NAME;
FILE *mrn_logfile = NULL;
int mrn_logfile_opened = 0;

void mrn_logger_func(int level, const char *time, const char *title,
                     const char *msg, const char *location, void *func_arg)
{
  const char slev[] = " EACewnid-";
  if (mrn_logfile_opened) {
    fprintf(mrn_logfile, "%s|%c|%u|%s\n", time,
            *(slev + level), (uint)pthread_self(), msg);
    fflush(mrn_logfile);
  }
}

grn_logger_info mrn_logger_info = {
  GRN_LOG_DUMP,
  GRN_LOG_TIME|GRN_LOG_MESSAGE,
  mrn_logger_func,
  NULL
};

const char *mrn_log_level_str[] =
{
  "NONE",
  "EMERG",
  "ALERT",
  "CRIT",
  "ERROR",
  "WARNING",
  "NOTICE",
  "INFO",
  "DEBUG",
  "DUMP"
};

/* system functions */

struct st_mysql_storage_engine storage_engine_structure =
{ MYSQL_HANDLERTON_INTERFACE_VERSION };

struct st_mysql_show_var mrn_status_variables[] =
{
  {NULL}
};

struct st_mysql_sys_var *mrn_system_variables[] =
{
  NULL
};

mysql_declare_plugin(mroonga)
{
  MYSQL_STORAGE_ENGINE_PLUGIN,
  &storage_engine_structure,
  "mroonga",
  "Tetsuro IKEDA",
  "MySQL binding for groonga",
  PLUGIN_LICENSE_BSD,
  mrn_init,
  mrn_deinit,
  0x0001,
  mrn_status_variables,
  mrn_system_variables,
  NULL
}
mysql_declare_plugin_end;

int mrn_init(void *p)
{
  grn_ctx *ctx;

  // init handlerton
  handlerton *hton;
  hton = (handlerton *)p;
  hton->state = SHOW_OPTION_YES;
  hton->create = mrn_handler_create;
  hton->flags = 0;
  hton->drop_database = mrn_drop_db;

  // init groonga
  if (grn_init() != GRN_SUCCESS) {
    goto err;
  }

  ctx = grn_ctx_open(0);

  grn_logger_info_set(ctx, &mrn_logger_info);
  if (!(mrn_logfile = fopen(mrn_logfile_name, "a"))) {
    goto err;
  }
  mrn_logfile_opened = 1;
  GRN_LOG(ctx, GRN_LOG_NOTICE, "%s init", MRN_PACKAGE_STRING);

  // init meta-info database
  if (!(mrn_db = grn_db_create(ctx, NULL, NULL))) {
    GRN_LOG(ctx, GRN_LOG_ERROR, "cannot create system database, exiting");
    goto err;
  }
  grn_ctx_use(ctx, mrn_db);

  // init hash
  if (!(mrn_hash = grn_hash_create(ctx,NULL,
                                   MRN_MAX_KEY_SIZE,sizeof(size_t),
                                   GRN_OBJ_KEY_VAR_SIZE))) {
    GRN_LOG(ctx, GRN_LOG_ERROR, "cannot init hash, exiting");
    goto err;
  }
  // init lock
  if ((pthread_mutex_init(&db_mutex, NULL) != 0)) {
    goto err;
  }
  grn_ctx_fin(ctx);
  return 0;

err:
  grn_ctx_fin(ctx);
  grn_fin();
  return -1;
}

int mrn_deinit(void *p)
{
  grn_ctx *ctx;
  ctx = grn_ctx_open(0);

  GRN_LOG(ctx, GRN_LOG_NOTICE, "%s deinit", MRN_PACKAGE_STRING);

  pthread_mutex_destroy(&db_mutex);
  grn_hash_close(ctx, mrn_hash);
  grn_obj_unlink(ctx, mrn_db);

  if (mrn_logfile_opened) {
    fclose(mrn_logfile);
    mrn_logfile_opened = 0;
  }

  grn_ctx_fin(ctx);
  grn_fin();

  return 0;
}

handler *mrn_handler_create(handlerton *hton, TABLE_SHARE *share, MEM_ROOT *root)
{
  return (new (root) ha_mroonga(hton, share));
}

void mrn_drop_db(handlerton *hton, char *path)
{
  char db_path[MRN_MAX_PATH_SIZE];
  char db_name[MRN_MAX_PATH_SIZE];
  mrn_db_path_gen(path, db_path);
  mrn_db_name_gen(path, db_name);
  grn_ctx *ctx;
  ctx = grn_ctx_open(0);
  struct stat dummy;
  if (stat(db_path, &dummy) == 0) {
    grn_obj *db = grn_db_open(ctx, db_path);
    if (grn_obj_remove(ctx, db)) {
      GRN_LOG(ctx, GRN_LOG_ERROR, "cannot drop database (%s)", db_path);
    }
  }
  mrn_hash_remove(ctx, mrn_hash, db_name);
  grn_ctx_fin(ctx);
}

grn_builtin_type mrn_get_type(grn_ctx *ctx, int mysql_field_type)
{
  switch (mysql_field_type) {
  case MYSQL_TYPE_BIT:      // bit
  case MYSQL_TYPE_ENUM:     // enum
  case MYSQL_TYPE_SET:      // set
  case MYSQL_TYPE_TINY:     // tinyint
    return GRN_DB_INT8;
  case MYSQL_TYPE_SHORT:    // smallint
    return GRN_DB_INT16; // 2bytes
  case MYSQL_TYPE_INT24:    // mediumint
  case MYSQL_TYPE_LONG:     // int
    return GRN_DB_INT32; // 4bytes
  case MYSQL_TYPE_LONGLONG: // bigint
    return GRN_DB_INT64; // 8bytes
  case MYSQL_TYPE_FLOAT:    // float
  case MYSQL_TYPE_DOUBLE:   // double
    return GRN_DB_FLOAT; // 8bytes
  case MYSQL_TYPE_DATE:     // date
  case MYSQL_TYPE_TIME:     // time
  case MYSQL_TYPE_YEAR:     // year
  case MYSQL_TYPE_DATETIME: // datetime
    return GRN_DB_TIME; // micro sec from epoc time by int64
  }
  // tinytext=256, text=64K, mediumtext=16M, longtext=4G
  // tinyblob...
  // GRN_DB_SHORTTEXT 4096bytes
  // GRN_DB_TEXT      ???bytes
  // GRN_DB_LONGTEXT  ???bytes
  return GRN_DB_TEXT;       // others
}

int mrn_set_buf(grn_ctx *ctx, Field *field, grn_obj *buf, int *size)
{
  switch (field->type()) {
  case MYSQL_TYPE_BIT:
  case MYSQL_TYPE_ENUM:
  case MYSQL_TYPE_SET:
  case MYSQL_TYPE_TINY:
    {
      int val = field->val_int();
      GRN_INT8_INIT(buf, 0);
      GRN_INT8_SET(ctx, buf, val);
      *size = 1; 
      break;
    }
  case MYSQL_TYPE_SHORT:
    { 
      int val = field->val_int();
      GRN_INT16_INIT(buf, 0);
      GRN_INT16_SET(ctx, buf, val);
      *size = 2;
      break;
    }
  case MYSQL_TYPE_INT24:
  case MYSQL_TYPE_LONG:
    {
      int val = field->val_int();
      GRN_INT32_INIT(buf, 0);
      GRN_INT32_SET(ctx, buf, val);
      *size = 4;
      break;
    }
  case MYSQL_TYPE_LONGLONG:
    {
      long long int val = field->val_int();
      GRN_INT64_INIT(buf, 0);
      GRN_INT64_SET(ctx, buf, val);
      *size = 8;
      break;
    }
  case MYSQL_TYPE_FLOAT:
  case MYSQL_TYPE_DOUBLE:
    {
      double val = field->val_real();
      GRN_FLOAT_INIT(buf, 0);
      GRN_FLOAT_SET(ctx, buf, val);
      *size = 8;
      break;
    }
  case MYSQL_TYPE_TIME:
  case MYSQL_TYPE_YEAR:
  case MYSQL_TYPE_DATE:
  case MYSQL_TYPE_DATETIME:
    {
      long long int val = field->val_int();
      GRN_TIME_INIT(buf, 0);
      GRN_TIME_SET(ctx, buf, val);
      *size = 8;
      break;
    }
  case MYSQL_TYPE_STRING:
  case MYSQL_TYPE_VARCHAR:
    {
      String tmp;
      const char *val = field->val_str(&tmp)->ptr();
      int len = field->data_length();
      GRN_TEXT_INIT(buf, 0);
      GRN_TEXT_SET(ctx, buf, val, len);
      *size = len;
      break;
    }
  case MYSQL_TYPE_BLOB:
    {
      String tmp;
      Field_blob *blob = (Field_blob*) field;
      const char *val = blob->val_str(0,&tmp)->ptr();
      int len = blob->get_length();
      GRN_TEXT_INIT(buf, 0);
      GRN_TEXT_SET(ctx, buf, val, len);
      *size = len;
      break;
    }
  default:
    return HA_ERR_UNSUPPORTED;
  }
  return 0;
}

void mrn_store_field(grn_ctx *ctx, Field *field, grn_obj *col, grn_id id)
{
  grn_obj buf;
  field->set_notnull();
  switch (field->type()) {
  case (MYSQL_TYPE_BIT) :
  case (MYSQL_TYPE_ENUM) :
  case (MYSQL_TYPE_SET) :
  case (MYSQL_TYPE_TINY) :
    {
      GRN_INT8_INIT(&buf,0);
      grn_obj_get_value(ctx, col, id, &buf);
      int val = GRN_INT8_VALUE(&buf);
      field->store(val);
      break;
    }
  case (MYSQL_TYPE_SHORT) :
    {
      GRN_INT16_INIT(&buf,0);
      grn_obj_get_value(ctx, col, id, &buf);
      int val = GRN_INT16_VALUE(&buf);
      field->store(val);
      break;
    }
  case (MYSQL_TYPE_INT24) :
  case (MYSQL_TYPE_LONG) :
    {
      GRN_INT32_INIT(&buf,0);
      grn_obj_get_value(ctx, col, id, &buf);
      int val = GRN_INT32_VALUE(&buf);
      field->store(val);
      break;
    }
  case (MYSQL_TYPE_LONGLONG) :
    {
      GRN_INT64_INIT(&buf,0);
      grn_obj_get_value(ctx, col, id, &buf);
      long long int val = GRN_INT64_VALUE(&buf);
      field->store(val);
      break;
    }
  case (MYSQL_TYPE_FLOAT) :
  case (MYSQL_TYPE_DOUBLE) :
    {
      GRN_FLOAT_INIT(&buf,0);
      grn_obj_get_value(ctx, col, id, &buf);
      double val = GRN_FLOAT_VALUE(&buf);
      field->store(val);
      break;
    }
  case (MYSQL_TYPE_TIME) :
  case (MYSQL_TYPE_DATE) :
  case (MYSQL_TYPE_YEAR) :
  case (MYSQL_TYPE_DATETIME) :
    {
      GRN_TIME_INIT(&buf,0);
      grn_obj_get_value(ctx, col, id, &buf);
      long long int val = GRN_TIME_VALUE(&buf);
      field->store(val);
      break;
    }
  default: //strings etc..
    {
      GRN_TEXT_INIT(&buf,0);
      grn_obj_get_value(ctx, col, id, &buf);
      char *val = GRN_TEXT_VALUE(&buf);
      int len = GRN_TEXT_LEN(&buf);
      field->store(val, len, field->charset());
      break;
    }
  }
}

/* handler implementation */
ha_mroonga::ha_mroonga(handlerton *hton, TABLE_SHARE *share)
  :handler(hton, share)
{
  ctx = grn_ctx_open(0);
  grn_ctx_use(ctx, mrn_db);
}

ha_mroonga::~ha_mroonga()
{
  grn_ctx_fin(ctx);
}

const char *ha_mroonga::table_type() const
{
  return "mroonga";
}

const char *ha_mroonga::index_type(uint keynr)
{
  KEY key_info = table->s->key_info[keynr];
  if (key_info.algorithm == HA_KEY_ALG_FULLTEXT) {
    return "FULLTEXT";
  } else if (key_info.algorithm == HA_KEY_ALG_HASH) {
    return "HASH";
  } else {
    return "BTREE";
  }
}

static const char*ha_mroonga_exts[] = {
  NullS
};
const char **ha_mroonga::bas_ext() const
{
  return ha_mroonga_exts;
}

ulonglong ha_mroonga_table_flags =
    HA_NO_TRANSACTIONS |
    HA_PARTIAL_COLUMN_READ |
    HA_REC_NOT_IN_SEQ |
    HA_NULL_IN_KEY |
    HA_CAN_INDEX_BLOBS |
    //HA_STATS_RECORDS_IS_EXACT |
    HA_NO_PREFIX_CHAR_KEYS |
    HA_CAN_FULLTEXT |
    HA_NO_AUTO_INCREMENT |
    HA_CAN_BIT_FIELD;
    //HA_HAS_RECORDS;

ulonglong ha_mroonga::table_flags() const
{
  return ha_mroonga_table_flags;
}

ulong ha_mroonga::index_flags(uint idx, uint part, bool all_parts) const
{
  return 0;
}

int ha_mroonga::create(const char *name, TABLE *table, HA_CREATE_INFO *info)
{
  /* First, we must check if database is alreadly opened, created */
  grn_obj *db_obj;
  char db_name[MRN_MAX_PATH_SIZE];
  char db_path[MRN_MAX_PATH_SIZE];
  struct stat dummy;
  mrn_db_name_gen(name, db_name);
  mrn_db_path_gen(name, db_path);

  pthread_mutex_lock(&db_mutex);
  if (mrn_hash_get(ctx, mrn_hash, db_name, (void**) &(db_obj)) != 0) {
    if (stat(db_path, &dummy)) {
      // creating new database
      GRN_LOG(ctx, GRN_LOG_INFO, "database not found. creating...(%s)", db_path);
      db_obj = grn_db_create(ctx, db_path, NULL);
      if (db_obj == NULL) {
        pthread_mutex_unlock(&db_mutex);
        GRN_LOG(ctx, GRN_LOG_ERROR, "cannot create database (%s)", db_path);
        return -1;
      }
    } else {
      // opening existing database
      db_obj = grn_db_open(ctx, db_path);
      if (db_obj == NULL) {
        GRN_LOG(ctx, GRN_LOG_ERROR, "cannot open database (%s)", db_path);
        pthread_mutex_unlock(&db_mutex);
        return -1;
      }
    }
    mrn_hash_put(ctx, mrn_hash, db_name, db_obj);
  }
  pthread_mutex_unlock(&db_mutex);
  grn_ctx_use(ctx, db_obj);

  grn_obj_flags tbl_flags = GRN_OBJ_PERSISTENT;

  /* primary key must be handled before creating table */
  grn_obj *pkey_type;
  uint pkeynr = table->s->primary_key;
  if (pkeynr != MAX_INDEXES) {
    KEY key_info = table->s->key_info[pkeynr];

    // surpose simgle column key
    int key_parts = key_info.key_parts;
    if (key_parts != 1) {
      GRN_LOG(ctx, GRN_LOG_ERROR, "complex key is not supported (%s)", db_path);
      return -1;
    }
    Field *pkey_field = key_info.key_part[0].field;

    int mysql_field_type = pkey_field->type();
    grn_builtin_type gtype = mrn_get_type(ctx, mysql_field_type);
    pkey_type = grn_ctx_at(ctx, gtype);

    // default algorithm is BTREE ==> PAT
    if (key_info.algorithm == HA_KEY_ALG_HASH) {
      tbl_flags |= GRN_OBJ_TABLE_HASH_KEY;
    } else {
      tbl_flags |= GRN_OBJ_TABLE_PAT_KEY;
    }

  } else {
    // primary key doesn't exists
    tbl_flags |= GRN_OBJ_TABLE_NO_KEY;
    pkey_type = NULL;
  }

  /* create table */
  grn_obj *tbl_obj;
  char tbl_name[MRN_MAX_PATH_SIZE];
  mrn_table_name_gen(name, tbl_name);
  int tbl_name_len = strlen(tbl_name);

  if (mrn_check_table_name(tbl_name)) {
    GRN_LOG(ctx, GRN_LOG_ERROR, "invalid table name: name=%s", tbl_name);
    return -1;    
  }

  char *tbl_path = NULL;           // we don't specify path
  grn_obj *pkey_value_type = NULL; // we don't use this   

  tbl_obj = grn_table_create(ctx, tbl_name, tbl_name_len, tbl_path,
                         tbl_flags, pkey_type, pkey_value_type);

  if (tbl_obj == NULL) {
    GRN_LOG(ctx, GRN_LOG_ERROR, "cannot create table: name=%s", tbl_name);
    return -1;
  }

  /* create columns */
  int i;
  uint n_columns = table->s->fields;
  for (i=0; i < n_columns; i++) {
    grn_obj *col_obj, *col_type;
    Field *field = table->s->field[i];
    const char *col_name = field->field_name;
    int col_name_size = strlen(col_name);
    grn_obj_flags col_flags = GRN_OBJ_PERSISTENT | GRN_OBJ_COLUMN_SCALAR;
    int mysql_field_type = field->type();
    grn_builtin_type gtype = mrn_get_type(ctx, mysql_field_type);
    col_type = grn_ctx_at(ctx, gtype);
    char *col_path = NULL; // we don't specify path

    col_obj = grn_column_create(ctx, tbl_obj, col_name, col_name_size,
                                col_path, col_flags, col_type);
    if (col_obj == NULL) {
      GRN_LOG(ctx, GRN_LOG_ERROR, "cannot create table: name=%s, col=%s",
              tbl_name, col_name);
      grn_obj_remove(ctx, tbl_obj);
      return -1;
    }
  }

  /* create indexes */
  uint n_keys = table->s->keys;
  char name_buff[MRN_MAX_PATH_SIZE];
  grn_obj *lex_obj=NULL, *hash_obj=NULL, *pat_obj=NULL;
  int need_lex=0, need_hash=0, need_pat=0;

  for (i=0; i < n_keys; i++) {
    if (i == pkeynr) {
      continue; // pkey is already handled
    }
    KEY key_info = table->s->key_info[i];
    int key_alg = key_info.algorithm;
    if (key_alg == HA_KEY_ALG_FULLTEXT) {
      need_lex = 1;
    } else if (key_alg == HA_KEY_ALG_HASH) {
      need_hash = 1;
    } else {
      need_pat = 1;
    }
  }

  if (need_lex) {
    mrn_lex_name_gen(name, name_buff);
    grn_obj_flags lex_flags = GRN_OBJ_TABLE_PAT_KEY | GRN_OBJ_PERSISTENT;
    grn_obj *lex_type = grn_ctx_at(ctx, GRN_DB_SHORT_TEXT);
    lex_obj = grn_table_create(ctx, name_buff, strlen(name_buff), NULL,
                               lex_flags, lex_type, 0);
    if (lex_obj == NULL) {
      GRN_LOG(ctx, GRN_LOG_ERROR, "cannot create lex for table: name=%s", tbl_name);
      grn_obj_remove(ctx, tbl_obj);
      return -1;
    }
    grn_info_type info_type = GRN_INFO_DEFAULT_TOKENIZER;
    grn_obj *index_type = grn_ctx_at(ctx, GRN_DB_BIGRAM);
    grn_obj_set_info(ctx, lex_obj, info_type, index_type);
  }

  if (need_hash) {
    mrn_hash_name_gen(name, name_buff);
    grn_obj_flags hash_flags = GRN_OBJ_TABLE_HASH_KEY | GRN_OBJ_PERSISTENT;
    grn_obj *hash_type = grn_ctx_at(ctx, GRN_DB_INT32); //TODO: check if this is ok
    hash_obj = grn_table_create(ctx, name_buff, strlen(name_buff), NULL,
                                hash_flags, hash_type, 0);
    if (hash_obj == NULL) {
      GRN_LOG(ctx, GRN_LOG_ERROR, "cannot create hash for table: name=%s", tbl_name);
      grn_obj_remove(ctx, tbl_obj);
      return -1;
    }
  }

  if (need_pat) {
    mrn_pat_name_gen(name, name_buff);
    grn_obj_flags pat_flags = GRN_OBJ_TABLE_PAT_KEY | GRN_OBJ_PERSISTENT;
    grn_obj *pat_type = grn_ctx_at(ctx, GRN_DB_INT32); //TODO: check if this is ok
    pat_obj = grn_table_create(ctx, name_buff, strlen(name_buff), NULL,
                               pat_flags, pat_type, 0);
    if (pat_obj == NULL) {
      GRN_LOG(ctx, GRN_LOG_ERROR, "cannot create pat for table: name=%s", tbl_name);
      grn_obj_remove(ctx, tbl_obj);
      return -1;
    }
  }

  for (i=0; i < n_keys; i++) {
    if (i == pkeynr) {
      continue; // pkey is already handled
    }
    grn_obj *index_obj, *col_obj, buf;
    KEY key_info = table->s->key_info[i];
    // must be single column key
    int key_parts = key_info.key_parts;
    if (key_parts != 1) {
      GRN_LOG(ctx, GRN_LOG_ERROR, "complex key is not supported (%s)", db_path);
      return -1;
    }
    int key_alg = key_info.algorithm;
    Field *field = key_info.key_part[0].field;
    const char *col_name = field->field_name;
    int col_name_size = strlen(col_name);
    col_obj = grn_obj_column(ctx, tbl_obj, col_name, col_name_size);

    grn_obj_flags index_flags = GRN_OBJ_COLUMN_INDEX | GRN_OBJ_PERSISTENT;
    if (key_alg == HA_KEY_ALG_FULLTEXT) {
      index_obj = grn_column_create(ctx, lex_obj, col_name, col_name_size, NULL,
                                    index_flags, tbl_obj);
    } else if (key_alg == HA_KEY_ALG_HASH) {
      index_obj = grn_column_create(ctx, hash_obj, col_name, col_name_size, NULL,
                                    index_flags, tbl_obj);
    } else {
      index_obj = grn_column_create(ctx, pat_obj, col_name, col_name_size, NULL,
                                    index_flags, tbl_obj);
    }

    if (index_obj == NULL) {
      GRN_LOG(ctx, GRN_LOG_ERROR, "cannot create index: name=%s, col=%s",
              tbl_name, col_name);
      grn_obj_remove(ctx, tbl_obj);
      return -1;
    }

    grn_id gid = grn_obj_id(ctx, col_obj);
    GRN_TEXT_INIT(&buf, 0);
    GRN_TEXT_SET(ctx, &buf, (char*) &gid, sizeof(grn_id));
    grn_obj_set_info(ctx, index_obj, GRN_INFO_SOURCE, &buf);
  }

  /* clean up */
  if (lex_obj != NULL) {
    grn_obj_unlink(ctx, lex_obj);
  }
  if (hash_obj != NULL) {
    grn_obj_unlink(ctx, hash_obj);
  }
  if (pat_obj != NULL) {
    grn_obj_unlink(ctx, pat_obj);
  }
  grn_obj_unlink(ctx, tbl_obj);

  return 0;
}

int ha_mroonga::open(const char *name, int mode, uint test_if_locked)
{
  thr_lock_init(&thr_lock);
  thr_lock_data_init(&thr_lock, &thr_lock_data, NULL);

  /* First, we must check if database is alreadly opened */
  char db_name[MRN_MAX_PATH_SIZE];
  char db_path[MRN_MAX_PATH_SIZE];
  struct stat dummy;
  mrn_db_name_gen(name, db_name);
  mrn_db_path_gen(name, db_path);

  pthread_mutex_lock(&db_mutex);
  // we should not call grn_db_open() very often. so we use cache.
  if (mrn_hash_get(ctx, mrn_hash, db_name, (void**) &(db)) != 0) {
    db = grn_db_open(ctx, db_path);
    if (db == NULL) {
      GRN_LOG(ctx, GRN_LOG_ERROR, "cannot open database (%s)", db_path);
      pthread_mutex_unlock(&db_mutex);
      return -1;
    }
    mrn_hash_put(ctx, mrn_hash, db_name, db);
  }
  pthread_mutex_unlock(&db_mutex);
  grn_ctx_use(ctx, db);

  /* open table */
  char buf[MRN_MAX_PATH_SIZE];
  mrn_table_name_gen(name, buf);
  tbl = grn_ctx_get(ctx, buf, strlen(buf));
  if (tbl == NULL) {
    GRN_LOG(ctx, GRN_LOG_ERROR, "cannot open table (%s)", buf);
    return -1;
  }

  /* open columns */
  int n_columns = table->s->fields;
  col = (grn_obj**) malloc(sizeof(grn_obj*) * n_columns);

  int i;
  for (i=0; i < n_columns; i++) {
    Field *field = table->field[i];
    const char *col_name = field->field_name;
    int col_name_size = strlen(col_name);
    col[i] = grn_obj_column(ctx, tbl, col_name, col_name_size);
    if (col[i] == NULL) {
      GRN_LOG(ctx, GRN_LOG_ERROR, "cannot open table(col) %s(%s)",
              buf, col_name);
      grn_obj_unlink(ctx, tbl);
      return -1;
    }
  }

  /* open indexes */
  mrn_lex_name_gen(name, buf);
  lex = grn_ctx_get(ctx, buf, strlen(buf));
  mrn_hash_name_gen(name, buf);
  hash = grn_ctx_get(ctx, buf, strlen(buf));
  mrn_pat_name_gen(name, buf);
  pat = grn_ctx_get(ctx, buf, strlen(buf));

  uint n_keys = table->s->keys;
  uint pkeynr = table->s->primary_key;
  if (n_keys > 0) {
    index = (grn_obj**) malloc(sizeof(grn_obj*) * n_keys);
  } else {
    index = NULL;
  }
  for (i=0; i < n_keys; i++) {
    if (i == pkeynr) {
      index[i] = NULL;
      continue;
    }
    KEY key_info = table->s->key_info[i];
    // surpose simgle column key
    int key_parts = key_info.key_parts;
    if (key_parts != 1) {
      GRN_LOG(ctx, GRN_LOG_ERROR, "complex key is not supported (%s)", db_path);
      return -1;
    }
    Field *field = key_info.key_part[0].field;
    const char *col_name = field->field_name;
    int col_name_size = strlen(col_name);
    int key_alg = key_info.algorithm;

    if (key_alg == HA_KEY_ALG_FULLTEXT) {    // fulltext
      index[i] = grn_obj_column(ctx, lex, col_name, col_name_size);
    } else if (key_alg == HA_KEY_ALG_HASH) { // hash
      index[i] = grn_obj_column(ctx, hash, col_name, col_name_size);
    } else {                                 // btree
      index[i] = grn_obj_column(ctx, pat, col_name, col_name_size);
    }
  }

  return 0;
}

int ha_mroonga::close()
{
  thr_lock_delete(&thr_lock);
  if (lex != NULL) {
    grn_obj_unlink(ctx, lex);
  }
  if (hash != NULL) {
    grn_obj_unlink(ctx, hash);
  }
  if (pat != NULL) {
    grn_obj_unlink(ctx, pat);
  }
  grn_obj_unlink(ctx, tbl);

  if (index != NULL) {
    free(index);
  }
  free(col);
  return 0;
}

int ha_mroonga::delete_table(const char *name)
{
  char buf[MRN_MAX_PATH_SIZE];

  grn_obj *db_obj, *tbl_obj, *lex_obj, *hash_obj, *pat_obj;
  mrn_db_path_gen(name, buf);
  db_obj = grn_db_open(ctx, buf);
  if (db_obj == NULL) {
    GRN_LOG(ctx, GRN_LOG_ERROR, "grn_db_open failed while delete_table (%s)", buf);
    return -1;
  }
  grn_ctx_use(ctx, db_obj);

  mrn_lex_name_gen(name, buf);
  lex_obj = grn_ctx_get(ctx, buf, strlen(buf));
  if (lex_obj != NULL) {
    grn_obj_remove(ctx, lex_obj);
  }

  mrn_hash_name_gen(name, buf);
  hash_obj = grn_ctx_get(ctx, buf, strlen(buf));
  if (hash_obj != NULL) {
    grn_obj_remove(ctx, hash_obj);
  }

  mrn_pat_name_gen(name, buf);
  pat_obj = grn_ctx_get(ctx, buf, strlen(buf));
  if (pat_obj != NULL) {
    grn_obj_remove(ctx, pat_obj);
  }

  mrn_table_name_gen(name, buf);
  tbl_obj = grn_ctx_get(ctx, buf, strlen(buf));
  if (tbl_obj == NULL) {
    GRN_LOG(ctx, GRN_LOG_ERROR, "grn_ctx_get failed while mrn_drop (%s)", buf);
    return -1;
  }
  return grn_obj_remove(ctx, tbl_obj);
}

int ha_mroonga::info(uint flag)
{
  ha_rows rows = grn_table_size(ctx, tbl);
  stats.records = rows;
  return 0;
}

THR_LOCK_DATA **ha_mroonga::store_lock(THD *thd, THR_LOCK_DATA **to,
                                       enum thr_lock_type lock_type)
{
  if (lock_type != TL_IGNORE && thr_lock_data.type == TL_UNLOCK) {
    thr_lock_data.type = lock_type;
  }
  *to++ = &thr_lock_data;
  return to;
}

int ha_mroonga::rnd_init(bool scan)
{
  cur = grn_table_cursor_open(ctx, tbl, NULL, 0, NULL, 0, 0, -1, 0);
  if (cur == NULL) {
      GRN_LOG(ctx, GRN_LOG_ERROR, "cannot open cursor");
      return -1;
  }
  return 0;
}

int ha_mroonga::rnd_next(uchar *buf)
{
  row_id = grn_table_cursor_next(ctx, cur);
  if (row_id == GRN_ID_NIL) {
    grn_table_cursor_close(ctx, cur);
    return HA_ERR_END_OF_FILE;
  }
  int i;
  int n_columns = table->s->fields;
  for (i=0; i < n_columns; i++) {
    Field *field = table->field[i];
    mrn_store_field(ctx, field, col[i], row_id);
  }
  return 0;
}

int ha_mroonga::rnd_pos(uchar *buf, uchar *pos)
{
  row_id = *((grn_id*) pos);
  int i;
  int n_columns = table->s->fields;
  for (i=0; i < n_columns; i++) {
    Field *field = table->field[i];
    mrn_store_field(ctx, field, col[i], row_id);
  }
  return 0;
}

void ha_mroonga::position(const uchar *record)
{
  memcpy(ref, &row_id, sizeof(grn_id));
}

int ha_mroonga::write_row(uchar *buf)
{
  void *pkey;
  grn_obj wrapper;
  int pkey_size = 0;
  uint pkeynr = table->s->primary_key;
  if (pkeynr != MAX_INDEXES) {
    KEY key_info = table->s->key_info[pkeynr];
    // surpose simgle column key
    int field_no = key_info.key_part[0].field->field_index;
    Field *pkey_field = table->field[field_no];
    mrn_set_buf(ctx, pkey_field, &wrapper, &pkey_size);
    pkey = GRN_TEXT_VALUE(&wrapper);
  }

  int added;
  row_id = grn_table_add(ctx, tbl, pkey, pkey_size, &added);
  if (added == 0) {
    // duplicated error
    return -1;
  }

  grn_obj colbuf;
  int i, col_size;
  int n_columns = table->s->fields;
  for (i=0; i < n_columns; i++) {
    Field *field = table->field[i];
    mrn_set_buf(ctx, field, &colbuf, &col_size);
    if (grn_obj_set_value(ctx, col[i], row_id, &colbuf, GRN_OBJ_SET)
        != GRN_SUCCESS) {
      return -1;
    }
  }

  return 0;
}

int ha_mroonga::update_row(const uchar *old_data, uchar *new_data)
{
  grn_obj colbuf;
  int i, col_size;
  int n_columns = table->s->fields;
  for (i=0; i < n_columns; i++) {
    Field *field = table->field[i];
    mrn_set_buf(ctx, field, &colbuf, &col_size);
    if (grn_obj_set_value(ctx, col[i], row_id, &colbuf, GRN_OBJ_SET)
        != GRN_SUCCESS) {
      return -1;
    }
  }
  return 0;
}

int ha_mroonga::delete_row(const uchar *buf)
{
  return 0;
}

ha_rows ha_mroonga::records_in_range(uint inx, key_range *min_key, key_range *max_key)
{
  uint pkeynr = table->s->primary_key;
  if (inx == pkeynr) {
    return 1;
  } else {
    return 2;
  }
}

int ha_mroonga::index_read(uchar *buf, const uchar *key,
                           uint key_len, enum ha_rkey_function find_flag)
{
  return 0;
}

int ha_mroonga::index_next(uchar *buf)
{
  return HA_ERR_END_OF_FILE;
}

int ha_mroonga::ft_init() {
  return 0;
}

FT_INFO *ha_mroonga::ft_init_ext(uint flags, uint keynr, String *key)
{
  grn_obj *ft = index[keynr];
  const char *keyword = key->ptr();
  int keyword_size = strlen(keyword);

  res = grn_table_create(ctx, NULL, 0, NULL, GRN_TABLE_HASH_KEY, tbl, 0);
  
  if (flags & FT_BOOL) {
    // boolean search
    grn_query *query = grn_query_open(ctx, keyword, keyword_size,
                                      GRN_OP_OR, MRN_MAX_EXPRS);
    grn_obj_search(ctx, ft, (grn_obj*) query, res, GRN_OP_OR, NULL);
  } else {
    // nlq search
    grn_obj buf;
    GRN_TEXT_INIT(&buf, 0);
    GRN_TEXT_SET(ctx, &buf, keyword, keyword_size);
    grn_obj_search(ctx, ft, &buf, res, GRN_OP_OR, NULL);
  }
  int n_rec = grn_table_size(ctx, res);
  cur = grn_table_cursor_open(ctx, res, NULL, 0, NULL, 0, 0, -1, 0);
  return NULL;
}

int ha_mroonga::ft_read(uchar *buf)
{
  grn_id rid;
  
  rid = grn_table_cursor_next(ctx, cur);

  if (rid == GRN_ID_NIL) {
    grn_table_cursor_close(ctx, cur);
    grn_obj_unlink(ctx, res);
    return HA_ERR_END_OF_FILE;
  }

  grn_table_get_key(ctx, res, rid, &row_id, sizeof(grn_id));

  int i;
  int n_columns = table->s->fields;
  for (i=0; i < n_columns; i++) {
    Field *field = table->field[i];
    mrn_store_field(ctx, field, col[i], row_id);
  }
  return 0;
}

const COND *ha_mroonga::cond_push(const COND *cond)
{
  return NULL;
}

void ha_mroonga::cond_pop()
{
}

#ifdef __cplusplus
}
#endif
