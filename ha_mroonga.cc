/* Copyright(C) 2010 Tetsuro IKEDA

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifdef USE_PRAGMA_IMPLEMENTATION
#pragma implementation
#endif

#define MYSQL_SERVER 1

#include <mysql_priv.h>
#include <mysql/plugin.h>
#include <sql_select.h>
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
const char *mrn_logfile_name = MRN_LOG_FILE_NAME;
FILE *mrn_logfile = NULL;
int mrn_logfile_opened = 0;

void mrn_logger_func(int level, const char *time, const char *title,
                     const char *msg, const char *location, void *func_arg)
{
  const char slev[] = " EACewnid-";
  if (mrn_logfile_opened) {
    fprintf(mrn_logfile, "%s|%c|%08x|%s\n", time,
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


/* Groonga information schema */
int GROONGA_VERSION_SHORT = 0x0001;
static const char plugin_author[] = "Yoshinori Matsunobu";
static struct st_mysql_information_schema	i_s_info =
{
	MYSQL_INFORMATION_SCHEMA_INTERFACE_VERSION
};

static ST_FIELD_INFO	i_s_groonga_stats_fields_info[] =
{
  {
    "VERSION",
    40,
    MYSQL_TYPE_STRING,
    0,
    0,
    "",
    SKIP_OPEN_TABLE
  },
  {
    "rows_written",
    MY_INT32_NUM_DECIMAL_DIGITS,
    MYSQL_TYPE_LONG,
    0,
    0,
    "Rows written to groonga",
    SKIP_OPEN_TABLE
  },
  {
    "rows_read",
    MY_INT32_NUM_DECIMAL_DIGITS,
    MYSQL_TYPE_LONG,
    0,
    0,
    "Rows read from groonga",
    SKIP_OPEN_TABLE
  }
};

static int i_s_groonga_stats_deinit(void* p)
{
  DBUG_ENTER("i_s_common_deinit");
  DBUG_RETURN(0);
}

static int i_s_groonga_stats_fill(
  THD* thd, TABLE_LIST* tables, COND* cond)
{
  TABLE*	table	= (TABLE *) tables->table;
  int	status	= 0;
  DBUG_ENTER("i_s_groonga_fill_low");
  table->field[0]->store(grn_get_version(), strlen(grn_get_version()),
     system_charset_info);
  table->field[0]->set_notnull();
  table->field[1]->store(1); /* TODO */
  table->field[2]->store(2); /* TODO */
  if (schema_table_store_record(thd, table)) {
    status = 1;
  }
  DBUG_RETURN(status);
}

static int i_s_groonga_stats_init(void* p)
{
  DBUG_ENTER("i_s_groonga_stats_init");
  ST_SCHEMA_TABLE* schema = (ST_SCHEMA_TABLE*) p;
  schema->fields_info = i_s_groonga_stats_fields_info;
  schema->fill_table = i_s_groonga_stats_fill;
  DBUG_RETURN(0);
}

struct st_mysql_plugin i_s_groonga_stats =
{
  MYSQL_INFORMATION_SCHEMA_PLUGIN,
  &i_s_info,
  "GROONGA_STATS",
  plugin_author,
  "Statistics for Groonga",
  PLUGIN_LICENSE_GPL,
  i_s_groonga_stats_init,
  i_s_groonga_stats_deinit,
  GROONGA_VERSION_SHORT,
  NULL,
  NULL,
  NULL,
};
/* End of groonga information schema implementations */

mysql_declare_plugin(mroonga)
{
  MYSQL_STORAGE_ENGINE_PLUGIN,
  &storage_engine_structure,
  "groonga",
  "Tetsuro IKEDA",
  "Fulltext search, column base",
  0,
  mrn_init,
  mrn_deinit,
  0x0001,
  mrn_status_variables,
  mrn_system_variables,
  NULL
},i_s_groonga_stats
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
      grn_obj_reinit(ctx, buf, GRN_DB_INT8, 0);
      GRN_INT8_SET(ctx, buf, val);
      *size = 1;
      break;
    }
  case MYSQL_TYPE_SHORT:
    {
      int val = field->val_int();
      grn_obj_reinit(ctx, buf, GRN_DB_INT16, 0);
      GRN_INT16_SET(ctx, buf, val);
      *size = 2;
      break;
    }
  case MYSQL_TYPE_INT24:
  case MYSQL_TYPE_LONG:
    {
      int val = field->val_int();
      grn_obj_reinit(ctx, buf, GRN_DB_INT32, 0);
      GRN_INT32_SET(ctx, buf, val);
      *size = 4;
      break;
    }
  case MYSQL_TYPE_LONGLONG:
    {
      long long int val = field->val_int();
      grn_obj_reinit(ctx, buf, GRN_DB_INT64, 0);
      GRN_INT64_SET(ctx, buf, val);
      *size = 8;
      break;
    }
  case MYSQL_TYPE_FLOAT:
  case MYSQL_TYPE_DOUBLE:
    {
      double val = field->val_real();
      grn_obj_reinit(ctx, buf, GRN_DB_FLOAT, 0);
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
      grn_obj_reinit(ctx, buf, GRN_DB_TIME, 0);
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
      grn_obj_reinit(ctx, buf, GRN_DB_TEXT, 0);
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
      grn_obj_reinit(ctx, buf, GRN_DB_TEXT, 0);
      GRN_TEXT_SET(ctx, buf, val, len);
      *size = len;
      break;
    }
  default:
    return HA_ERR_UNSUPPORTED;
  }
  return 0;
}

int mrn_set_key_buf(grn_ctx *ctx, Field *field, const uchar *key, char *buf, uint *size)
{
  char *ptr = (char*) key;

  if (field->null_bit != 0x0) {
    ptr += 1;
  }

  switch (field->type()) {
  case MYSQL_TYPE_BIT:
  case MYSQL_TYPE_ENUM:
  case MYSQL_TYPE_SET:
  case MYSQL_TYPE_TINY:
    {
      char val = *ptr;
      buf[0] = val;
      *size = 1;
      break;
    }
  case MYSQL_TYPE_SHORT:
    {
      memcpy(buf, ptr, 2);
      *size = 2;
      break;
    }
  case MYSQL_TYPE_INT24:
    {
      memcpy(buf, ptr, 3);
      buf[3] = 0;
      *size = 4;
      break;
    }
  case MYSQL_TYPE_LONG:
    {
      memcpy(buf, ptr, 4);
      *size = 4;
      break;
    }
  case MYSQL_TYPE_LONGLONG:
    {
      memcpy(buf, ptr, 8);
      *size = 8;
      break;
    }
  case MYSQL_TYPE_FLOAT:
    {
      double val;
      float4get(val, ptr);
      memcpy(buf, &val, 8);
      *size = 8;
      break;
    }
  case MYSQL_TYPE_DOUBLE:
    {
      double val;
      float8get(val, ptr);
      memcpy(buf, &val, 8);
      *size = 8;
      break;
    }
  case MYSQL_TYPE_TIME:
  case MYSQL_TYPE_YEAR:
  case MYSQL_TYPE_DATE:
  case MYSQL_TYPE_DATETIME:
    {
      long long int val = (long long int) sint8korr(ptr);
      memcpy(buf, &val, 8);
      *size = 8;
      break;
    }
  case MYSQL_TYPE_STRING:
  case MYSQL_TYPE_VARCHAR:
    {
      ptr += 2;
      String tmp;
      const char *val = ptr;
      int len = strlen(val);
      memcpy(buf, val, len);
      *size = len;
      break;
    }
  case MYSQL_TYPE_BLOB:
    {
      ptr += 2;
      String tmp;
      const char *val = ptr;
      int len = strlen(val);
      memcpy(buf, val, len);
      *size = len;
      break;
    }
  default:
    return -1;
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
  grn_obj_unlink(ctx, &buf);
}

/* handler implementation */
ha_mroonga::ha_mroonga(handlerton *hton, TABLE_SHARE *share)
  :handler(hton, share)
{
  DBUG_ENTER("ha_mroonga::ha_mroonga");
  ctx = grn_ctx_open(0);
  grn_ctx_use(ctx, mrn_db);
  DBUG_VOID_RETURN;
}

ha_mroonga::~ha_mroonga()
{
  DBUG_ENTER("ha_mroonga::~ha_mroonga");
  grn_ctx_fin(ctx);
  DBUG_VOID_RETURN;
}

const char *ha_mroonga::table_type() const
{
  DBUG_ENTER("ha_mroonga::table_type");
  DBUG_RETURN("groonga");
}

const char *ha_mroonga::index_type(uint keynr)
{
  DBUG_ENTER("ha_mroonga::index_type");
  KEY key_info = table->s->key_info[keynr];
  if (key_info.algorithm == HA_KEY_ALG_FULLTEXT) {
    DBUG_RETURN("FULLTEXT");
  } else if (key_info.algorithm == HA_KEY_ALG_HASH) {
    DBUG_RETURN("HASH");
  } else {
    DBUG_RETURN("BTREE");
  }
}

static const char*ha_mroonga_exts[] = {
  NullS
};
const char **ha_mroonga::bas_ext() const
{
  DBUG_ENTER("ha_mroonga::bas_ext");
  DBUG_RETURN(ha_mroonga_exts);
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
  DBUG_ENTER("ha_mroonga::table_flags");
  DBUG_RETURN(ha_mroonga_table_flags);
}

ulong ha_mroonga::index_flags(uint idx, uint part, bool all_parts) const
{
  DBUG_ENTER("ha_mroonga::index_flags");
  KEY key = table_share->key_info[idx];
  if (key.algorithm == HA_KEY_ALG_BTREE | key.algorithm == HA_KEY_ALG_UNDEF) {
    DBUG_RETURN(HA_READ_NEXT | HA_READ_PREV | HA_READ_ORDER | HA_READ_RANGE);
  } else {
    DBUG_RETURN(HA_ONLY_WHOLE_INDEX | HA_KEY_SCAN_NOT_ROR);
  }
}

int ha_mroonga::create(const char *name, TABLE *table, HA_CREATE_INFO *info)
{
  DBUG_ENTER("ha_mroonga::create");
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
        DBUG_RETURN(-1);
      }
    } else {
      // opening existing database
      db_obj = grn_db_open(ctx, db_path);
      if (db_obj == NULL) {
        GRN_LOG(ctx, GRN_LOG_ERROR, "cannot open database (%s)", db_path);
        pthread_mutex_unlock(&db_mutex);
        DBUG_RETURN(-1);
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
      DBUG_RETURN(-1);
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

  char *tbl_path = NULL;           // we don't specify path
  grn_obj *pkey_value_type = NULL; // we don't use this

  tbl_obj = grn_table_create(ctx, tbl_name, tbl_name_len, tbl_path,
                         tbl_flags, pkey_type, pkey_value_type);

  if (tbl_obj == NULL) {
    GRN_LOG(ctx, GRN_LOG_ERROR, "cannot create table: name=%s", tbl_name);
    DBUG_RETURN(-1);
  }

  /* create columns */
  int i;
  uint n_columns = table->s->fields;
  for (i = 0; i < n_columns; i++) {
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
      DBUG_RETURN(-1);
    }
  }

  /* create indexes */
  uint n_keys = table->s->keys;
  char idx_name[MRN_MAX_PATH_SIZE];

  for (i = 0; i < n_keys; i++) {
    if (i == pkeynr) {
      continue; // pkey is already handled
    }

    grn_obj *idx_tbl_obj, *idx_col_obj, *col_obj, *col_type, buf;
    KEY key_info = table->s->key_info[i];

    // must be single column key
    int key_parts = key_info.key_parts;
    if (key_parts != 1) {
      GRN_LOG(ctx, GRN_LOG_ERROR, "complex key is not supported (%s)", db_path);
      DBUG_RETURN(-1);
    }

    mrn_index_name_gen(tbl_name, i, idx_name);

    Field *field = key_info.key_part[0].field;
    const char *col_name = field->field_name;
    int col_name_size = strlen(col_name);
    col_obj = grn_obj_column(ctx, tbl_obj, col_name, col_name_size);
    int mysql_field_type = field->type();
    grn_builtin_type gtype = mrn_get_type(ctx, mysql_field_type);
    col_type = grn_ctx_at(ctx, gtype);
    grn_obj_flags idx_col_flags =
      GRN_OBJ_COLUMN_INDEX | GRN_OBJ_WITH_POSITION | GRN_OBJ_PERSISTENT;

    int key_alg = key_info.algorithm;
    grn_obj_flags idx_tbl_flags;
    if (key_alg == HA_KEY_ALG_FULLTEXT) {
      idx_tbl_flags = GRN_OBJ_TABLE_PAT_KEY | GRN_OBJ_PERSISTENT;
    } else if (key_alg == HA_KEY_ALG_HASH) {
      idx_tbl_flags = GRN_OBJ_TABLE_HASH_KEY | GRN_OBJ_PERSISTENT;
    } else {
      idx_tbl_flags = GRN_OBJ_TABLE_PAT_KEY | GRN_OBJ_PERSISTENT;
    }

    idx_tbl_obj = grn_table_create(ctx, idx_name, strlen(idx_name), NULL,
                                   idx_tbl_flags, col_type, 0);
    if (idx_tbl_obj == NULL) {
      GRN_LOG(ctx, GRN_LOG_ERROR, "cannot create index: name=%s", idx_name);
      grn_obj_remove(ctx, tbl_obj);
      DBUG_RETURN(-1);
    }

    if (key_alg == HA_KEY_ALG_FULLTEXT) {
      grn_info_type info_type = GRN_INFO_DEFAULT_TOKENIZER;
      grn_obj *token_type = grn_ctx_at(ctx, GRN_DB_BIGRAM);
      grn_obj_set_info(ctx, idx_tbl_obj, info_type, token_type);
    }

    idx_col_obj = grn_column_create(ctx, idx_tbl_obj, col_name, col_name_size, NULL,
                                    idx_col_flags, tbl_obj);

    if (idx_col_obj == NULL) {
      GRN_LOG(ctx, GRN_LOG_ERROR, "cannot create index: name=%s, col=%s",
              idx_name, col_name);
      grn_obj_remove(ctx, idx_tbl_obj);
      grn_obj_remove(ctx, tbl_obj);
      DBUG_RETURN(-1);
    }

    grn_id gid = grn_obj_id(ctx, col_obj);
    GRN_TEXT_INIT(&buf, 0);
    GRN_TEXT_SET(ctx, &buf, (char*) &gid, sizeof(grn_id));
    grn_obj_set_info(ctx, idx_col_obj, GRN_INFO_SOURCE, &buf);
    grn_obj_unlink(ctx, &buf);
  }

  /* clean up */
  grn_obj_unlink(ctx, tbl_obj);

  DBUG_RETURN(0);
}

int ha_mroonga::open(const char *name, int mode, uint test_if_locked)
{
  DBUG_ENTER("ha_mroonga::open");
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
      DBUG_RETURN(-1);
    }
    mrn_hash_put(ctx, mrn_hash, db_name, db);
  }
  pthread_mutex_unlock(&db_mutex);
  grn_ctx_use(ctx, db);

  /* open table */
  char tbl_name[MRN_MAX_PATH_SIZE];
  mrn_table_name_gen(name, tbl_name);
  tbl = grn_ctx_get(ctx, tbl_name, strlen(tbl_name));
  if (tbl == NULL) {
    GRN_LOG(ctx, GRN_LOG_ERROR, "cannot open table (%s)", tbl_name);
    DBUG_RETURN(-1);
  }

  /* open columns */
  int n_columns = table->s->fields;
  col = (grn_obj**) malloc(sizeof(grn_obj*) * n_columns);

  int i;
  for (i = 0; i < n_columns; i++) {
    Field *field = table->field[i];
    const char *col_name = field->field_name;
    int col_name_size = strlen(col_name);
    col[i] = grn_obj_column(ctx, tbl, col_name, col_name_size);
    if (col[i] == NULL) {
      GRN_LOG(ctx, GRN_LOG_ERROR, "cannot open table(col) %s(%s)",
              tbl_name, col_name);
      grn_obj_unlink(ctx, tbl);
      DBUG_RETURN(-1);
    }
  }

  /* open indexes */
  char idx_name[MRN_MAX_PATH_SIZE];
  uint n_keys = table->s->keys;
  uint pkeynr = table->s->primary_key;
  if (n_keys > 0) {
    idx_tbl = (grn_obj**) malloc(sizeof(grn_obj*) * n_keys);
    idx_col = (grn_obj**) malloc(sizeof(grn_obj*) * n_keys);
    key_min = (char**) malloc(sizeof(char*) * n_keys);
    key_max = (char**) malloc(sizeof(char*) * n_keys);
  } else {
    idx_tbl = idx_col = NULL;
    key_min = key_max = NULL;
  }

  for (i = 0; i < n_keys; i++) {
    key_min[i] = (char*) malloc(MRN_MAX_KEY_SIZE);
    key_max[i] = (char*) malloc(MRN_MAX_KEY_SIZE);

    if (i == pkeynr) {
      idx_tbl[i] = idx_col[i] = NULL;
      continue;
    }

    mrn_index_name_gen(tbl_name, i, idx_name);
    idx_tbl[i] = grn_ctx_get(ctx, idx_name, strlen(idx_name));

    KEY key_info = table->s->key_info[i];
    Field *field = key_info.key_part[0].field;
    const char *col_name = field->field_name;
    int col_name_size = strlen(col_name);
    idx_col[i] = grn_obj_column(ctx, idx_tbl[i], col_name, col_name_size);
  }
  DBUG_RETURN(0);
}

int ha_mroonga::close()
{
  DBUG_ENTER("ha_mroonga::close");
  thr_lock_delete(&thr_lock);

  int i;
  uint n_keys = table->s->keys;
  uint pkeynr = table->s->primary_key;
  for (i = 0; i < n_keys; i++) {
    free(key_min[i]);
    free(key_max[i]);
    if (i == pkeynr) {
      continue;
    }
    grn_obj_unlink(ctx, idx_tbl[i]);
  }
  grn_obj_unlink(ctx, tbl);

  if (idx_tbl != NULL) {
    free(idx_tbl);
    free(idx_col);
    free(key_min);
    free(key_max);
  }

  free(col);
  DBUG_RETURN(0);
}

int ha_mroonga::delete_table(const char *name)
{
  DBUG_ENTER("ha_mroonga::delete_table");
  char db_path[MRN_MAX_PATH_SIZE];
  char tbl_name[MRN_MAX_PATH_SIZE];
  char idx_name[MRN_MAX_PATH_SIZE];

  grn_obj *db_obj, *tbl_obj, *lex_obj, *hash_obj, *pat_obj;
  mrn_db_path_gen(name, db_path);
  db_obj = grn_db_open(ctx, db_path);
  if (db_obj == NULL) {
    GRN_LOG(ctx, GRN_LOG_ERROR, "grn_db_open failed while delete_table (%s)", db_path);
    DBUG_RETURN(-1);
  }
  grn_ctx_use(ctx, db_obj);

  mrn_table_name_gen(name, tbl_name);

  /* FIXME: remove const 100 */
  int i;
  for (i = 0; i < 100; i++) { // 100 is enough
    mrn_index_name_gen(tbl_name, i, idx_name);
    grn_obj *idx_tbl_obj = grn_ctx_get(ctx, idx_name, strlen(idx_name));
    if (idx_tbl_obj != NULL) {
      grn_obj_remove(ctx, idx_tbl_obj);
    }
  }

  tbl_obj = grn_ctx_get(ctx, tbl_name, strlen(tbl_name));
  if (tbl_obj == NULL) {
    GRN_LOG(ctx, GRN_LOG_ERROR, "grn_ctx_get failed while mrn_drop (%s)", tbl_name);
    DBUG_RETURN(-1);
  }
  DBUG_RETURN(grn_obj_remove(ctx, tbl_obj));
}

int ha_mroonga::info(uint flag)
{
  DBUG_ENTER("ha_mroonga::info");
  ha_rows rows = grn_table_size(ctx, tbl);
  stats.records = rows;
  DBUG_RETURN(0);
}

THR_LOCK_DATA **ha_mroonga::store_lock(THD *thd, THR_LOCK_DATA **to,
                                       enum thr_lock_type lock_type)
{
  DBUG_ENTER("ha_mroonga::store_lock");
  if (lock_type != TL_IGNORE && thr_lock_data.type == TL_UNLOCK) {
    thr_lock_data.type = lock_type;
  }
  *to++ = &thr_lock_data;
  DBUG_RETURN(to);
}

int ha_mroonga::rnd_init(bool scan)
{
  DBUG_ENTER("ha_mroonga::rnd_init");
  cur = grn_table_cursor_open(ctx, tbl, NULL, 0, NULL, 0, 0, -1, 0);
  if (cur == NULL) {
      GRN_LOG(ctx, GRN_LOG_ERROR, "cannot open cursor");
      DBUG_RETURN(-1);
  }
  DBUG_RETURN(0);
}

int ha_mroonga::rnd_next(uchar *buf)
{
  DBUG_ENTER("ha_mroonga::rnd_next");
  row_id = grn_table_cursor_next(ctx, cur);
  if (row_id == GRN_ID_NIL) {
    grn_table_cursor_close(ctx, cur);
    DBUG_RETURN(HA_ERR_END_OF_FILE);
  }
  int i;
  int n_columns = table->s->fields;
  for (i = 0; i < n_columns; i++) {
    Field *field = table->field[i];
    bitmap_set_bit(table->write_set, field->field_index);
    mrn_store_field(ctx, field, col[i], row_id);
  }
  DBUG_RETURN(0);
}

int ha_mroonga::rnd_pos(uchar *buf, uchar *pos)
{
  DBUG_ENTER("ha_mroonga::rnd_pos");
  row_id = *((grn_id*) pos);
  int i;
  int n_columns = table->s->fields;
  for (i = 0; i < n_columns; i++) {
    Field *field = table->field[i];
    bitmap_set_bit(table->write_set, field->field_index);
    mrn_store_field(ctx, field, col[i], row_id);
  }
  DBUG_RETURN(0);
}

void ha_mroonga::position(const uchar *record)
{
  DBUG_ENTER("ha_mroonga::position");
  memcpy(ref, &row_id, sizeof(grn_id));
  DBUG_VOID_RETURN;
}

int ha_mroonga::write_row(uchar *buf)
{
  DBUG_ENTER("ha_mroonga::write_row");
  grn_obj wrapper;
  void *pkey = NULL;
  int pkey_size = 0;
  uint pkeynr = table->s->primary_key;
  GRN_VOID_INIT(&wrapper);
  if (pkeynr != MAX_INDEXES) {
    KEY key_info = table->s->key_info[pkeynr];
    // surpose simgle column key
    int field_no = key_info.key_part[0].field->field_index;
    Field *pkey_field = table->field[field_no];
    bitmap_set_bit(table->read_set, pkey_field->field_index);
    mrn_set_buf(ctx, pkey_field, &wrapper, &pkey_size);
    pkey = GRN_TEXT_VALUE(&wrapper);
  }

  int added;
  row_id = grn_table_add(ctx, tbl, pkey, pkey_size, &added);
  grn_obj_unlink(ctx, &wrapper);
  if (added == 0) {
    // duplicated error
    DBUG_RETURN(-1);
  }

  grn_obj colbuf;
  int i, col_size;
  int n_columns = table->s->fields;
  GRN_VOID_INIT(&colbuf);
  for (i = 0; i < n_columns; i++) {
    Field *field = table->field[i];
    bitmap_set_bit(table->read_set, field->field_index);
    mrn_set_buf(ctx, field, &colbuf, &col_size);
    if (grn_obj_set_value(ctx, col[i], row_id, &colbuf, GRN_OBJ_SET)
        != GRN_SUCCESS) {
      grn_obj_unlink(ctx, &colbuf);
      DBUG_RETURN(-1);
    }
  }
  grn_obj_unlink(ctx, &colbuf);
  DBUG_RETURN(0);
}

int ha_mroonga::update_row(const uchar *old_data, uchar *new_data)
{
  DBUG_ENTER("ha_mroonga::update_row");
  grn_obj colbuf;
  int i, col_size;
  int n_columns = table->s->fields;
  GRN_VOID_INIT(&colbuf);
  for (i = 0; i < n_columns; i++) {
    Field *field = table->field[i];
    bitmap_set_bit(table->read_set, field->field_index);
    mrn_set_buf(ctx, field, &colbuf, &col_size);
    if (grn_obj_set_value(ctx, col[i], row_id, &colbuf, GRN_OBJ_SET)
        != GRN_SUCCESS) {
      grn_obj_unlink(ctx, &colbuf);
      DBUG_RETURN(-1);
    }
  }
  grn_obj_unlink(ctx, &colbuf);
  DBUG_RETURN(0);
}

int ha_mroonga::delete_row(const uchar *buf)
{
  DBUG_ENTER("ha_mroonga::delete_row");
  if (grn_table_delete_by_id(ctx, tbl, row_id) != GRN_SUCCESS) {
    DBUG_RETURN(-1);
  }
  DBUG_RETURN(0);
}

ha_rows ha_mroonga::records_in_range(uint keynr, key_range *range_min, key_range *range_max)
{
  DBUG_ENTER("ha_mroonga::records_in_range");
  int flags = 0;
  uint size_min = 0, size_max = 0;
  ha_rows row_count = 0;
  void *val_min = NULL, *val_max = NULL;
  KEY key_info = table->s->key_info[keynr];
  KEY_PART_INFO key_part = key_info.key_part[0];
  Field *field = key_part.field;
  if (range_min != NULL) {
    mrn_set_key_buf(ctx, field, range_min->key, key_min[keynr], &size_min);
    val_min = key_min[keynr];
    if (range_min->flag & HA_READ_AFTER_KEY) {
      flags |= GRN_CURSOR_GT;
    }
  }
  if (range_max != NULL) {
    mrn_set_key_buf(ctx, field, range_max->key, key_max[keynr], &size_max);
    val_max = key_max[keynr];
    if (range_max->flag & HA_READ_BEFORE_KEY) {
      flags |= GRN_CURSOR_LT;
    }
  }
  uint pkeynr = table->s->primary_key;

  if (keynr == pkeynr) { // primary index
    grn_table_cursor *cur_t =
      grn_table_cursor_open(ctx, tbl, val_min, size_min, val_max, size_max, 0, -1, flags);
    grn_id gid;
    while ((gid = grn_table_cursor_next(ctx, cur_t)) != GRN_ID_NIL) {
      row_count++;
    }
    grn_table_cursor_close(ctx, cur_t);
  } else { // normal index
    uint table_size = grn_table_size(ctx, tbl);
    uint cardinality = grn_table_size(ctx, idx_tbl[keynr]);
    grn_table_cursor *cur_t =
      grn_table_cursor_open(ctx, idx_tbl[keynr], val_min, size_min, val_max, size_max, 0, -1, flags);
    grn_id gid;
    while ((gid = grn_table_cursor_next(ctx, cur_t)) != GRN_ID_NIL) {
      row_count++;
    }
    grn_table_cursor_close(ctx, cur_t);
    row_count = (int) ((double) table_size * ((double) row_count / (double) cardinality));
  }
  DBUG_RETURN(row_count);
}

int ha_mroonga::index_read(uchar * record_buffer, const uchar * key, uint key_len,
                           enum ha_rkey_function find_flag)
{
  DBUG_ENTER("ha_mroonga::index_read");
  uint keynr = active_index;
  uint pkeynr = table->s->primary_key;
  KEY key_info = table->key_info[keynr];
  KEY_PART_INFO key_part = key_info.key_part[0];
  if (keynr == pkeynr) {
    row_id = grn_table_get(ctx, tbl, key, key_len);
    int i;
    int n_columns = table->s->fields;
    for (i = 0; i < n_columns; i++) {
      Field *field = table->field[i];
      if (bitmap_is_set(table->read_set, field->field_index)) {
        bitmap_set_bit(table->write_set, field->field_index);
        mrn_store_field(ctx, field, col[i], row_id);
      }
    }
  }
  DBUG_RETURN(0);
}

int ha_mroonga::index_read_idx(uchar * buf, uint index, const uchar * key,
                               uint key_len, enum ha_rkey_function find_flag)
{
  DBUG_ENTER("ha_mroonga::index_read_idx");
  DBUG_RETURN(0);
}

int ha_mroonga::index_read_last(uchar *buf, const uchar *key, uint key_len)
{
  DBUG_ENTER("ha_mroonga::index_read_last");
  DBUG_RETURN(0);
}

int ha_mroonga::index_next(uchar *buf)
{
  DBUG_ENTER("ha_mroonga::index_next");
  row_id = grn_table_cursor_next(ctx, cur);
  if (row_id == GRN_ID_NIL) {
    grn_table_cursor_close(ctx, cur);
    DBUG_RETURN(HA_ERR_END_OF_FILE);
  }
  int i;
  int n_columns = table->s->fields;
  for (i = 0; i < n_columns; i++) {
    Field *field = table->field[i];
    bitmap_set_bit(table->write_set, field->field_index);
    mrn_store_field(ctx, field, col[i], row_id);
  }
  DBUG_RETURN(0);
}

int ha_mroonga::index_prev(uchar *buf)
{
  DBUG_ENTER("ha_mroonga::index_prev");
  DBUG_RETURN(HA_ERR_END_OF_FILE);
}

int ha_mroonga::index_first(uchar *buf)
{
  DBUG_ENTER("ha_mroonga::index_first");
  DBUG_RETURN(HA_ERR_END_OF_FILE);
}

int ha_mroonga::index_last(uchar *buf)
{
  DBUG_ENTER("ha_mroonga::index_last");
  DBUG_RETURN(HA_ERR_END_OF_FILE);
}

int ha_mroonga::ft_init() {
  DBUG_ENTER("ha_mroonga::ft_init");
  DBUG_RETURN(0);
}

FT_INFO *ha_mroonga::ft_init_ext(uint flags, uint keynr, String *key)
{
  DBUG_ENTER("ha_mroonga::ft_init_ext");
  grn_obj *ft = idx_col[keynr];
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
  DBUG_RETURN(NULL);
}

int ha_mroonga::ft_read(uchar *buf)
{
  DBUG_ENTER("ha_mroonga::ft_read");
  grn_id rid;

  rid = grn_table_cursor_next(ctx, cur);

  if (rid == GRN_ID_NIL) {
    grn_table_cursor_close(ctx, cur);
    grn_obj_unlink(ctx, res);
    DBUG_RETURN(HA_ERR_END_OF_FILE);
  }

  if (table->in_use->lex->select_lex.join->tmp_table_param.sum_func_count > 0) {
    DBUG_RETURN(0);
  }

  grn_table_get_key(ctx, res, rid, &row_id, sizeof(grn_id));

  int i;
  int n_columns = table->s->fields;
  for (i = 0; i < n_columns; i++) {
    Field *field = table->field[i];
    if (bitmap_is_set(table->read_set, field->field_index)) {
      bitmap_set_bit(table->write_set, field->field_index);
      mrn_store_field(ctx, field, col[i], row_id);
    }
  }
  DBUG_RETURN(0);
}

const COND *ha_mroonga::cond_push(const COND *cond)
{
  DBUG_ENTER("ha_mroonga::cond_push");
  DBUG_RETURN(NULL);
}

void ha_mroonga::cond_pop()
{
  DBUG_ENTER("ha_mroonga::cond_pop");
  DBUG_VOID_RETURN;
}

#ifdef __cplusplus
}
#endif
