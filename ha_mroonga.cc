/* 
  Copyright(C) 2010 Tetsuro IKEDA
  Copyright(C) 2010 Kentoku SHIBA

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

#ifdef HAVE_CONFIG_H
#  include "config.h"
/* We need to undefine them because my_config.h defines them. :< */
#  undef VERSION
#  undef PACKAGE
#  undef PACKAGE_BUGREPORT
#  undef PACKAGE_NAME
#  undef PACKAGE_STRING
#  undef PACKAGE_TARNAME
#  undef PACKAGE_VERSION
#endif

#ifdef USE_PRAGMA_IMPLEMENTATION
#pragma implementation
#endif

#define MYSQL_SERVER 1

#ifdef MYSQL51
#include <mysql_priv.h>
#include <mysql/plugin.h>
#else /* MYSQL51 */
#include <sql_priv.h>
#include <sql_class.h>
#include <probes_mysql.h>
#include <sql_plugin.h>
#include <sql_show.h>
#endif
#include <sql_select.h>
#include <ft_global.h>
#include <mysql.h>
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
pthread_mutex_t mrn_log_mutex;
_ft_vft mrn_ft_vft = {
  NULL, // mrn_ft_read_next
  mrn_ft_find_relevance,
  mrn_ft_close_search,
  mrn_ft_get_relevance,
  NULL // mrn_ft_reinit_search
};
handlerton *mrn_hton_ptr;

/* status */
st_mrn_statuses mrn_status_vals;
long mrn_count_skip = 0;
long mrn_fast_order_limit = 0;

/* logging */
const char *mrn_logfile_name = MRN_LOG_FILE_NAME;
FILE *mrn_logfile = NULL;
int mrn_logfile_opened = 0;
grn_log_level mrn_log_level_default = GRN_LOG_DUMP;
ulong mrn_log_level = (ulong) mrn_log_level_default;

void mrn_logger_func(int level, const char *time, const char *title,
                     const char *msg, const char *location, void *func_arg)
{
  const char slev[] = " EACewnid-";
  if (mrn_logfile_opened) {
    pthread_mutex_lock(&mrn_log_mutex);
    fprintf(mrn_logfile, "%s|%c|%08x|%s\n", time,
            *(slev + level), (uint)pthread_self(), msg);
    fflush(mrn_logfile);
    pthread_mutex_unlock(&mrn_log_mutex);
  }
}

grn_logger_info mrn_logger_info = {
  mrn_log_level_default,
  GRN_LOG_TIME|GRN_LOG_MESSAGE,
  mrn_logger_func,
  NULL
};

/* global hashes and mutexes */
HASH mrn_allocated_thds;
pthread_mutex_t mrn_allocated_thds_mutex;
uchar *mrn_allocated_thds_get_key(
  THD *thd,
  size_t *length,
  my_bool not_used __attribute__ ((unused))
) {
  DBUG_ENTER("mrn_allocated_thds_get_key");
  *length = sizeof(THD *);
  DBUG_RETURN((uchar*) thd);
}

/* system functions */

static void mrn_create_status()
{
  DBUG_ENTER("mrn_create_status");
  mrn_status_vals.count_skip = mrn_count_skip;
  mrn_status_vals.fast_order_limit = mrn_fast_order_limit;
  DBUG_VOID_RETURN;
}

struct st_mysql_show_var mrn_statuses[] =
{
  {"count_skip", (char *) &mrn_status_vals.count_skip, SHOW_LONG},
  {"fast_order_limit", (char *) &mrn_status_vals.fast_order_limit, SHOW_LONG},
  {NullS, NullS, SHOW_LONG}
};

static int mrn_show_status(THD *thd, SHOW_VAR *var, char *buff)
{
  DBUG_ENTER("mrn_show_status");
  mrn_create_status();
  var->type = SHOW_ARRAY;
  var->value = (char *) &mrn_statuses;
  DBUG_RETURN(0);
}

struct st_mysql_storage_engine storage_engine_structure =
{ MYSQL_HANDLERTON_INTERFACE_VERSION };

struct st_mysql_show_var mrn_status_variables[] =
{
  {"groonga", (char *) &mrn_show_status, SHOW_FUNC},
  {NullS, NullS, SHOW_LONG}
};

const char *mrn_log_level_type_names[] = { "NONE", "EMERG", "ALERT",
                                           "CRIT", "ERROR", "WARNING",
                                           "NOTICE", "INFO", "DEBUG",
                                           "DUMP", NullS };
TYPELIB mrn_log_level_typelib=
{
  array_elements(mrn_log_level_type_names)-1,
  "mrn_log_level_typelib",
  mrn_log_level_type_names,
  NULL
};

static void mrn_log_level_update(THD *thd, struct st_mysql_sys_var *var,
                                      void *var_ptr, const void *save)
{
  DBUG_ENTER("mrn_log_level_update");
  ulong new_value = *(ulong*) save;
  ulong old_value = mrn_log_level;
  mrn_log_level = new_value;
  mrn_logger_info.max_level = (grn_log_level) mrn_log_level;
  grn_ctx *ctx = grn_ctx_open(0);
  GRN_LOG(ctx, GRN_LOG_NOTICE, "log level changed from '%s' to '%s'", 
          mrn_log_level_type_names[old_value],
          mrn_log_level_type_names[new_value]);
  grn_ctx_fin(ctx);
  DBUG_VOID_RETURN;
}

static MYSQL_SYSVAR_ENUM(log_level, mrn_log_level,
                         PLUGIN_VAR_RQCMDARG,
                         "logging level",
                         NULL,
                         mrn_log_level_update,
                         (ulong) mrn_log_level,
                         &mrn_log_level_typelib);

struct st_mysql_sys_var *mrn_system_variables[] =
{
  MYSQL_SYSVAR(log_level),
  NULL
};

/* UDF - last_insert_grn_id() */
my_bool last_insert_grn_id_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
  if (args->arg_count != 0) {
    strcpy(message, "last_insert_grn_id must not have arguments");
    return 1;
  }
  initid->maybe_null = 0;
  return 0;
}

int last_insert_grn_id(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error)
{
  THD *thd = current_thd;
  st_mrn_slot_data *slot_data = (st_mrn_slot_data*) *thd_ha_data(thd, mrn_hton_ptr);
  if (slot_data == NULL) {
    return 0;
  }
  int last_insert_rid = (int) slot_data->last_insert_rid;
  return last_insert_rid;
}

void last_insert_grn_id_deinit(UDF_INIT *initid)
{
}

/* Groonga information schema */
int GROONGA_VERSION_SHORT = 0x0001;
static const char plugin_author[] = "Yoshinori Matsunobu";
static struct st_mysql_information_schema i_s_info =
{
  MYSQL_INFORMATION_SCHEMA_INTERFACE_VERSION
};

static ST_FIELD_INFO i_s_groonga_stats_fields_info[] =
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
  TABLE* table = (TABLE *) tables->table;
  int status = 0;
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
  "groonga_stats",
  plugin_author,
  "Statistics for groonga",
  0,
  i_s_groonga_stats_init,
  i_s_groonga_stats_deinit,
  0x0001,
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
  hton->close_connection = mrn_close_connection;
  hton->flush_logs = mrn_flush_logs;
  mrn_hton_ptr = hton;

  // init groonga
  if (grn_init() != GRN_SUCCESS) {
    goto err;
  }

  ctx = grn_ctx_open(0);

  if (pthread_mutex_init(&mrn_log_mutex, NULL) != 0) {
    goto err_log_mutex_init;
  }
  grn_logger_info_set(ctx, &mrn_logger_info);
  if (!(mrn_logfile = fopen(mrn_logfile_name, "a"))) {
    goto err;
  }
  mrn_logfile_opened = 1;
  GRN_LOG(ctx, GRN_LOG_NOTICE, "%s started.", MRN_PACKAGE_STRING);
  GRN_LOG(ctx, GRN_LOG_NOTICE, "log level is '%s'",
          mrn_log_level_type_names[mrn_log_level]);

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
    goto err_db_mutex_init;
  }
  if ((pthread_mutex_init(&mrn_allocated_thds_mutex, NULL) != 0)) {
    goto err_allocated_thds_mutex_init;
  }
  if(
    hash_init(&mrn_allocated_thds, system_charset_info, 32, 0, 0,
                   (hash_get_key) mrn_allocated_thds_get_key, 0, 0)
  ) {
    goto error_allocated_thds_hash_init;
  }

  grn_ctx_fin(ctx);
  return 0;

error_allocated_thds_hash_init:
  pthread_mutex_destroy(&mrn_allocated_thds_mutex);
err_allocated_thds_mutex_init:
  pthread_mutex_destroy(&db_mutex);
err_db_mutex_init:
err:
  pthread_mutex_destroy(&mrn_log_mutex);
err_log_mutex_init:
  grn_ctx_fin(ctx);
  grn_fin();
  return -1;
}

int mrn_deinit(void *p)
{
  THD *thd = current_thd, *tmp_thd;
  grn_ctx *ctx;
  ctx = grn_ctx_open(0);

  GRN_LOG(ctx, GRN_LOG_NOTICE, "%s deinit", MRN_PACKAGE_STRING);

  if (thd && thd_sql_command(thd) == SQLCOM_UNINSTALL_PLUGIN) {
    pthread_mutex_lock(&mrn_allocated_thds_mutex);
    while ((tmp_thd = (THD *) hash_element(&mrn_allocated_thds, 0)))
    {
      void *slot_ptr = *thd_ha_data(tmp_thd, mrn_hton_ptr);
      if (slot_ptr) free(slot_ptr);
      *thd_ha_data(tmp_thd, mrn_hton_ptr) = (void *) NULL;
      hash_delete(&mrn_allocated_thds, (uchar *) tmp_thd);
    }
    pthread_mutex_unlock(&mrn_allocated_thds_mutex);
  }

  hash_free(&mrn_allocated_thds);
  pthread_mutex_destroy(&mrn_allocated_thds_mutex);
  pthread_mutex_destroy(&mrn_log_mutex);
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

int mrn_close_connection(handlerton *hton, THD *thd)
{
  void *p = *thd_ha_data(thd, mrn_hton_ptr);
  if (p) {
    free(p);
    *thd_ha_data(thd, mrn_hton_ptr) = (void *) NULL;
    pthread_mutex_lock(&mrn_allocated_thds_mutex);
    hash_delete(&mrn_allocated_thds, (uchar*) thd);
    pthread_mutex_unlock(&mrn_allocated_thds_mutex);
  }
  return 0;
}

bool mrn_flush_logs(handlerton *hton)
{
  bool result = 0;
  if (mrn_logfile_opened) {
    pthread_mutex_lock(&mrn_log_mutex);
    fclose(mrn_logfile);
    mrn_logfile = fopen(mrn_logfile_name, "a");
    pthread_mutex_unlock(&mrn_log_mutex);
  }
  return result;
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

float mrn_ft_find_relevance(FT_INFO *handler, uchar *record, uint length)
{
  st_mrn_ft_info *info = (st_mrn_ft_info*) handler;
  if (info->rid != GRN_ID_NIL) {
    grn_ctx *ctx = info->ctx;
    grn_obj *res = info->res;
    grn_id rid = info->rid;

    if (res && res->header.flags & GRN_OBJ_WITH_SUBREC) {
      float score;
      grn_id res_id = grn_table_get(ctx, res, &rid, sizeof(rid));
      if (res_id != GRN_ID_NIL) {
        return (float) -1.0;
      } else {
        return (float) 0.0;
      }
    }
  }
  return (float) -1.0;
}

float mrn_ft_get_relevance(FT_INFO *handler)
{
  return (float) -1.0;
}

void mrn_ft_close_search(FT_INFO *handler)
{
  st_mrn_ft_info *info = (st_mrn_ft_info*) handler;
  info->ctx = NULL;
  info->res = NULL;
  info->rid = GRN_ID_NIL;
}

/* handler implementation */
ha_mroonga::ha_mroonga(handlerton *hton, TABLE_SHARE *share)
  :handler(hton, share)
{
  DBUG_ENTER("ha_mroonga::ha_mroonga");
  ctx = grn_ctx_open(0);
  grn_ctx_use(ctx, mrn_db);
  cur = NULL;
  cur0 = NULL;
  res = NULL;
  res0 = NULL;
  sort_keys = NULL;
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
    HA_STATS_RECORDS_IS_EXACT |
    HA_NO_PREFIX_CHAR_KEYS |
    HA_CAN_FULLTEXT |
    HA_NO_AUTO_INCREMENT |
    HA_CAN_INSERT_DELAYED |
    HA_BINLOG_FLAGS |
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
  /* checking data type of virtual columns */
  int i;
  uint n_columns = table->s->fields;
  for (i = 0; i < n_columns; i++) {
    Field *field = table->s->field[i];
    const char *col_name = field->field_name;
    int col_name_size = strlen(col_name);
    if (strncmp(MRN_ID_COL_NAME, col_name, col_name_size) == 0) {
      switch (field->type()) {
      case (MYSQL_TYPE_TINY) :
      case (MYSQL_TYPE_SHORT) :
      case (MYSQL_TYPE_INT24) :
      case (MYSQL_TYPE_LONG) :
      case (MYSQL_TYPE_LONGLONG) :
        break;
      default:
        GRN_LOG(ctx, GRN_LOG_ERROR, "_id must be numeric data type");
        my_message(ER_CANT_CREATE_TABLE, "_id must be numeric data type", MYF(0));
        DBUG_RETURN(ER_CANT_CREATE_TABLE);
      }        
    }
    if (strncmp(MRN_SCORE_COL_NAME, col_name, col_name_size) == 0) {
      switch (field->type()) {
      case (MYSQL_TYPE_FLOAT) :
      case (MYSQL_TYPE_DOUBLE) :
        break;
      default:
        GRN_LOG(ctx, GRN_LOG_ERROR, "_score must be float or double");
        my_message(ER_CANT_CREATE_TABLE, "_score must be float or double", MYF(0));
        DBUG_RETURN(ER_CANT_CREATE_TABLE);
      }        
    }
  }

  /* checking if index is used for virtual columns  */
  uint n_keys = table->s->keys;
  for (i = 0; i < n_keys; i++) {
    KEY key_info = table->s->key_info[i];
    // must be single column key
    int key_parts = key_info.key_parts;
    if (key_parts != 1) {
      GRN_LOG(ctx, GRN_LOG_ERROR, "complex key is not supported yet");
      my_message(ER_NOT_SUPPORTED_YET, "complex key is not supported yet.", MYF(0));
      DBUG_RETURN(ER_NOT_SUPPORTED_YET);
    }
    Field *field = key_info.key_part[0].field;
    const char *col_name = field->field_name;
    int col_name_size = strlen(col_name);
    if (strncmp(MRN_ID_COL_NAME, col_name, col_name_size) == 0) {
      if (key_info.algorithm == HA_KEY_ALG_HASH) {
        continue; // hash index is ok
      }
      GRN_LOG(ctx, GRN_LOG_ERROR, "only hash index can be defined for _id");
      my_message(ER_CANT_CREATE_TABLE, "only hash index can be defined for _id", MYF(0));
      DBUG_RETURN(ER_CANT_CREATE_TABLE);
    }
    if (strncmp(MRN_SCORE_COL_NAME, col_name, col_name_size) == 0) {
      GRN_LOG(ctx, GRN_LOG_ERROR, "_score cannot be used for index");
      my_message(ER_CANT_CREATE_TABLE, "_score cannot be used for index", MYF(0));
      DBUG_RETURN(ER_CANT_CREATE_TABLE);  
    }
  }

  /* before creating table, we must check if database is alreadly opened, created */
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
      if (ctx->rc) {
        pthread_mutex_unlock(&db_mutex);
        my_message(ER_CANT_CREATE_FILE, ctx->errbuf, MYF(0));
        DBUG_RETURN(ER_CANT_CREATE_FILE);
      }
    } else {
      // opening existing database
      db_obj = grn_db_open(ctx, db_path);
      if (ctx->rc) {
        pthread_mutex_unlock(&db_mutex);
        my_message(ER_CANT_OPEN_FILE, ctx->errbuf, MYF(0));
        DBUG_RETURN(ER_CANT_OPEN_FILE);
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
      GRN_LOG(ctx, GRN_LOG_ERROR, "complex key is not supported yet");
      my_message(ER_NOT_SUPPORTED_YET, "complex key is not supported yet", MYF(0));
      DBUG_RETURN(ER_NOT_SUPPORTED_YET);
    }
    Field *pkey_field = key_info.key_part[0].field;
    const char *col_name = pkey_field->field_name;
    int col_name_size = strlen(col_name);
    bool is_id = (strncmp(MRN_ID_COL_NAME, col_name, col_name_size) == 0);

    int mysql_field_type = pkey_field->type();
    grn_builtin_type gtype = mrn_get_type(ctx, mysql_field_type);
    pkey_type = grn_ctx_at(ctx, gtype);

    // default algorithm is BTREE ==> PAT
    if (!is_id && key_info.algorithm == HA_KEY_ALG_HASH) {
      tbl_flags |= GRN_OBJ_TABLE_HASH_KEY;
    } else if (!is_id) {
      tbl_flags |= GRN_OBJ_TABLE_PAT_KEY;
    } else {
      // for _id 
      tbl_flags |= GRN_OBJ_TABLE_NO_KEY;
      pkey_type = NULL;
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
  if (ctx->rc) {
    my_message(ER_CANT_CREATE_TABLE, ctx->errbuf, MYF(0));
    DBUG_RETURN(ER_CANT_CREATE_TABLE);
  }

  /* create columns */
  for (i = 0; i < n_columns; i++) {
    grn_obj *col_obj, *col_type;
    Field *field = table->s->field[i];
    const char *col_name = field->field_name;
    int col_name_size = strlen(col_name);

    if (strncmp(MRN_ID_COL_NAME, col_name, col_name_size) == 0) continue;
    if (strncmp(MRN_SCORE_COL_NAME, col_name, col_name_size) == 0) continue;

    grn_obj_flags col_flags = GRN_OBJ_PERSISTENT | GRN_OBJ_COLUMN_SCALAR;
    int mysql_field_type = field->type();
    grn_builtin_type gtype = mrn_get_type(ctx, mysql_field_type);
    col_type = grn_ctx_at(ctx, gtype);
    char *col_path = NULL; // we don't specify path

    col_obj = grn_column_create(ctx, tbl_obj, col_name, col_name_size,
                                col_path, col_flags, col_type);
    if (ctx->rc) {
      grn_obj_remove(ctx, tbl_obj);
      my_message(ER_CANT_CREATE_TABLE, ctx->errbuf, MYF(0));
      DBUG_RETURN(ER_CANT_CREATE_TABLE);
    }
  }

  /* create indexes */
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
      GRN_LOG(ctx, GRN_LOG_ERROR, "complex key is not supported yet");
      my_message(ER_NOT_SUPPORTED_YET, "complex key is not supported yet.", MYF(0));
      DBUG_RETURN(ER_NOT_SUPPORTED_YET);
    }

    mrn_index_name_gen(tbl_name, i, idx_name);

    Field *field = key_info.key_part[0].field;
    const char *col_name = field->field_name;
    int col_name_size = strlen(col_name);

    if (strncmp(MRN_ID_COL_NAME, col_name, col_name_size) == 0) {
      // skipping _id virtual column
      continue;
    }

    col_obj = grn_obj_column(ctx, tbl_obj, col_name, col_name_size);
    int mysql_field_type = field->type();
    grn_builtin_type gtype = mrn_get_type(ctx, mysql_field_type);
    col_type = grn_ctx_at(ctx, gtype);
    grn_obj_flags idx_col_flags =
      GRN_OBJ_COLUMN_INDEX | GRN_OBJ_WITH_POSITION | GRN_OBJ_PERSISTENT;

    int key_alg = key_info.algorithm;
    grn_obj_flags idx_tbl_flags;
    if (key_alg == HA_KEY_ALG_FULLTEXT) {
      idx_tbl_flags = GRN_OBJ_TABLE_PAT_KEY | GRN_OBJ_PERSISTENT | GRN_OBJ_KEY_NORMALIZE;
    } else if (key_alg == HA_KEY_ALG_HASH) {
      idx_tbl_flags = GRN_OBJ_TABLE_HASH_KEY | GRN_OBJ_PERSISTENT | GRN_OBJ_KEY_NORMALIZE;
    } else {
      idx_tbl_flags = GRN_OBJ_TABLE_PAT_KEY | GRN_OBJ_PERSISTENT | GRN_OBJ_KEY_NORMALIZE;
    }

    idx_tbl_obj = grn_table_create(ctx, idx_name, strlen(idx_name), NULL,
                                   idx_tbl_flags, col_type, 0);
    if (ctx->rc) {
      grn_obj_remove(ctx, tbl_obj);
      my_message(ER_CANT_CREATE_TABLE, ctx->errbuf, MYF(0));
      DBUG_RETURN(ER_CANT_CREATE_TABLE);
    }

    if (key_alg == HA_KEY_ALG_FULLTEXT) {
      grn_info_type info_type = GRN_INFO_DEFAULT_TOKENIZER;
      grn_obj *token_type = grn_ctx_at(ctx, GRN_DB_BIGRAM);
      grn_obj_set_info(ctx, idx_tbl_obj, info_type, token_type);
    }

    idx_col_obj = grn_column_create(ctx, idx_tbl_obj, col_name, col_name_size, NULL,
                                    idx_col_flags, tbl_obj);

    if (ctx->rc) {
      grn_obj_remove(ctx, idx_tbl_obj);
      grn_obj_remove(ctx, tbl_obj);
      my_message(ER_CANT_CREATE_TABLE, ctx->errbuf, MYF(0));
      DBUG_RETURN(ER_CANT_CREATE_TABLE);
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
    if (ctx->rc) {
      pthread_mutex_unlock(&db_mutex);
      my_message(ER_CANT_OPEN_FILE, ctx->errbuf, MYF(0));
      DBUG_RETURN(ER_CANT_OPEN_FILE);
    }
    mrn_hash_put(ctx, mrn_hash, db_name, db);
  }
  pthread_mutex_unlock(&db_mutex);
  grn_ctx_use(ctx, db);

  /* open table */
  char tbl_name[MRN_MAX_PATH_SIZE];
  mrn_table_name_gen(name, tbl_name);
  tbl = grn_ctx_get(ctx, tbl_name, strlen(tbl_name));
  if (ctx->rc) {
    my_message(ER_CANT_OPEN_FILE, ctx->errbuf, MYF(0));
    DBUG_RETURN(ER_CANT_OPEN_FILE);
  }

  /* open columns */
  int n_columns = table->s->fields;
  col = (grn_obj**) malloc(sizeof(grn_obj*) * n_columns);

  int i;
  for (i = 0; i < n_columns; i++) {
    Field *field = table->field[i];
    const char *col_name = field->field_name;
    int col_name_size = strlen(col_name);

    if (strncmp(MRN_ID_COL_NAME, col_name, col_name_size) == 0) {
      col[i] = NULL;
      continue;
    }
    if (strncmp(MRN_SCORE_COL_NAME, col_name, col_name_size) == 0) {
      col[i] = NULL;
      continue;
    }

    col[i] = grn_obj_column(ctx, tbl, col_name, col_name_size);
    if (ctx->rc) {
      grn_obj_unlink(ctx, tbl);
      my_message(ER_CANT_OPEN_FILE, ctx->errbuf, MYF(0));
      DBUG_RETURN(ER_CANT_OPEN_FILE);
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
    if (ctx->rc) {
      grn_obj_unlink(ctx, tbl);
      my_message(ER_CANT_OPEN_FILE, ctx->errbuf, MYF(0));
      DBUG_RETURN(ER_CANT_OPEN_FILE);
    }

    KEY key_info = table->s->key_info[i];
    Field *field = key_info.key_part[0].field;
    const char *col_name = field->field_name;
    int col_name_size = strlen(col_name);
    idx_col[i] = grn_obj_column(ctx, idx_tbl[i], col_name, col_name_size);
    if (ctx->rc) {
      grn_obj_unlink(ctx, idx_tbl[i]);
      grn_obj_unlink(ctx, tbl);
      my_message(ER_CANT_OPEN_FILE, ctx->errbuf, MYF(0));
      DBUG_RETURN(ER_CANT_OPEN_FILE);
    }
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
  if (ctx->rc) {
    my_message(ER_CANT_OPEN_FILE, ctx->errbuf, MYF(0));
    DBUG_RETURN(ER_CANT_OPEN_FILE);
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
  if (ctx->rc) {
    my_message(ER_CANT_OPEN_FILE, ctx->errbuf, MYF(0));
    DBUG_RETURN(ER_CANT_OPEN_FILE);
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
  count_skip = FALSE;
  cur = grn_table_cursor_open(ctx, tbl, NULL, 0, NULL, 0, 0, -1, 0);
  if (ctx->rc) {
    my_message(ER_ERROR_ON_READ, ctx->errbuf, MYF(0));
    DBUG_RETURN(ER_ERROR_ON_READ);
  }
  DBUG_RETURN(0);
}

int ha_mroonga::rnd_end()
{
  DBUG_ENTER("ha_mroonga::rnd_end");
  if (cur) {
    grn_table_cursor_close(ctx, cur);
    cur = NULL;
  }
  if (cur0) {
    grn_table_cursor_close(ctx, cur0);
    cur0 = NULL;
  }
  DBUG_RETURN(0);
}

int ha_mroonga::rnd_next(uchar *buf)
{
  DBUG_ENTER("ha_mroonga::rnd_next");
  row_id = grn_table_cursor_next(ctx, cur);
  if (ctx->rc) {
    my_message(ER_ERROR_ON_READ, ctx->errbuf, MYF(0));
    grn_table_cursor_close(ctx, cur);
    cur = NULL;
    DBUG_RETURN(ER_ERROR_ON_READ);
  }
  if (row_id == GRN_ID_NIL) {
    grn_table_cursor_close(ctx, cur);
    cur = NULL;
    table->status = STATUS_NOT_FOUND;
    DBUG_RETURN(HA_ERR_END_OF_FILE);
  }
  store_fields_from_primary_table(buf, row_id);
  table->status = 0;

  DBUG_RETURN(0);
}

int ha_mroonga::rnd_pos(uchar *buf, uchar *pos)
{
  DBUG_ENTER("ha_mroonga::rnd_pos");
  row_id = *((grn_id*) pos);
  store_fields_from_primary_table(buf, row_id);
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
  THD *thd = ha_thd();
  int i, col_size;
  int n_columns = table->s->fields;
#ifndef DBUG_OFF
  my_bitmap_map *tmp_map = dbug_tmp_use_all_columns(table, table->read_set);
#endif
  if (thd->abort_on_warning) {
    for (i = 0; i < n_columns; i++) {
      Field *field = table->field[i];
      const char *col_name = field->field_name;
      int col_name_size = strlen(col_name);

      if (field->is_null()) continue;

      if (strncmp(MRN_ID_COL_NAME, col_name, col_name_size) == 0) {
#ifndef DBUG_OFF
        dbug_tmp_restore_column_map(table->read_set, tmp_map);
#endif
        my_message(ER_DATA_TOO_LONG, "cannot insert value to _id column", MYF(0));
        DBUG_RETURN(ER_DATA_TOO_LONG);
      } 
      if (strncmp(MRN_SCORE_COL_NAME, col_name, col_name_size) == 0) {
#ifndef DBUG_OFF
        dbug_tmp_restore_column_map(table->read_set, tmp_map);
#endif
        my_message(ER_DATA_TOO_LONG, "cannot insert value to _score column", MYF(0));
        DBUG_RETURN(ER_DATA_TOO_LONG);
      } 
    }
  }

  GRN_VOID_INIT(&wrapper);
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
  if (ctx->rc) {
    my_message(ER_ERROR_ON_WRITE, ctx->errbuf, MYF(0));
    DBUG_RETURN(ER_ERROR_ON_WRITE);
  }
  grn_obj_unlink(ctx, &wrapper);
  if (added == 0) {
    // duplicated error
#ifndef DBUG_OFF
    dbug_tmp_restore_column_map(table->read_set, tmp_map);
#endif
    GRN_LOG(ctx, GRN_LOG_ERROR, "duplicated _id on insert");
    my_message(ER_DUP_UNIQUE, "duplicated _id on insert", MYF(0));
    DBUG_RETURN(ER_DUP_UNIQUE);
  }

  grn_obj colbuf;
  GRN_VOID_INIT(&colbuf);
  for (i = 0; i < n_columns; i++) {
    Field *field = table->field[i];
    const char *col_name = field->field_name;
    int col_name_size = strlen(col_name);

    if (field->is_null()) continue;

    if (strncmp(MRN_ID_COL_NAME, col_name, col_name_size) == 0) {
      push_warning(thd, MYSQL_ERROR::WARN_LEVEL_WARN, WARN_DATA_TRUNCATED,
                   "data truncated for _id column");
      continue;
    }

    if (strncmp(MRN_SCORE_COL_NAME, col_name, col_name_size) == 0) {
      push_warning(thd, MYSQL_ERROR::WARN_LEVEL_WARN, WARN_DATA_TRUNCATED,
                   "data truncated for _score column");
      continue;
    }

    mrn_set_buf(ctx, field, &colbuf, &col_size);
    grn_obj_set_value(ctx, col[i], row_id, &colbuf, GRN_OBJ_SET);
    if (ctx->rc) {
#ifndef DBUG_OFF
      dbug_tmp_restore_column_map(table->read_set, tmp_map);
#endif
      grn_obj_unlink(ctx, &colbuf);
      my_message(ER_ERROR_ON_WRITE, ctx->errbuf, MYF(0));
      DBUG_RETURN(ER_ERROR_ON_WRITE);
    }
  }
#ifndef DBUG_OFF
  dbug_tmp_restore_column_map(table->read_set, tmp_map);
#endif
  grn_obj_unlink(ctx, &colbuf);

  // for UDF last_insert_grn_id()
  st_mrn_slot_data *slot_data = (st_mrn_slot_data*) *thd_ha_data(thd, mrn_hton_ptr);
  if (slot_data == NULL) {
    slot_data = (st_mrn_slot_data*) malloc(sizeof(st_mrn_slot_data)); 
    *thd_ha_data(thd, mrn_hton_ptr) = (void *) slot_data;
    pthread_mutex_lock(&mrn_allocated_thds_mutex);
    if (my_hash_insert(&mrn_allocated_thds, (uchar*) thd))
    {
      DBUG_RETURN(HA_ERR_OUT_OF_MEM);
    }
    pthread_mutex_unlock(&mrn_allocated_thds_mutex);
  }
  slot_data->last_insert_rid = row_id;

  DBUG_RETURN(0);
}

int ha_mroonga::update_row(const uchar *old_data, uchar *new_data)
{
  DBUG_ENTER("ha_mroonga::update_row");
  grn_obj colbuf;
  int i, col_size;
  int n_columns = table->s->fields;
  THD *thd = ha_thd();

  if (thd->abort_on_warning) {
    for (i = 0; i < n_columns; i++) {
      Field *field = table->field[i];
      const char *col_name = field->field_name;
      int col_name_size = strlen(col_name);

      if (bitmap_is_set(table->write_set, field->field_index)) {
        if (field->is_null()) continue;
        if (strncmp(MRN_ID_COL_NAME, col_name, col_name_size) == 0) {
          my_message(ER_DATA_TOO_LONG, "cannot update value to _id column", MYF(0));
          DBUG_RETURN(ER_DATA_TOO_LONG);
        } 
        if (strncmp(MRN_SCORE_COL_NAME, col_name, col_name_size) == 0) {
          my_message(ER_DATA_TOO_LONG, "cannot update value to _score column", MYF(0));
          DBUG_RETURN(ER_DATA_TOO_LONG);
        }
      }
    }
  }

  GRN_VOID_INIT(&colbuf);
  for (i = 0; i < n_columns; i++) {
    Field *field = table->field[i];
    const char *col_name = field->field_name;
    int col_name_size = strlen(col_name);
    if (bitmap_is_set(table->write_set, field->field_index)) {
#ifndef DBUG_OFF
      my_bitmap_map *tmp_map = dbug_tmp_use_all_columns(table, table->read_set);
#endif
      DBUG_PRINT("info",("mroonga update column %d(%d)",i,field->field_index));

      if (field->is_null()) continue;

      if (strncmp(MRN_ID_COL_NAME, col_name, col_name_size) == 0) {
        push_warning(thd, MYSQL_ERROR::WARN_LEVEL_WARN, WARN_DATA_TRUNCATED,
                     "data truncated for  _id column");
#ifndef DBUG_OFF
        dbug_tmp_restore_column_map(table->read_set, tmp_map);
#endif
        continue;
      }

      if (strncmp(MRN_SCORE_COL_NAME, col_name, col_name_size) == 0) {
        push_warning(thd, MYSQL_ERROR::WARN_LEVEL_WARN, WARN_DATA_TRUNCATED,
                     "data truncated for  _score column");
#ifndef DBUG_OFF
        dbug_tmp_restore_column_map(table->read_set, tmp_map);
#endif
        continue;
      }

      mrn_set_buf(ctx, field, &colbuf, &col_size);
      grn_obj_set_value(ctx, col[i], row_id, &colbuf, GRN_OBJ_SET);
      if (ctx->rc) {
#ifndef DBUG_OFF
        dbug_tmp_restore_column_map(table->read_set, tmp_map);
#endif
        grn_obj_unlink(ctx, &colbuf);
        my_message(ER_ERROR_ON_WRITE, ctx->errbuf, MYF(0));
        DBUG_RETURN(ER_ERROR_ON_WRITE);
      }
#ifndef DBUG_OFF
      dbug_tmp_restore_column_map(table->read_set, tmp_map);
#endif
    }
  }
  grn_obj_unlink(ctx, &colbuf);
  DBUG_RETURN(0);
}

int ha_mroonga::delete_row(const uchar *buf)
{
  DBUG_ENTER("ha_mroonga::delete_row");
  grn_table_delete_by_id(ctx, tbl, row_id);
  if (ctx->rc) {
    my_message(ER_ERROR_ON_WRITE, ctx->errbuf, MYF(0));
    DBUG_RETURN(ER_ERROR_ON_WRITE);
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
  const char *col_name = field->field_name;
  int col_name_size = strlen(col_name);

  if (strncmp(MRN_ID_COL_NAME, col_name, col_name_size) == 0) {
    DBUG_RETURN((ha_rows) 1) ;
  }

  if (range_min != NULL) {
    mrn_set_key_buf(ctx, field, range_min->key, key_min[keynr], &size_min);
    val_min = key_min[keynr];
    if (range_min->flag == HA_READ_AFTER_KEY) {
      flags |= GRN_CURSOR_GT;
    }
  }
  if (range_max != NULL) {
    mrn_set_key_buf(ctx, field, range_max->key, key_max[keynr], &size_max);
    val_max = key_max[keynr];
    if (range_max->flag == HA_READ_BEFORE_KEY) {
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
    grn_table_cursor *cur_t0 =
      grn_table_cursor_open(ctx, idx_tbl[keynr], val_min, size_min, val_max, size_max, 0, -1, flags);
    grn_table_cursor *cur_t =
      grn_index_cursor_open(ctx, cur_t0, idx_col[keynr], 0, GRN_ID_MAX, 0);
    grn_id gid;
    while ((gid = grn_table_cursor_next(ctx, cur_t)) != GRN_ID_NIL) {
      row_count++;
    }
    grn_table_cursor_close(ctx, cur_t);
    grn_table_cursor_close(ctx, cur_t0);
    row_count = (int) ((double) table_size * ((double) row_count / (double) cardinality));
  }
  DBUG_RETURN(row_count);
}

int ha_mroonga::index_init(uint idx, bool sorted)
{
  DBUG_ENTER("ha_mroonga::index_init");
  active_index = idx;
  count_skip = FALSE;
  DBUG_RETURN(0);
}

int ha_mroonga::index_end()
{
  DBUG_ENTER("ha_mroonga::index_end");
  if (cur) {
    grn_table_cursor_close(ctx, cur);
    cur = NULL;
  }
  if (cur0) {
    grn_table_cursor_close(ctx, cur0);
    cur0 = NULL;
  }
  DBUG_RETURN(0);
}

int ha_mroonga::index_read_map(uchar * buf, const uchar * key,
                               key_part_map keypart_map,
                               enum ha_rkey_function find_flag)
{
  DBUG_ENTER("ha_mroonga::index_read_map");
  uint keynr = active_index;
  KEY key_info = table->key_info[keynr];
  KEY_PART_INFO key_part = key_info.key_part[0];
  check_count_skip(keypart_map, 0, FALSE);

  int flags = 0;
  uint size_min = 0, size_max = 0;
  void *val_min = NULL, *val_max = NULL;
  Field *field = key_part.field;
  const char *col_name = field->field_name;
  int col_name_size = strlen(col_name);

  if (cur) {
    grn_table_cursor_close(ctx, cur);
    cur = NULL;
  }
  if (cur0) {
    grn_table_cursor_close(ctx, cur0);
    cur0 = NULL;
  }

  if (find_flag == HA_READ_KEY_EXACT) {
    mrn_set_key_buf(ctx, field, key, key_min[keynr], &size_min);
    val_min = key_min[keynr];
    val_max = key_min[keynr];
    size_max = size_min;

    // for _id
    if (strncmp(MRN_ID_COL_NAME, col_name, col_name_size) == 0) {
      grn_id rid = *(grn_id*) key_min[keynr];
      if (grn_table_at(ctx, tbl, rid) != GRN_ID_NIL) { // found
        store_fields_from_primary_table(buf, rid);
        table->status = 0;
        cur = NULL;
        row_id = rid;
        DBUG_RETURN(0);
      } else {
        table->status = STATUS_NOT_FOUND;
        cur = NULL;
        DBUG_RETURN(HA_ERR_END_OF_FILE);
      }
    }

  } else if (
    find_flag == HA_READ_BEFORE_KEY ||
    find_flag == HA_READ_PREFIX_LAST_OR_PREV
  ) {
    mrn_set_key_buf(ctx, field, key, key_max[keynr], &size_max);
    val_max = key_max[keynr];
    if (find_flag == HA_READ_BEFORE_KEY) {
      flags |= GRN_CURSOR_LT;
    }
  } else {
    mrn_set_key_buf(ctx, field, key, key_min[keynr], &size_min);
    val_min = key_min[keynr];
    if (find_flag == HA_READ_AFTER_KEY) {
      flags |= GRN_CURSOR_GT;
    }
  }

  uint pkeynr = table->s->primary_key;

  if (keynr == pkeynr) { // primary index
    DBUG_PRINT("info",("mroonga use primary key"));
    cur =
      grn_table_cursor_open(ctx, tbl, val_min, size_min, val_max, size_max,
                            0, -1, flags);
  } else { // normal index
    DBUG_PRINT("info",("mroonga use key%u", keynr));
    cur0 =
      grn_table_cursor_open(ctx, idx_tbl[keynr], val_min, size_min,
                            val_max, size_max, 0, -1, flags);
    cur =
      grn_index_cursor_open(ctx, cur0, idx_col[keynr], 0, GRN_ID_MAX, 0);
  }
  if (ctx->rc) {
    my_message(ER_ERROR_ON_READ, ctx->errbuf, MYF(0));
    DBUG_RETURN(ER_ERROR_ON_READ);
  }
  row_id = grn_table_cursor_next(ctx, cur);
  if (ctx->rc) {
    my_message(ER_ERROR_ON_READ, ctx->errbuf, MYF(0));
    grn_table_cursor_close(ctx, cur);
    cur = NULL;
    DBUG_RETURN(ER_ERROR_ON_READ);
  }
  if (row_id == GRN_ID_NIL) {
    grn_table_cursor_close(ctx, cur);
    cur = NULL;
    table->status = STATUS_NOT_FOUND;
    DBUG_RETURN(HA_ERR_END_OF_FILE);
  }
  store_fields_from_primary_table(buf, row_id);
  table->status = 0;
  DBUG_RETURN(0);
}

int ha_mroonga::index_read_last_map(uchar *buf, const uchar *key,
                                    key_part_map keypart_map)
{
  DBUG_ENTER("ha_mroonga::index_read_last_map");
  uint keynr = active_index;
  KEY key_info = table->key_info[keynr];
  KEY_PART_INFO key_part = key_info.key_part[0];

  int flags = GRN_CURSOR_DESCENDING;
  uint size_min = 0, size_max = 0;
  void *val_min = NULL, *val_max = NULL;
  Field *field = key_part.field;

  if (cur) {
    grn_table_cursor_close(ctx, cur);
    cur = NULL;
  }
  if (cur0) {
    grn_table_cursor_close(ctx, cur0);
    cur0 = NULL;
  }

  mrn_set_key_buf(ctx, field, key, key_min[keynr], &size_min);
  val_min = key_min[keynr];
  val_max = key_min[keynr];
  size_max = size_min;

  uint pkeynr = table->s->primary_key;

  if (keynr == pkeynr) { // primary index
    DBUG_PRINT("info",("mroonga use primary key"));
    cur =
      grn_table_cursor_open(ctx, tbl, val_min, size_min, val_max, size_max,
                            0, -1, flags);
  } else { // normal index
    DBUG_PRINT("info",("mroonga use key%u", keynr));
    cur0 =
      grn_table_cursor_open(ctx, idx_tbl[keynr], val_min, size_min,
                            val_max, size_max, 0, -1, flags);
    cur =
      grn_index_cursor_open(ctx, cur0, idx_col[keynr], 0, GRN_ID_MAX, 0);
  }
  if (ctx->rc) {
    my_message(ER_ERROR_ON_READ, ctx->errbuf, MYF(0));
    DBUG_RETURN(ER_ERROR_ON_READ);
  }
  row_id = grn_table_cursor_next(ctx, cur);
  if (ctx->rc) {
    my_message(ER_ERROR_ON_READ, ctx->errbuf, MYF(0));
    grn_table_cursor_close(ctx, cur);
    cur = NULL;
    DBUG_RETURN(ER_ERROR_ON_READ);
  }
  if (row_id == GRN_ID_NIL) {
    grn_table_cursor_close(ctx, cur);
    cur = NULL;
    table->status = STATUS_NOT_FOUND;
    DBUG_RETURN(HA_ERR_END_OF_FILE);
  }
  store_fields_from_primary_table(buf, row_id);
  table->status = 0;
  DBUG_RETURN(0);
}

int ha_mroonga::index_next(uchar *buf)
{
  DBUG_ENTER("ha_mroonga::index_next");
  row_id = grn_table_cursor_next(ctx, cur);
  if (ctx->rc) {
    my_message(ER_ERROR_ON_READ, ctx->errbuf, MYF(0));
    grn_table_cursor_close(ctx, cur);
    cur = NULL;
    DBUG_RETURN(ER_ERROR_ON_READ);
  }
  if (row_id == GRN_ID_NIL) {
    grn_table_cursor_close(ctx, cur);
    cur = NULL;
    table->status = STATUS_NOT_FOUND;
    DBUG_RETURN(HA_ERR_END_OF_FILE);
  }
  store_fields_from_primary_table(buf, row_id);
  table->status = 0;
  DBUG_RETURN(0);
}

int ha_mroonga::index_prev(uchar *buf)
{
  DBUG_ENTER("ha_mroonga::index_prev");
  row_id = grn_table_cursor_next(ctx, cur);
  if (ctx->rc) {
    my_message(ER_ERROR_ON_READ, ctx->errbuf, MYF(0));
    grn_table_cursor_close(ctx, cur);
    cur = NULL;
    DBUG_RETURN(ER_ERROR_ON_READ);
  }
  if (row_id == GRN_ID_NIL) {
    grn_table_cursor_close(ctx, cur);
    cur = NULL;
    table->status = STATUS_NOT_FOUND;
    DBUG_RETURN(HA_ERR_END_OF_FILE);
  }
  store_fields_from_primary_table(buf, row_id);
  table->status = 0;
  DBUG_RETURN(0);
}

int ha_mroonga::index_first(uchar *buf)
{
  DBUG_ENTER("ha_mroonga::index_first");
  if (cur) {
    grn_table_cursor_close(ctx, cur);
    cur = NULL;
  }
  if (cur0) {
    grn_table_cursor_close(ctx, cur0);
    cur0 = NULL;
  }
  uint pkeynr = table->s->primary_key;
  if (active_index == pkeynr) { // primary index
    DBUG_PRINT("info",("mroonga use primary key"));
    cur =
      grn_table_cursor_open(ctx, tbl, NULL, 0, NULL, 0,
                            0, -1, 0);
  } else { // normal index
    DBUG_PRINT("info",("mroonga use key%u", active_index));
    cur0 =
      grn_table_cursor_open(ctx, idx_tbl[active_index], NULL, 0,
                            NULL, 0, 0, -1, 0);
    cur =
      grn_index_cursor_open(ctx, cur0, idx_col[active_index], 0, GRN_ID_MAX, 0);
  }
  if (ctx->rc) {
    my_message(ER_ERROR_ON_READ, ctx->errbuf, MYF(0));
    DBUG_RETURN(ER_ERROR_ON_READ);
  }
  row_id = grn_table_cursor_next(ctx, cur);
  if (ctx->rc) {
    my_message(ER_ERROR_ON_READ, ctx->errbuf, MYF(0));
    grn_table_cursor_close(ctx, cur);
    cur = NULL;
    DBUG_RETURN(ER_ERROR_ON_READ);
  }
  if (row_id == GRN_ID_NIL) {
    grn_table_cursor_close(ctx, cur);
    cur = NULL;
    table->status = STATUS_NOT_FOUND;
    DBUG_RETURN(HA_ERR_END_OF_FILE);
  }
  store_fields_from_primary_table(buf, row_id);
  table->status = 0;
  DBUG_RETURN(0);
}

int ha_mroonga::index_last(uchar *buf)
{
  DBUG_ENTER("ha_mroonga::index_last");
  if (cur) {
    grn_table_cursor_close(ctx, cur);
    cur = NULL;
  }
  if (cur0) {
    grn_table_cursor_close(ctx, cur0);
    cur0 = NULL;
  }
  int flags = GRN_CURSOR_DESCENDING;
  uint pkeynr = table->s->primary_key;
  if (active_index == pkeynr) { // primary index
    DBUG_PRINT("info",("mroonga use primary key"));
    cur =
      grn_table_cursor_open(ctx, tbl, NULL, 0, NULL, 0,
                            0, -1, flags);
  } else { // normal index
    DBUG_PRINT("info",("mroonga use key%u", active_index));
    cur0 =
      grn_table_cursor_open(ctx, idx_tbl[active_index], NULL, 0,
                            NULL, 0, 0, -1, flags);
    cur =
      grn_index_cursor_open(ctx, cur0, idx_col[active_index], 0, GRN_ID_MAX, 0);
  }
  if (ctx->rc) {
    my_message(ER_ERROR_ON_READ, ctx->errbuf, MYF(0));
    DBUG_RETURN(ER_ERROR_ON_READ);
  }
  row_id = grn_table_cursor_next(ctx, cur);
  if (ctx->rc) {
    my_message(ER_ERROR_ON_READ, ctx->errbuf, MYF(0));
    grn_table_cursor_close(ctx, cur);
    cur = NULL;
    DBUG_RETURN(ER_ERROR_ON_READ);
  }
  if (row_id == GRN_ID_NIL) {
    grn_table_cursor_close(ctx, cur);
    cur = NULL;
    table->status = STATUS_NOT_FOUND;
    DBUG_RETURN(HA_ERR_END_OF_FILE);
  }
  store_fields_from_primary_table(buf, row_id);
  table->status = 0;
  DBUG_RETURN(0);
}

int ha_mroonga::index_next_same(uchar *buf, const uchar *key, uint keylen)
{
  DBUG_ENTER("ha_mroonga::index_next_same");
  if (cur == NULL) { // for _id
    table->status = STATUS_NOT_FOUND;
    DBUG_RETURN(HA_ERR_END_OF_FILE);
  }
  row_id = grn_table_cursor_next(ctx, cur);
  if (ctx->rc) {
    my_message(ER_ERROR_ON_READ, ctx->errbuf, MYF(0));
    grn_table_cursor_close(ctx, cur);
    cur = NULL;
    DBUG_RETURN(ER_ERROR_ON_READ);
  }
  if (row_id == GRN_ID_NIL) {
    grn_table_cursor_close(ctx, cur);
    cur = NULL;
    table->status = STATUS_NOT_FOUND;
    DBUG_RETURN(HA_ERR_END_OF_FILE);
  }
  if (!count_skip)
    store_fields_from_primary_table(buf, row_id);
  table->status = 0;
  DBUG_RETURN(0);
}

int ha_mroonga::read_range_first(const key_range *start_key,
                                 const key_range *end_key,
                                 bool eq_range, bool sorted)
{
  DBUG_ENTER("ha_mroonga::read_range_first");
  check_count_skip(start_key ? start_key->keypart_map : 0,
                   end_key ? end_key->keypart_map : 0, FALSE);
  int flags = 0;
  uint size_min = 0, size_max = 0;
  void *val_min = NULL, *val_max = NULL;
  KEY key_info = table->s->key_info[active_index];
  KEY_PART_INFO key_part = key_info.key_part[0];
  Field *field = key_part.field;
  const char *col_name = field->field_name;
  int col_name_size = strlen(col_name);

  if (cur) {
    grn_table_cursor_close(ctx, cur);
    cur = NULL;
  }
  if (cur0) {
    grn_table_cursor_close(ctx, cur0);
    cur0 = NULL;
  }

  if (start_key != NULL) {
    mrn_set_key_buf(ctx, field, start_key->key, key_min[active_index],
                    &size_min);
    val_min = key_min[active_index];
    if (start_key->flag == HA_READ_AFTER_KEY) {
      flags |= GRN_CURSOR_GT;
    } else if (start_key->flag == HA_READ_KEY_EXACT) {
      // for _id
      if (strncmp(MRN_ID_COL_NAME, col_name, col_name_size) == 0) {
        grn_id rid = *(grn_id*) key_min[active_index];
        if (grn_table_at(ctx, tbl, rid) != GRN_ID_NIL) { // found
          store_fields_from_primary_table(table->record[0], rid);
          table->status = 0;
          cur = NULL;
          row_id = rid;
          DBUG_RETURN(0);
        } else {
          table->status = STATUS_NOT_FOUND;
          cur = NULL;
          row_id = GRN_ID_NIL;
          DBUG_RETURN(HA_ERR_END_OF_FILE);
        }
      }

    }
  }
  if (end_key != NULL) {
    mrn_set_key_buf(ctx, field, end_key->key, key_max[active_index],
      &size_max);
    val_max = key_max[active_index];
    if (end_key->flag == HA_READ_BEFORE_KEY) {
      flags |= GRN_CURSOR_LT;
    }
  }
  uint pkeynr = table->s->primary_key;

  if (active_index == pkeynr) { // primary index
    DBUG_PRINT("info",("mroonga use primary key"));
    cur =
      grn_table_cursor_open(ctx, tbl, val_min, size_min, val_max, size_max,
                            0, -1, flags);
  } else { // normal index
    DBUG_PRINT("info",("mroonga use key%u", active_index));
    cur0 =
      grn_table_cursor_open(ctx, idx_tbl[active_index], val_min, size_min,
                            val_max, size_max, 0, -1, flags);
    cur =
      grn_index_cursor_open(ctx, cur0, idx_col[active_index], 0, GRN_ID_MAX, 0);
  }
  if (ctx->rc) {
    my_message(ER_ERROR_ON_READ, ctx->errbuf, MYF(0));
    DBUG_RETURN(ER_ERROR_ON_READ);
  }
  row_id = grn_table_cursor_next(ctx, cur);
  if (ctx->rc) {
    my_message(ER_ERROR_ON_READ, ctx->errbuf, MYF(0));
    grn_table_cursor_close(ctx, cur);
    cur = NULL;
    DBUG_RETURN(ER_ERROR_ON_READ);
  }
  if (row_id == GRN_ID_NIL) {
    grn_table_cursor_close(ctx, cur);
    cur = NULL;
    table->status = STATUS_NOT_FOUND;
    DBUG_RETURN(HA_ERR_END_OF_FILE);
  }
  store_fields_from_primary_table(table->record[0], row_id);
  table->status = 0;
  DBUG_RETURN(0);
}

int ha_mroonga::read_range_next() {
  DBUG_ENTER("ha_mroonga::read_range_next");

  if (cur == NULL) {
    DBUG_RETURN(HA_ERR_END_OF_FILE);
  }

  row_id = grn_table_cursor_next(ctx, cur);
  if (ctx->rc) {
    my_message(ER_ERROR_ON_READ, ctx->errbuf, MYF(0));
    grn_table_cursor_close(ctx, cur);
    cur = NULL;
    DBUG_RETURN(ER_ERROR_ON_READ);
  }

  if (row_id == GRN_ID_NIL) {
    grn_table_cursor_close(ctx, cur);
    cur = NULL;
    table->status = STATUS_NOT_FOUND;
    DBUG_RETURN(HA_ERR_END_OF_FILE);
  }

  uint pkeynr = table->s->primary_key;
  if (!count_skip)
    store_fields_from_primary_table(table->record[0], row_id);
  table->status = 0;
  DBUG_RETURN(0);
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
  int keyword_size = key->length();
  check_count_skip(0, 0, TRUE);
  if (sort_keys != NULL) {
    free(sort_keys);
    sort_keys = NULL;
  }
  check_fast_order_limit();
  if (res0 != NULL) {
    grn_obj_unlink(ctx, res0);
    res0 = NULL;
  }
  if (res != NULL) {
    grn_obj_unlink(ctx, res);
    _score = NULL;
    res = NULL;
  }

  row_id = GRN_ID_NIL;

  res = grn_table_create(ctx, NULL, 0, NULL,
                         GRN_TABLE_HASH_KEY | GRN_OBJ_WITH_SUBREC, tbl, 0);

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
  _score = grn_obj_column(ctx, res, MRN_SCORE_COL_NAME, strlen(MRN_SCORE_COL_NAME));
  int n_rec = grn_table_size(ctx, res);
  if (!fast_order_limit) {
    cur = grn_table_cursor_open(ctx, res, NULL, 0, NULL, 0, 0, -1, 0);
  } else {
    st_select_lex *select_lex = table->pos_in_table_list->select_lex;
    res0 = grn_table_create(ctx, NULL, 0, NULL,
                            GRN_OBJ_TABLE_NO_KEY, NULL, res);
    for (int i = 0; i < n_sort_keys; i++) {
      if (!sort_keys[i].key) {
        sort_keys[i].key = _score;
      }
    }
    grn_table_sort(ctx, res, 0, limit, res0, sort_keys, n_sort_keys);
    cur = grn_table_cursor_open(ctx, res0, NULL, 0, NULL, 0, 0, -1, 0);
  }

  { // for "not match"
    mrn_ft_info.please = &mrn_ft_vft;
    mrn_ft_info.ctx = ctx;
    mrn_ft_info.res = res;
    mrn_ft_info.rid = GRN_ID_NIL;
  }

  DBUG_RETURN((FT_INFO*) &mrn_ft_info);
}

int ha_mroonga::ft_read(uchar *buf)
{
  DBUG_ENTER("ha_mroonga::ft_read");
  grn_id rid;

  rid = grn_table_cursor_next(ctx, cur);
  if (ctx->rc) {
    my_message(ER_ERROR_ON_READ, ctx->errbuf, MYF(0));
    grn_table_cursor_close(ctx, cur);
    cur = NULL;
    DBUG_RETURN(ER_ERROR_ON_READ);
  }

  if (rid == GRN_ID_NIL) { // res will be closed by reset()
    grn_table_cursor_close(ctx, cur);
    cur = NULL;
    table->status = STATUS_NOT_FOUND;
    DBUG_RETURN(HA_ERR_END_OF_FILE);
  }
  table->status = 0;

  if (count_skip && row_id != GRN_ID_NIL) {
    DBUG_RETURN(0);
  }

  if (!fast_order_limit) {
    grn_table_get_key(ctx, res, rid, &row_id, sizeof(grn_id));
  } else if (fast_order_limit_with_index) {
    grn_table_get_key(ctx, res0, rid, &row_id, sizeof(grn_id));
  } else {
    grn_id rid2;
    grn_table_get_key(ctx, res0, rid, &rid2, sizeof(grn_id));
    grn_table_get_key(ctx, res, rid2, &row_id, sizeof(grn_id));
  }
  store_fields_from_primary_table(buf, row_id);
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

bool ha_mroonga::get_error_message(int error, String *buf)
{
  DBUG_ENTER("ha_mroonga::get_error_message");
  // latest error message
  buf->copy(ctx->errbuf, (uint) strlen(ctx->errbuf), system_charset_info);
  DBUG_RETURN(FALSE);
}

void ha_mroonga::check_count_skip(key_part_map start_key_part_map,
                                  key_part_map end_key_part_map, bool fulltext)
{
  DBUG_ENTER("ha_mroonga::check_count_skip");
  st_select_lex *select_lex = table->pos_in_table_list->select_lex;

  if (
    thd_sql_command(ha_thd()) == SQLCOM_SELECT &&
    !select_lex->non_agg_fields.elements &&
    !select_lex->group_list.elements &&
    !select_lex->having &&
    select_lex->table_list.elements == 1
  ) {
    Item *info = (Item *) select_lex->item_list.first_node()->info;
    if (
      info->type() != Item::SUM_FUNC_ITEM ||
      ((Item_sum *) info)->sum_func() != Item_sum::COUNT_FUNC ||
      ((Item_sum *) info)->nest_level ||
      ((Item_sum *) info)->aggr_level ||
      ((Item_sum *) info)->max_arg_level != -1 ||
      ((Item_sum *) info)->max_sum_func_level != -1
    ) {
      count_skip = FALSE;
      DBUG_VOID_RETURN;
    }

    int i = 0;
    Item *where;
    if (fulltext) {
      where = select_lex->where;
      if (!where ||
          where->type() != Item::FUNC_ITEM ||
          ((Item_func *)where)->functype() != Item_func::FT_FUNC) {
        count_skip = FALSE;
        DBUG_VOID_RETURN;
      }
      where = where->next;
      if (!where ||
          where->type() != Item::STRING_ITEM) {
        count_skip = FALSE;
        DBUG_VOID_RETURN;
      }
      for (where = where->next; where; where = where->next) {
        if (where->type() != Item::FIELD_ITEM)
          break;
      }
      if (where != info) {
        count_skip = FALSE;
        DBUG_VOID_RETURN;
      }
      count_skip = TRUE;
      mrn_count_skip++;
      DBUG_VOID_RETURN;
    } else {
      uint keynr = active_index;
      KEY key_info = table->key_info[keynr];
      KEY_PART_INFO *key_part = key_info.key_part;
      for (where = select_lex->where; where; where = where->next) {
        if (where->type() == Item::FIELD_ITEM)
        {
          Field *field = ((Item_field *)where)->field;
          if (field->table != table)
            break;
          int j;
          for (j = 0; j < key_info.key_parts; j++) {
            if (key_part[j].field == field)
            {
              if (!(start_key_part_map >> j) && !(end_key_part_map >> j))
                j = key_info.key_parts;
              else
                i++;
              break;
            }
          }
          if (j >= key_info.key_parts)
            break;
        }
        if (i >= select_lex->select_n_where_fields)
        {
          count_skip = TRUE;
          mrn_count_skip++;
          DBUG_VOID_RETURN;
        }
      }
    }
  }
  count_skip = FALSE;
  DBUG_VOID_RETURN;
}

void ha_mroonga::check_fast_order_limit()
{
  DBUG_ENTER("ha_mroonga::check_fast_order_limit");
  st_select_lex *select_lex = table->pos_in_table_list->select_lex;

  if (
    thd_sql_command(ha_thd()) == SQLCOM_SELECT &&
    !select_lex->n_sum_items &&
    !select_lex->group_list.elements &&
    !select_lex->having &&
    select_lex->table_list.elements == 1 &&
    select_lex->order_list.elements &&
    select_lex->explicit_limit &&
    select_lex->select_limit &&
    select_lex->select_limit->val_int() > 0
  ) {
    limit = (select_lex->offset_limit ?
            select_lex->offset_limit->val_int() : 0) +
            select_lex->select_limit->val_int();
    if (limit > (longlong) INT_MAX) {
      DBUG_PRINT("info",("mroonga fast_order_limit = FALSE"));
      fast_order_limit = FALSE;
      DBUG_VOID_RETURN;
    }
    Item *info = (Item *) select_lex->item_list.first_node()->info;
    Item *where;
    where = select_lex->where;
    if (!where ||
        where->type() != Item::FUNC_ITEM ||
        ((Item_func *)where)->functype() != Item_func::FT_FUNC) {
      DBUG_PRINT("info",("mroonga fast_order_limit = FALSE"));
      fast_order_limit = FALSE;
      DBUG_VOID_RETURN;
    }
    where = where->next;
    if (!where ||
        where->type() != Item::STRING_ITEM) {
      DBUG_PRINT("info",("mroonga fast_order_limit = FALSE"));
      fast_order_limit = FALSE;
      DBUG_VOID_RETURN;
    }
    for (where = where->next; where; where = where->next) {
      if (where->type() != Item::FIELD_ITEM || where == info)
        break;
    }
    if (where && where != info) {
      DBUG_PRINT("info",("mroonga fast_order_limit = FALSE"));
      fast_order_limit = FALSE;
      DBUG_VOID_RETURN;
    }
    n_sort_keys = select_lex->order_list.elements;
    sort_keys = (grn_table_sort_key *) malloc(sizeof(grn_table_sort_key) *
                                              n_sort_keys);
    ORDER *order;
    int i, col_field_index = -1;
    for (order = (ORDER *) select_lex->order_list.first, i = 0; order;
         order = order->next, i++) {
      if ((*order->item)->type() != Item::FIELD_ITEM)
      {
        DBUG_PRINT("info",("mroonga fast_order_limit = FALSE"));
        fast_order_limit = FALSE;
        DBUG_VOID_RETURN;
      }
      Field *field = ((Item_field *) (*order->item))->field;
      const char *col_name = field->field_name;
      int col_name_size = strlen(col_name);

      if (strncmp(MRN_ID_COL_NAME, col_name, col_name_size) == 0) {
        sort_keys[i].key = grn_obj_column(ctx, tbl, col_name, col_name_size);
      } else if (strncmp(MRN_SCORE_COL_NAME, col_name, col_name_size) == 0) {
        sort_keys[i].key = NULL;
      } else {
        sort_keys[i].key = col[field->field_index];
        col_field_index = field->field_index;
      }
      sort_keys[i].offset = 0;
      if (order->asc)
        sort_keys[i].flags = GRN_TABLE_SORT_ASC;
      else
        sort_keys[i].flags = GRN_TABLE_SORT_DESC;
    }
    grn_obj *index;
    if (i == 1 && col_field_index >= 0 &&
        grn_column_index(ctx, col[col_field_index], GRN_OP_LESS,
                         &index, 1, NULL)) {
      DBUG_PRINT("info",("mroonga fast_order_limit_with_index = TRUE"));
      fast_order_limit_with_index = TRUE;
    } else {
      DBUG_PRINT("info",("mroonga fast_order_limit_with_index = FALSE"));
      fast_order_limit_with_index = FALSE;
    }
    DBUG_PRINT("info",("mroonga fast_order_limit = TRUE"));
    fast_order_limit = TRUE;
    mrn_fast_order_limit++;
    DBUG_VOID_RETURN;
  }
  DBUG_PRINT("info",("mroonga fast_order_limit = FALSE"));
  fast_order_limit = FALSE;
  DBUG_VOID_RETURN;
}

void ha_mroonga::store_fields_from_primary_table(uchar *buf, grn_id rid)
{
  DBUG_ENTER("ha_mroonga::store_fields_from_primary_table");
  my_ptrdiff_t ptr_diff = PTR_BYTE_DIFF(buf, table->record[0]);
  int i;
  int n_columns = table->s->fields;
  for (i = 0; i < n_columns; i++) {
    Field *field = table->field[i];
    const char *col_name = field->field_name;
    int col_name_size = strlen(col_name);

    if (bitmap_is_set(table->read_set, field->field_index) ||
        bitmap_is_set(table->write_set, field->field_index)) {
#ifndef DBUG_OFF
      my_bitmap_map *tmp_map = dbug_tmp_use_all_columns(table,
        table->write_set);
#endif
      DBUG_PRINT("info",("mroonga store column %d(%d)",i,field->field_index));
      field->move_field_offset(ptr_diff);
      if (strncmp(MRN_ID_COL_NAME, col_name, col_name_size) == 0) {
        // for _id column
        field->set_notnull();
        field->store((int) rid);
      } else if (strncmp(MRN_SCORE_COL_NAME, col_name, col_name_size) == 0) {
        // for _score column
        if (res && res->header.flags & GRN_OBJ_WITH_SUBREC) {
          float score;
          grn_obj buf;
          GRN_INT32_INIT(&buf,0);
          grn_id res_id = grn_table_get(ctx, res, &rid, sizeof(rid));
          grn_obj_get_value(ctx, _score, res_id, &buf);
          score = GRN_INT32_VALUE(&buf);
          grn_obj_unlink(ctx, &buf);
          field->set_notnull();
          field->store((float) score);
        } else {
          field->set_notnull();
          field->store(0.0);
        }
      } else {
        // actual column
        mrn_store_field(ctx, field, col[i], rid);
      }
      field->move_field_offset(-ptr_diff);
#ifndef DBUG_OFF
      dbug_tmp_restore_column_map(table->write_set, tmp_map);
#endif
    }
  }
  // for "not match against"
  mrn_ft_info.rid = rid;

  DBUG_VOID_RETURN;
}

int ha_mroonga::reset()
{
  DBUG_ENTER("ha_mroonga::reset()");
  if (sort_keys != NULL) {
    free(sort_keys);
    sort_keys = NULL;
  }
  if (res0 != NULL) {
    grn_obj_unlink(ctx, res0);
    res0 = NULL;
  }
  if (res != NULL) {
    grn_obj_unlink(ctx, res);
    _score = NULL;
    res = NULL;
  }
  DBUG_RETURN(0);
}

#ifdef __cplusplus
}
#endif
