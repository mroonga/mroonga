/* -*- c-basic-offset: 2 -*- */
/*
  Copyright(C) 2010 Tetsuro IKEDA
  Copyright(C) 2010 Kentoku SHIBA
  Copyright(C) 2011 Kouhei Sutou <kou@clear-code.com>

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
#include "mysql_version.h"

#ifdef MYSQL51
#include <mysql_priv.h>
#include <mysql/plugin.h>
#else /* MYSQL51 */
#include <sql_priv.h>
#include <sql_class.h>
#include <probes_mysql.h>
#include <sql_plugin.h>
#include <sql_show.h>
#include "sql_partition.h"
#endif
#include <sql_select.h>
#include <ft_global.h>
#include <key.h>
#include <mysql.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "mrn_err.h"
#include "mrn_table.h"
#include "ha_mroonga.h"

#define MRN_DBUG_ENTER_FUNCTION() DBUG_ENTER(__FUNCTION__)
#if !defined(DBUG_OFF) && !defined(_lint)
#  define MRN_DBUG_ENTER_METHOD()                 \
    char method_name[512];                        \
    method_name[0] = '\0';                        \
    strcat(method_name, "ha_mroonga::");          \
    strcat(method_name, __FUNCTION__);            \
    DBUG_ENTER(method_name)
#else
#  define MRN_DBUG_ENTER_METHOD() MRN_DBUG_ENTER_FUNCTION()
#endif


#if MYSQL_VERSION_ID >= 50500
extern mysql_mutex_t LOCK_open;
#else
extern pthread_mutex_t LOCK_open;
#  define mysql_mutex_lock(mutex) pthread_mutex_lock(mutex)
#  define mysql_mutex_unlock(mutex) pthread_mutex_unlock(mutex)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* global variables */
grn_obj *mrn_db;
grn_hash *mrn_hash;
pthread_mutex_t mrn_db_mutex;
pthread_mutex_t mrn_log_mutex;
handlerton *mrn_hton_ptr;
HASH mrn_open_tables;
pthread_mutex_t mrn_open_tables_mutex;
static uchar *mrn_open_tables_get_key(MRN_SHARE *share,
                                      size_t *length,
                                      my_bool not_used __attribute__ ((unused)))
{
  MRN_DBUG_ENTER_FUNCTION();
  *length = share->table_name_length;
  DBUG_RETURN((uchar*) share->table_name);
}

/* status */
st_mrn_statuses mrn_status_vals;
long mrn_count_skip = 0;
long mrn_fast_order_limit = 0;

/* logging */
const char *mrn_logfile_name = MRN_LOG_FILE_NAME;
FILE *mrn_logfile = NULL;
int mrn_logfile_opened = 0;
grn_log_level mrn_log_level_default = GRN_LOG_DEFAULT_LEVEL;
ulong mrn_log_level = (ulong) mrn_log_level_default;

static void mrn_logger_func(int level, const char *time, const char *title,
                            const char *msg, const char *location,
                            void *func_arg)
{
  const char slev[] = " EACewnid-";
  if (mrn_logfile_opened) {
    pthread_mutex_lock(&mrn_log_mutex);
    fprintf(mrn_logfile, "%s|%c|%08x|%s\n", time,
            *(slev + level), (uint)(ulong)pthread_self(), msg);
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
static uchar *mrn_allocated_thds_get_key(THD *thd,
                                         size_t *length,
                                         my_bool not_used __attribute__ ((unused)))
{
  MRN_DBUG_ENTER_FUNCTION();
  *length = sizeof(THD *);
  DBUG_RETURN((uchar*) thd);
}

/* system functions */

static void mrn_create_status()
{
  MRN_DBUG_ENTER_FUNCTION();
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
  MRN_DBUG_ENTER_FUNCTION();
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
  MRN_DBUG_ENTER_FUNCTION();
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
  int last_insert_record_id = (int)slot_data->last_insert_record_id;
  return last_insert_record_id;
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
  MRN_DBUG_ENTER_FUNCTION();
  DBUG_RETURN(0);
}

static int i_s_groonga_stats_fill(
  THD* thd, TABLE_LIST* tables, COND* cond)
{
  TABLE* table = (TABLE *) tables->table;
  int status = 0;
  MRN_DBUG_ENTER_FUNCTION();
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
  MRN_DBUG_ENTER_FUNCTION();
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

static handler *mrn_handler_create(handlerton *hton, TABLE_SHARE *share, MEM_ROOT *root)
{
  return (new (root) ha_mroonga(hton, share));
}

static void mrn_drop_db(handlerton *hton, char *path)
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

static int mrn_close_connection(handlerton *hton, THD *thd)
{
  void *p = *thd_ha_data(thd, mrn_hton_ptr);
  if (p) {
    free(p);
    *thd_ha_data(thd, mrn_hton_ptr) = (void *) NULL;
    pthread_mutex_lock(&mrn_allocated_thds_mutex);
    my_hash_delete(&mrn_allocated_thds, (uchar*) thd);
    pthread_mutex_unlock(&mrn_allocated_thds_mutex);
  }
  return 0;
}

static bool mrn_flush_logs(handlerton *hton)
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

static grn_builtin_type mrn_get_type(grn_ctx *ctx, int mysql_field_type)
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

static int mrn_set_buf(grn_ctx *ctx, Field *field, grn_obj *buf, int *size)
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

static int mrn_set_key_buf(grn_ctx *ctx, Field *field,
                           const uchar *key, char *buf, uint *size)
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

static void mrn_store_field(grn_ctx *ctx, Field *field, grn_obj *col, grn_id id)
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

static int mrn_init(void *p)
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
  if ((pthread_mutex_init(&mrn_db_mutex, NULL) != 0)) {
    goto err_db_mutex_init;
  }
  if ((pthread_mutex_init(&mrn_allocated_thds_mutex, NULL) != 0)) {
    goto err_allocated_thds_mutex_init;
  }
  if (my_hash_init(&mrn_allocated_thds, system_charset_info, 32, 0, 0,
                   (my_hash_get_key) mrn_allocated_thds_get_key, 0, 0)) {
    goto error_allocated_thds_hash_init;
  }
  if ((pthread_mutex_init(&mrn_open_tables_mutex, NULL) != 0)) {
    goto err_allocated_open_tables_mutex_init;
  }
  if (my_hash_init(&mrn_open_tables, system_charset_info, 32, 0, 0,
                   (my_hash_get_key) mrn_open_tables_get_key, 0, 0)) {
    goto error_allocated_open_tables_hash_init;
  }

  grn_ctx_fin(ctx);
  return 0;

error_allocated_open_tables_hash_init:
  pthread_mutex_destroy(&mrn_open_tables_mutex);
err_allocated_open_tables_mutex_init:
  my_hash_free(&mrn_allocated_thds);
error_allocated_thds_hash_init:
  pthread_mutex_destroy(&mrn_allocated_thds_mutex);
err_allocated_thds_mutex_init:
  pthread_mutex_destroy(&mrn_db_mutex);
err_db_mutex_init:
err:
  pthread_mutex_destroy(&mrn_log_mutex);
err_log_mutex_init:
  grn_ctx_fin(ctx);
  grn_fin();
  return -1;
}

static int mrn_deinit(void *p)
{
  THD *thd = current_thd, *tmp_thd;
  grn_ctx *ctx;
  ctx = grn_ctx_open(0);

  GRN_LOG(ctx, GRN_LOG_NOTICE, "%s deinit", MRN_PACKAGE_STRING);

  if (thd && thd_sql_command(thd) == SQLCOM_UNINSTALL_PLUGIN) {
    pthread_mutex_lock(&mrn_allocated_thds_mutex);
    while ((tmp_thd = (THD *) my_hash_element(&mrn_allocated_thds, 0)))
    {
      void *slot_ptr = *thd_ha_data(tmp_thd, mrn_hton_ptr);
      if (slot_ptr) free(slot_ptr);
      *thd_ha_data(tmp_thd, mrn_hton_ptr) = (void *) NULL;
      my_hash_delete(&mrn_allocated_thds, (uchar *) tmp_thd);
    }
    pthread_mutex_unlock(&mrn_allocated_thds_mutex);
  }

  my_hash_free(&mrn_open_tables);
  pthread_mutex_destroy(&mrn_open_tables_mutex);
  my_hash_free(&mrn_allocated_thds);
  pthread_mutex_destroy(&mrn_allocated_thds_mutex);
  pthread_mutex_destroy(&mrn_log_mutex);
  pthread_mutex_destroy(&mrn_db_mutex);
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


static int mrn_wrapper_ft_read_next(FT_INFO *handler, char *record)
{
  MRN_DBUG_ENTER_FUNCTION();
  DBUG_RETURN(HA_ERR_END_OF_FILE);
}

static float mrn_wrapper_ft_find_relevance(FT_INFO *handler, uchar *record,
                                           uint length)
{
  MRN_DBUG_ENTER_FUNCTION();
  st_mrn_ft_info *info = (st_mrn_ft_info *)handler;

  grn_obj *score_column;
  score_column = grn_obj_column(info->ctx, info->result,
                                MRN_SCORE_COL_NAME, strlen(MRN_SCORE_COL_NAME));
  grn_obj score_value;
  GRN_INT32_INIT(&score_value, 0);
  grn_obj_get_value(info->ctx, score_column, info->record_id, &score_value);
  float score = (float)GRN_INT32_VALUE(&score_value);
  grn_obj_unlink(info->ctx, &score_value);
  grn_obj_unlink(info->ctx, score_column);

  DBUG_RETURN(score);
}

static void mrn_wrapper_ft_close_search(FT_INFO *handler)
{
  MRN_DBUG_ENTER_FUNCTION();
  st_mrn_ft_info *info = (st_mrn_ft_info *)handler;
  grn_obj_unlink(info->ctx, info->result);
  info->ctx = NULL;
  info->result = NULL;
  info->record_id = GRN_ID_NIL;
  delete info;
  DBUG_VOID_RETURN;
}

static float mrn_wrapper_ft_get_relevance(FT_INFO *handler)
{
  MRN_DBUG_ENTER_FUNCTION();
  DBUG_RETURN((float)-1.0);
}

static void mrn_wrapper_ft_reinit_search(FT_INFO *handler)
{
  MRN_DBUG_ENTER_FUNCTION();
  DBUG_VOID_RETURN;
}

static _ft_vft mrn_wrapper_ft_vft = {
  mrn_wrapper_ft_read_next,
  mrn_wrapper_ft_find_relevance,
  mrn_wrapper_ft_close_search,
  mrn_wrapper_ft_get_relevance,
  mrn_wrapper_ft_reinit_search
};


static float mrn_default_ft_find_relevance(FT_INFO *handler, uchar *record,
                                           uint length)
{
  st_mrn_ft_info *info = (st_mrn_ft_info*) handler;
  if (info->record_id != GRN_ID_NIL) {
    grn_ctx *ctx = info->ctx;
    grn_obj *result = info->result;
    grn_id record_id = info->record_id;

    if (result && result->header.flags & GRN_OBJ_WITH_SUBREC) {
      float score;
      grn_id result_id = grn_table_get(ctx, result, &record_id, sizeof(record_id));
      if (result_id != GRN_ID_NIL) {
        return (float) -1.0;
      } else {
        return (float) 0.0;
      }
    }
  }
  return (float) -1.0;
}

static float mrn_default_ft_get_relevance(FT_INFO *handler)
{
  return (float) -1.0;
}

static void mrn_default_ft_close_search(FT_INFO *handler)
{
  st_mrn_ft_info *info = (st_mrn_ft_info *)handler;
  info->ctx = NULL;
  info->result = NULL;
  info->record_id = GRN_ID_NIL;
}

static _ft_vft mrn_default_ft_vft = {
  NULL, // mrn_default_ft_read_next
  mrn_default_ft_find_relevance,
  mrn_default_ft_close_search,
  mrn_default_ft_get_relevance,
  NULL // mrn_default_ft_reinit_search
};


/* handler implementation */
ha_mroonga::ha_mroonga(handlerton *hton, TABLE_SHARE *share)
  :handler(hton, share),
   ignoring_duplicated_key(false)
{
  MRN_DBUG_ENTER_METHOD();
  ctx = grn_ctx_open(0);
  grn_ctx_use(ctx, mrn_db);
  cur = NULL;
  cur0 = NULL;
  result = NULL;
  result0 = NULL;
  sort_keys = NULL;
  share = NULL;
  is_clone = FALSE;
  wrap_handler = NULL;
  fulltext_searching = FALSE;
  DBUG_VOID_RETURN;
}

ha_mroonga::~ha_mroonga()
{
  MRN_DBUG_ENTER_METHOD();
  grn_ctx_fin(ctx);
  DBUG_VOID_RETURN;
}

const char *ha_mroonga::table_type() const
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN("groonga");
}

const char *ha_mroonga::index_type(uint key_nr)
{
  MRN_DBUG_ENTER_METHOD();
  KEY key_info = table->s->key_info[key_nr];
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
  MRN_DBUG_ENTER_METHOD();
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
    HA_CAN_INSERT_DELAYED |
    HA_BINLOG_FLAGS |
    HA_CAN_BIT_FIELD |
    HA_DUPLICATE_POS;
    //HA_HAS_RECORDS;

ulonglong ha_mroonga::wrapper_table_flags() const
{
  ulonglong table_flags;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  table_flags = wrap_handler->ha_table_flags() |
    HA_CAN_FULLTEXT | HA_PRIMARY_KEY_REQUIRED_FOR_DELETE;
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(table_flags);
}

ulonglong ha_mroonga::default_table_flags() const
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(ha_mroonga_table_flags);
}

ulonglong ha_mroonga::table_flags() const
{
  MRN_DBUG_ENTER_METHOD();
  if (wrap_handler && share && share->wrapper_mode)
    DBUG_RETURN(wrapper_table_flags());
  DBUG_RETURN(default_table_flags());
}

ulong ha_mroonga::wrapper_index_flags(uint idx, uint part, bool all_parts) const
{
  ulong index_flags;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  index_flags = wrap_handler->index_flags(idx, part, all_parts);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(index_flags);
}

ulong ha_mroonga::default_index_flags(uint idx, uint part, bool all_parts) const
{
  MRN_DBUG_ENTER_METHOD();
  KEY key = table_share->key_info[idx];
  if (key.algorithm == HA_KEY_ALG_BTREE || key.algorithm == HA_KEY_ALG_UNDEF) {
    DBUG_RETURN(HA_READ_NEXT | HA_READ_PREV | HA_READ_ORDER | HA_READ_RANGE);
  } else {
    DBUG_RETURN(HA_ONLY_WHOLE_INDEX | HA_KEY_SCAN_NOT_ROR);
  }
}

ulong ha_mroonga::index_flags(uint idx, uint part, bool all_parts) const
{
  MRN_DBUG_ENTER_METHOD();
  KEY key = table_share->key_info[idx];
  if (key.algorithm == HA_KEY_ALG_FULLTEXT) {
    DBUG_RETURN(HA_ONLY_WHOLE_INDEX | HA_KEY_SCAN_NOT_ROR);
  }
  if (wrap_handler && share && share->wrapper_mode)
    DBUG_RETURN(wrapper_index_flags(idx, part, all_parts));
  DBUG_RETURN(default_index_flags(idx, part, all_parts));
}

int ha_mroonga::wrapper_create(const char *name, TABLE *table,
                               HA_CREATE_INFO *info, MRN_SHARE *tmp_share)
{
  int error;
  handler *hnd;
  MRN_DBUG_ENTER_METHOD();

  if (table_share->primary_key == MAX_KEY)
  {
    my_message(ER_REQUIRES_PRIMARY_KEY, ER(ER_REQUIRES_PRIMARY_KEY), MYF(0));
    DBUG_RETURN(ER_REQUIRES_PRIMARY_KEY);
  }

  error = wrapper_create_index(name, table, info, tmp_share);
  if (error)
    DBUG_RETURN(error);

  wrap_key_info = mrn_create_key_info_for_table(tmp_share, table, &error);
  if (error)
    DBUG_RETURN(error);
  base_key_info = table->key_info;

  MRN_SET_WRAP_SHARE_KEY(tmp_share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  if (!(hnd =
      tmp_share->hton->create(tmp_share->hton, table->s,
        current_thd->mem_root)))
  {
    MRN_SET_BASE_SHARE_KEY(tmp_share, table->s);
    MRN_SET_BASE_TABLE_KEY(this, table);
    if (wrap_key_info)
    {
      my_free(wrap_key_info, MYF(0));
      wrap_key_info = NULL;
    }
    base_key_info = NULL;
    DBUG_RETURN(HA_ERR_OUT_OF_MEM);
  }
  error = hnd->ha_create(name, table, info);
  MRN_SET_BASE_SHARE_KEY(tmp_share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  delete hnd;

  if (wrap_key_info)
  {
    my_free(wrap_key_info, MYF(0));
    wrap_key_info = NULL;
  }
  base_key_info = NULL;
  DBUG_RETURN(error);
}

int ha_mroonga::wrapper_create_index(const char *name, TABLE *table,
                                     HA_CREATE_INFO *info, MRN_SHARE *tmp_share)
{
  MRN_DBUG_ENTER_METHOD();

  int error;
  error = ensure_database_create(name);
  if (error)
    DBUG_RETURN(error);

  grn_obj *grn_table;
  char grn_table_name[MRN_MAX_PATH_SIZE];
  mrn_table_name_gen(name, grn_table_name);
  char *grn_table_path = NULL;     // we don't specify path
  grn_obj *pkey_type = grn_ctx_at(ctx, GRN_DB_SHORT_TEXT);
  grn_obj *pkey_value_type = NULL; // we don't use this
  grn_obj_flags grn_table_flags = GRN_OBJ_PERSISTENT | GRN_OBJ_TABLE_HASH_KEY;

  grn_table = grn_table_create(ctx, grn_table_name, strlen(grn_table_name),
                               grn_table_path, grn_table_flags,
                               pkey_type, pkey_value_type);
  if (ctx->rc) {
    error = ER_CANT_CREATE_TABLE;
    my_message(ER_CANT_CREATE_TABLE, ctx->errbuf, MYF(0));
    DBUG_RETURN(error);
  }

  uint i;
  uint n_keys = table->s->keys;
  for (i = 0; i < n_keys; i++) {
    KEY key_info = table->s->key_info[i];

    if (key_info.algorithm != HA_KEY_ALG_FULLTEXT) {
      continue;
    }

    // must be single column key
    int key_parts = key_info.key_parts;
    if (key_parts != 1) {
      GRN_LOG(ctx, GRN_LOG_ERROR,
              "complex key is not supported yet: n-keys=<%d>", key_parts);
      error = ER_NOT_SUPPORTED_YET;
      my_message(error, "complex key is not supported yet.", MYF(0));
      DBUG_RETURN(error);
    }

    char index_name[MRN_MAX_PATH_SIZE];
    mrn_index_name_gen(grn_table_name, i, index_name);

    grn_obj_flags index_table_flags =
      GRN_OBJ_TABLE_PAT_KEY |
      GRN_OBJ_PERSISTENT |
      GRN_OBJ_KEY_NORMALIZE;
    grn_obj *index_table;

    Field *field = key_info.key_part[0].field;

    int mysql_field_type = field->type();
    grn_builtin_type gtype = mrn_get_type(ctx, mysql_field_type);
    grn_obj *column_type = grn_ctx_at(ctx, gtype);
    grn_obj_flags index_column_flags =
      GRN_OBJ_COLUMN_INDEX | GRN_OBJ_WITH_POSITION | GRN_OBJ_PERSISTENT;

    index_table = grn_table_create(ctx, index_name, strlen(index_name), NULL,
                                   index_table_flags, column_type, 0);
    if (ctx->rc) {
      grn_obj_remove(ctx, grn_table);
      error = ER_CANT_CREATE_TABLE;
      my_message(ER_CANT_CREATE_TABLE, ctx->errbuf, MYF(0));
      DBUG_RETURN(error);
    }

    grn_info_type info_type = GRN_INFO_DEFAULT_TOKENIZER;
    grn_obj *token_type = grn_ctx_at(ctx, GRN_DB_BIGRAM);
    grn_obj_set_info(ctx, index_table, info_type, token_type);
    grn_obj_unlink(ctx, token_type);

    const char *column_name = field->field_name;
    grn_obj *index_column = grn_column_create(ctx, index_table,
                                              column_name, strlen(column_name),
                                              NULL,
                                              index_column_flags,
                                              grn_table);
    if (ctx->rc) {
      grn_obj_remove(ctx, index_table);
      grn_obj_remove(ctx, grn_table);
      error = ER_CANT_CREATE_TABLE;
      my_message(error, ctx->errbuf, MYF(0));
      DBUG_RETURN(error);
    }
  }

  DBUG_RETURN(error);
}

int ha_mroonga::default_create(const char *name, TABLE *table,
                               HA_CREATE_INFO *info, MRN_SHARE *tmp_share)
{
  int error, i;
  MRN_DBUG_ENTER_METHOD();

  error = default_create_validate_pseudo_column(table);
  if (error)
    DBUG_RETURN(error);

  error = default_create_validate_index(table);
  if (error)
    DBUG_RETURN(error);

  error = ensure_database_create(name);
  if (error)
    DBUG_RETURN(error);

  grn_obj_flags tbl_flags = GRN_OBJ_PERSISTENT;

  /* primary key must be handled before creating table */
  grn_obj *pkey_type;
  uint pkey_nr = table->s->primary_key;
  if (pkey_nr != MAX_INDEXES) {
    KEY key_info = table->s->key_info[pkey_nr];

    // surpose simgle column key
    int key_parts = key_info.key_parts;
    if (key_parts != 1) {
      GRN_LOG(ctx, GRN_LOG_ERROR, "complex key is not supported yet");
      error = ER_NOT_SUPPORTED_YET;
      my_message(error, "complex key is not supported yet", MYF(0));
      DBUG_RETURN(error);
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
    error = ER_CANT_CREATE_TABLE;
    my_message(error, ctx->errbuf, MYF(0));
    DBUG_RETURN(error);
  }

  /* create columns */
  uint n_columns = table->s->fields;
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
      error = ER_CANT_CREATE_TABLE;
      my_message(error, ctx->errbuf, MYF(0));
      DBUG_RETURN(error);
    }
  }

  /* create indexes */
  char idx_name[MRN_MAX_PATH_SIZE];

  uint n_keys = table->s->keys;
  for (i = 0; i < n_keys; i++) {
    if (i == pkey_nr) {
      continue; // pkey is already handled
    }

    grn_obj *idx_tbl_obj, *idx_col_obj, *col_obj, *col_type, buf;
    KEY key_info = table->s->key_info[i];

    // must be single column key
    int key_parts = key_info.key_parts;
    if (key_parts != 1) {
      GRN_LOG(ctx, GRN_LOG_ERROR, "complex key is not supported yet");
      error = ER_NOT_SUPPORTED_YET;
      my_message(error, "complex key is not supported yet.", MYF(0));
      DBUG_RETURN(error);
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
      error = ER_CANT_CREATE_TABLE;
      my_message(ER_CANT_CREATE_TABLE, ctx->errbuf, MYF(0));
      DBUG_RETURN(error);
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
      error = ER_CANT_CREATE_TABLE;
      my_message(error, ctx->errbuf, MYF(0));
      DBUG_RETURN(error);
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

int ha_mroonga::default_create_validate_pseudo_column(TABLE *table)
{
  int error = 0;
  uint i, n_columns;

  MRN_DBUG_ENTER_METHOD();
  n_columns = table->s->fields;
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
        error = ER_CANT_CREATE_TABLE;
        my_message(error, "_id must be numeric data type", MYF(0));
        DBUG_RETURN(error);
      }
    } else if (strncmp(MRN_SCORE_COL_NAME, col_name, col_name_size) == 0) {
      switch (field->type()) {
      case (MYSQL_TYPE_FLOAT) :
      case (MYSQL_TYPE_DOUBLE) :
        break;
      default:
        GRN_LOG(ctx, GRN_LOG_ERROR, "_score must be float or double");
        error = ER_CANT_CREATE_TABLE;
        my_message(error, "_score must be float or double", MYF(0));
        DBUG_RETURN(error);
      }
    }
  }

  DBUG_RETURN(error);
}

int ha_mroonga::default_create_validate_index(TABLE *table)
{
  int error = 0;
  uint i;

  MRN_DBUG_ENTER_METHOD();
  /* checking if index is used for virtual columns  */
  uint n_keys = table->s->keys;
  for (i = 0; i < n_keys; i++) {
    KEY key_info = table->s->key_info[i];
    // must be single column key
    int key_parts = key_info.key_parts;
    if (key_parts != 1) {
      GRN_LOG(ctx, GRN_LOG_ERROR, "complex key is not supported yet");
      error = ER_NOT_SUPPORTED_YET;
      my_message(error, "complex key is not supported yet.", MYF(0));
      DBUG_RETURN(error);
    }
    Field *field = key_info.key_part[0].field;
    const char *col_name = field->field_name;
    int col_name_size = strlen(col_name);
    if (strncmp(MRN_ID_COL_NAME, col_name, col_name_size) == 0) {
      if (key_info.algorithm == HA_KEY_ALG_HASH) {
        continue; // hash index is ok
      }
      GRN_LOG(ctx, GRN_LOG_ERROR, "only hash index can be defined for _id");
      error = ER_CANT_CREATE_TABLE;
      my_message(error, "only hash index can be defined for _id", MYF(0));
      DBUG_RETURN(error);
    }
    if (strncmp(MRN_SCORE_COL_NAME, col_name, col_name_size) == 0) {
      GRN_LOG(ctx, GRN_LOG_ERROR, "_score cannot be used for index");
      error = ER_CANT_CREATE_TABLE;
      my_message(error, "_score cannot be used for index", MYF(0));
      DBUG_RETURN(error);
    }
  }

  DBUG_RETURN(error);
}

int ha_mroonga::ensure_database_create(const char *name)
{
  int error = 0;

  MRN_DBUG_ENTER_METHOD();
  /* before creating table, we must check if database is alreadly opened, created */
  grn_obj *db;
  char db_name[MRN_MAX_PATH_SIZE];
  char db_path[MRN_MAX_PATH_SIZE];
  struct stat db_stat;
  mrn_db_name_gen(name, db_name);
  mrn_db_path_gen(name, db_path);

  pthread_mutex_lock(&mrn_db_mutex);
  if (mrn_hash_get(ctx, mrn_hash, db_name, (void **) &(db)) != 0) {
    if (stat(db_path, &db_stat)) {
      // creating new database
      GRN_LOG(ctx, GRN_LOG_INFO, "database not found. creating...(%s)", db_path);
      db = grn_db_create(ctx, db_path, NULL);
      if (ctx->rc) {
        pthread_mutex_unlock(&mrn_db_mutex);
        error = ER_CANT_CREATE_TABLE;
        my_message(error, ctx->errbuf, MYF(0));
        DBUG_RETURN(error);
      }
    } else {
      // opening existing database
      db = grn_db_open(ctx, db_path);
      if (ctx->rc) {
        pthread_mutex_unlock(&mrn_db_mutex);
        error = ER_CANT_OPEN_FILE;
        my_message(error, ctx->errbuf, MYF(0));
        DBUG_RETURN(error);
      }
    }
    mrn_hash_put(ctx, mrn_hash, db_name, db);
  }
  pthread_mutex_unlock(&mrn_db_mutex);
  grn_ctx_use(ctx, db);

  DBUG_RETURN(error);
}

int ha_mroonga::ensure_database_open(const char *name)
{
  int error = 0;

  MRN_DBUG_ENTER_METHOD();
  grn_obj *db;
  char db_name[MRN_MAX_PATH_SIZE];
  char db_path[MRN_MAX_PATH_SIZE];
  mrn_db_name_gen(name, db_name);
  mrn_db_path_gen(name, db_path);

  pthread_mutex_lock(&mrn_db_mutex);
  if (mrn_hash_get(ctx, mrn_hash, db_name, (void **) &(db)) != 0) {
    db = grn_db_open(ctx, db_path);
    if (ctx->rc) {
      pthread_mutex_unlock(&mrn_db_mutex);
      error = ER_CANT_OPEN_FILE;
      my_message(error, ctx->errbuf, MYF(0));
      DBUG_RETURN(error);
    }
    mrn_hash_put(ctx, mrn_hash, db_name, db);
  }
  pthread_mutex_unlock(&mrn_db_mutex);
  grn_ctx_use(ctx, db);

  DBUG_RETURN(error);
}


int ha_mroonga::create(const char *name, TABLE *table, HA_CREATE_INFO *info)
{
  int i, error = 0;
  MRN_SHARE *tmp_share;
  MRN_DBUG_ENTER_METHOD();
  /* checking data type of virtual columns */

  if (!(tmp_share = mrn_get_share(name, table, &error)))
    DBUG_RETURN(error);

  if (tmp_share->wrapper_mode)
  {
    error = wrapper_create(name, table, info, tmp_share);
  } else {
    error = default_create(name, table, info, tmp_share);
  }

  mrn_free_share(tmp_share);
  DBUG_RETURN(error);
}

int ha_mroonga::wrapper_open(const char *name, int mode, uint test_if_locked)
{
  int error;
  MRN_DBUG_ENTER_METHOD();

  error = ensure_database_open(name);
  if (error)
    DBUG_RETURN(error);

  error = open_table(name);
  if (error)
    DBUG_RETURN(error);

  error = wrapper_open_indexes(name);
  if (error) {
    grn_obj_unlink(ctx, grn_table);
    grn_table = NULL;
    DBUG_RETURN(error);
  }

  init_alloc_root(&mem_root, 1024, 0);
  wrap_key_info = mrn_create_key_info_for_table(share, table, &error);
  if (error)
    DBUG_RETURN(error);
  base_key_info = table->key_info;

  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  if (!is_clone)
  {
    if (!(wrap_handler =
        share->hton->create(share->hton, table->s, &mem_root)))
    {
      MRN_SET_BASE_SHARE_KEY(share, table->s);
      MRN_SET_BASE_TABLE_KEY(this, table);
      if (wrap_key_info)
      {
        my_free(wrap_key_info, MYF(0));
        wrap_key_info = NULL;
      }
      base_key_info = NULL;
      DBUG_RETURN(HA_ERR_OUT_OF_MEM);
    }
    error = wrap_handler->ha_open(table, name, mode, test_if_locked);
  } else {
#if MYSQL_VERSION_ID >= 50513
    if (!(wrap_handler = parent_for_clone->wrap_handler->clone(name,
      mem_root_for_clone)))
#else
    if (!(wrap_handler = parent_for_clone->wrap_handler->clone(
      mem_root_for_clone)))
#endif
    {
      MRN_SET_BASE_SHARE_KEY(share, table->s);
      MRN_SET_BASE_TABLE_KEY(this, table);
      if (wrap_key_info)
      {
        my_free(wrap_key_info, MYF(0));
        wrap_key_info = NULL;
      }
      base_key_info = NULL;
      DBUG_RETURN(HA_ERR_OUT_OF_MEM);
    }
  }
  ref_length = wrap_handler->ref_length;
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  init();

  pk_keypart_map = make_prev_keypart_map(
    table->key_info[table_share->primary_key].key_parts);

  if (error)
  {
    grn_obj_unlink(ctx, grn_table);
    grn_table = NULL;
    // TODO: free indexes.

    delete wrap_handler;
    wrap_handler = NULL;
    if (wrap_key_info)
    {
      my_free(wrap_key_info, MYF(0));
      wrap_key_info = NULL;
    }
    base_key_info = NULL;
  }
  DBUG_RETURN(error);
}

int ha_mroonga::wrapper_open_indexes(const char *name)
{
  int error = 0;

  MRN_DBUG_ENTER_METHOD();

  char index_name[MRN_MAX_PATH_SIZE];
  uint n_keys = table->s->keys;
  uint n_primary_keys = table->s->primary_key;
  if (n_keys > 0) {
    // TODO: reduce allocate memories. We only need just
    // for HA_KEY_ALG_FULLTEXT keys.
    grn_index_tables = (grn_obj **)malloc(sizeof(grn_obj *) * n_keys);
    grn_index_columns = (grn_obj **)malloc(sizeof(grn_obj *) * n_keys);
    key_min = (char **)malloc(sizeof(char *) * n_keys);
    key_max = (char **)malloc(sizeof(char *) * n_keys);
  } else {
    grn_index_tables = grn_index_columns = NULL;
    key_min = key_max = NULL;
  }

  char table_name[MRN_MAX_PATH_SIZE];
  mrn_table_name_gen(name, table_name);
  int i = 0;
  for (i = 0; i < n_keys; i++) {
    KEY key_info = table->s->key_info[i];

    if (key_info.algorithm != HA_KEY_ALG_FULLTEXT) {
      continue;
    }

    char index_name[MRN_MAX_PATH_SIZE];
    mrn_index_name_gen(table_name, i, index_name);

    key_min[i] = (char *)malloc(MRN_MAX_KEY_SIZE);
    key_max[i] = (char *)malloc(MRN_MAX_KEY_SIZE);

    if (i == n_primary_keys) {
      grn_index_tables[i] = grn_index_columns[i] = NULL;
      continue;
    }

    mrn_index_name_gen(table_name, i, index_name);
    grn_index_tables[i] = grn_ctx_get(ctx, index_name, strlen(index_name));
    if (ctx->rc) {
      error = ER_CANT_OPEN_FILE;
      my_message(error, ctx->errbuf, MYF(0));
      goto error;
    }

    Field *field = key_info.key_part[0].field;
    const char *column_name = field->field_name;
    int column_name_size = strlen(column_name);
    grn_index_columns[i] = grn_obj_column(ctx, grn_index_tables[i],
                                          column_name, column_name_size);
    if (ctx->rc) {
      error = ER_CANT_OPEN_FILE;
      my_message(error, ctx->errbuf, MYF(0));
      goto error;
    }
  }

error:
  if (error) {
    for (; i >= 0; i--) {
      if (key_min[i]) {
        free(key_min[i]);
      }
      if (key_max[i]) {
        free(key_max[i]);
      }
      grn_obj *index_column = grn_index_columns[i];
      if (index_column) {
        grn_obj_unlink(ctx, index_column);
      }
      grn_obj *index_table = grn_index_tables[i];
      if (index_table) {
        grn_obj_unlink(ctx, index_table);
      }
    }
    free(key_min);
    free(key_max);
    free(grn_index_columns);
    free(grn_index_tables);
    key_min = NULL;
    key_max = NULL;
    grn_index_columns = NULL;
    grn_index_tables = NULL;
  }

  DBUG_RETURN(error);
}

int ha_mroonga::default_open(const char *name, int mode, uint test_if_locked)
{
  int error;
  MRN_DBUG_ENTER_METHOD();

  error = ensure_database_open(name);
  if (error)
    DBUG_RETURN(error);

  error = open_table(name);
  if (error)
    DBUG_RETURN(error);

  error = default_open_columns();
  if (error) {
    grn_obj_unlink(ctx, grn_table);
    grn_table = NULL;
    DBUG_RETURN(error);
  }

  error = default_open_indexes(name);
  if (error) {
    // TODO: free grn_columns and set NULL;
    grn_obj_unlink(ctx, grn_table);
    grn_table = NULL;
    DBUG_RETURN(error);
  }

  ref_length = sizeof(my_off_t);
  DBUG_RETURN(0);
}

int ha_mroonga::open_table(const char *name)
{
  MRN_DBUG_ENTER_METHOD();

  char table_name[MRN_MAX_PATH_SIZE];
  mrn_table_name_gen(name, table_name);
  grn_table = grn_ctx_get(ctx, table_name, strlen(table_name));
  if (ctx->rc) {
    int error = ER_CANT_OPEN_FILE;
    my_message(error, ctx->errbuf, MYF(0));
    DBUG_RETURN(error);
  }

  DBUG_RETURN(0);
}

int ha_mroonga::default_open_columns(void)
{
  MRN_DBUG_ENTER_METHOD();

  int n_columns = table->s->fields;
  grn_columns = (grn_obj **)malloc(sizeof(grn_obj *) * n_columns);

  int i;
  for (i = 0; i < n_columns; i++) {
    Field *field = table->field[i];
    const char *column_name = field->field_name;
    int column_name_size = strlen(column_name);

    if (strncmp(MRN_ID_COL_NAME, column_name, column_name_size) == 0) {
      grn_columns[i] = NULL;
      continue;
    }
    if (strncmp(MRN_SCORE_COL_NAME, column_name, column_name_size) == 0) {
      grn_columns[i] = NULL;
      continue;
    }

    grn_columns[i] = grn_obj_column(ctx, grn_table,
                                    column_name, column_name_size);
    if (ctx->rc) {
      // TODO: free grn_columns and set NULL;
      int error = ER_CANT_OPEN_FILE;
      my_message(error, ctx->errbuf, MYF(0));
      DBUG_RETURN(error);
    }
  }

  DBUG_RETURN(0);
}

int ha_mroonga::default_open_indexes(const char *name)
{
  int error = 0;

  MRN_DBUG_ENTER_METHOD();

  char index_name[MRN_MAX_PATH_SIZE];
  uint n_keys = table->s->keys;
  uint pkey_nr = table->s->primary_key;
  if (n_keys > 0) {
    grn_index_tables = (grn_obj **)malloc(sizeof(grn_obj *) * n_keys);
    grn_index_columns = (grn_obj **)malloc(sizeof(grn_obj *) * n_keys);
    key_min = (char **)malloc(sizeof(char *) * n_keys);
    key_max = (char **)malloc(sizeof(char *) * n_keys);
  } else {
    grn_index_tables = grn_index_columns = NULL;
    key_min = key_max = NULL;
  }

  char table_name[MRN_MAX_PATH_SIZE];
  mrn_table_name_gen(name, table_name);
  int i = 0;
  for (i = 0; i < n_keys; i++) {
    key_min[i] = (char *)malloc(MRN_MAX_KEY_SIZE);
    key_max[i] = (char *)malloc(MRN_MAX_KEY_SIZE);

    if (i == pkey_nr) {
      grn_index_tables[i] = grn_index_columns[i] = NULL;
      continue;
    }

    mrn_index_name_gen(table_name, i, index_name);
    grn_index_tables[i] = grn_ctx_get(ctx, index_name, strlen(index_name));
    if (ctx->rc) {
      error = ER_CANT_OPEN_FILE;
      my_message(error, ctx->errbuf, MYF(0));
      goto error;
    }

    KEY key_info = table->s->key_info[i];
    Field *field = key_info.key_part[0].field;
    const char *column_name = field->field_name;
    int column_name_size = strlen(column_name);
    grn_index_columns[i] = grn_obj_column(ctx, grn_index_tables[i],
                                          column_name, column_name_size);
    if (ctx->rc) {
      error = ER_CANT_OPEN_FILE;
      my_message(error, ctx->errbuf, MYF(0));
      goto error;
    }
  }

error:
  if (error) {
    for (; i >= 0; i--) {
      if (key_min[i]) {
        free(key_min[i]);
      }
      if (key_max[i]) {
        free(key_max[i]);
      }
      grn_obj *index_column = grn_index_columns[i];
      if (index_column) {
        grn_obj_unlink(ctx, index_column);
      }
      grn_obj *index_table = grn_index_tables[i];
      if (index_table) {
        grn_obj_unlink(ctx, index_table);
      }
    }
    free(key_min);
    free(key_max);
    free(grn_index_columns);
    free(grn_index_tables);
    key_min = NULL;
    key_max = NULL;
    grn_index_columns = NULL;
    grn_index_tables = NULL;
  }

  DBUG_RETURN(error);
}

int ha_mroonga::open(const char *name, int mode, uint test_if_locked)
{
  int error;
  MRN_DBUG_ENTER_METHOD();
  thr_lock_init(&thr_lock);
  thr_lock_data_init(&thr_lock, &thr_lock_data, NULL);

  if (!(share = mrn_get_share(name, table, &error)))
    DBUG_RETURN(error);

  if (share->wrapper_mode)
  {
    error = wrapper_open(name, mode, test_if_locked);
  } else {
    error = default_open(name, mode, test_if_locked);
  }

  if (error)
  {
    mrn_free_share(share);
    share = NULL;
  }
  DBUG_RETURN(error);
}

int ha_mroonga::wrapper_close()
{
  int error;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  error = wrap_handler->close();
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  delete wrap_handler;
  wrap_handler = NULL;
  if (wrap_key_info)
  {
    my_free(wrap_key_info, MYF(0));
    wrap_key_info = NULL;
  }
  base_key_info = NULL;
  free_root(&mem_root, MYF(0));
  DBUG_RETURN(error);
}

int ha_mroonga::default_close()
{
  MRN_DBUG_ENTER_METHOD();
  int i;
  uint n_keys = table->s->keys;
  uint pkey_nr = table->s->primary_key;
  for (i = 0; i < n_keys; i++) {
    free(key_min[i]);
    free(key_max[i]);
    if (i == pkey_nr) {
      continue;
    }
    grn_obj_unlink(ctx, grn_index_tables[i]);
  }
  grn_obj_unlink(ctx, grn_table);

  if (grn_index_tables != NULL) {
    free(grn_index_tables);
    free(grn_index_columns);
    free(key_min);
    free(key_max);
  }

  free(grn_columns);
  DBUG_RETURN(0);
}

int ha_mroonga::close()
{
  int error;
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
  {
    error = wrapper_close();
  } else {
    error = default_close();
  }

  mrn_free_share(share);
  share = NULL;
  is_clone = FALSE;
  thr_lock_delete(&thr_lock);
  DBUG_RETURN(0);
}

int ha_mroonga::wrapper_delete_table(const char *name, MRN_SHARE *tmp_share,
                                     const char *table_name)
{
  int error;
  handler *hnd;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(tmp_share, tmp_share->table_share);
  if (!(hnd =
      tmp_share->hton->create(tmp_share->hton, tmp_share->table_share,
      current_thd->mem_root)))
  {
    MRN_SET_BASE_SHARE_KEY(tmp_share, tmp_share->table_share);
    DBUG_RETURN(HA_ERR_OUT_OF_MEM);
  }
  MRN_SET_BASE_SHARE_KEY(tmp_share, tmp_share->table_share);

  if ((error = hnd->ha_delete_table(name)))
  {
    delete hnd;
    DBUG_RETURN(error);
  }

  error = wrapper_delete_index(name, tmp_share, table_name);

  delete hnd;
  DBUG_RETURN(error);
}

int ha_mroonga::wrapper_delete_index(const char *name, MRN_SHARE *tmp_share,
                                     const char *table_name)
{
  int error;
  MRN_DBUG_ENTER_METHOD();

  error = ensure_database_open(name);
  if (error)
    DBUG_RETURN(error);

  TABLE_SHARE *tmp_table_share = tmp_share->table_share;

  int i;
  for (i = 0; i < tmp_table_share->keys; i++) {
    char index_name[MRN_MAX_PATH_SIZE];
    mrn_index_name_gen(table_name, i, index_name);
    grn_obj *index_table = grn_ctx_get(ctx, index_name, strlen(index_name));
    if (index_table != NULL) {
      grn_obj_remove(ctx, index_table);
    }
  }

  grn_obj *table = grn_ctx_get(ctx, table_name, strlen(table_name));
  if (ctx->rc) {
    error = ER_CANT_OPEN_FILE;
    my_message(error, ctx->errbuf, MYF(0));
    DBUG_RETURN(error);
  }
  error = grn_obj_remove(ctx, table);
  DBUG_RETURN(error);
}

int ha_mroonga::default_delete_table(const char *name, MRN_SHARE *tmp_share,
                                     const char *tbl_name)
{
  int error;
  TABLE_SHARE *tmp_table_share = tmp_share->table_share;
  MRN_DBUG_ENTER_METHOD();
  char idx_name[MRN_MAX_PATH_SIZE];

  error = ensure_database_open(name);
  if (error)
    DBUG_RETURN(error);

  int i;
  for (i = 0; i < tmp_table_share->keys; i++) {
    mrn_index_name_gen(tbl_name, i, idx_name);
    grn_obj *idx_tbl_obj = grn_ctx_get(ctx, idx_name, strlen(idx_name));
    if (idx_tbl_obj != NULL) {
      grn_obj_remove(ctx, idx_tbl_obj);
    }
  }

  grn_obj *tbl_obj = grn_ctx_get(ctx, tbl_name, strlen(tbl_name));
  if (ctx->rc) {
    error = ER_CANT_OPEN_FILE;
    my_message(error, ctx->errbuf, MYF(0));
    DBUG_RETURN(error);
  }
  error = grn_obj_remove(ctx, tbl_obj);
  DBUG_RETURN(error);
}

int ha_mroonga::delete_table(const char *name)
{
  int error = 0;
  char db_name[MRN_MAX_PATH_SIZE];
  char tbl_name[MRN_MAX_PATH_SIZE];
  TABLE_LIST table_list;
  TABLE_SHARE *tmp_table_share;
  TABLE tmp_table;
  MRN_SHARE *tmp_share;
  MRN_DBUG_ENTER_METHOD();
  mrn_db_name_gen(name, db_name);
  mrn_table_name_gen(name, tbl_name);
#if MYSQL_VERSION_ID >= 50500
  table_list.init_one_table(db_name, strlen(db_name),
                            tbl_name, strlen(tbl_name), tbl_name, TL_WRITE);
#else
  table_list.init_one_table(db_name, tbl_name, TL_WRITE);
#endif
#if MYSQL_VERSION_ID >= 50500
  mysql_mutex_lock(&LOCK_open);
#endif
  if (!(tmp_table_share = mrn_get_table_share(&table_list, &error)))
  {
#if MYSQL_VERSION_ID >= 50500
    mysql_mutex_unlock(&LOCK_open);
#endif
    DBUG_RETURN(error);
  }
#if MYSQL_VERSION_ID >= 50500
  mysql_mutex_unlock(&LOCK_open);
#endif
  /* This is previous version */
  tmp_table_share->version--;
  tmp_table.s = tmp_table_share;
#if MYSQL_VERSION_ID >= 50500
  tmp_table.part_info = NULL;
#endif
  if (!(tmp_share = mrn_get_share(name, &tmp_table, &error)))
  {
    mrn_free_table_share(tmp_table_share);
    DBUG_RETURN(error);
  }

  if (tmp_share->wrapper_mode)
  {
    error = wrapper_delete_table(name, tmp_share, tbl_name);
  } else {
    error = default_delete_table(name, tmp_share, tbl_name);
  }

  mrn_free_share(tmp_share);
#if MYSQL_VERSION_ID >= 50500
  mysql_mutex_lock(&LOCK_open);
#endif
  mrn_free_table_share(tmp_table_share);
#if MYSQL_VERSION_ID >= 50500
  mysql_mutex_unlock(&LOCK_open);
#endif
  DBUG_RETURN(error);
}

int ha_mroonga::wrapper_info(uint flag)
{
  int error;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  error = wrap_handler->info(flag);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  if (flag & (HA_STATUS_ERRKEY | HA_STATUS_NO_LOCK)) {
    errkey = wrap_handler->errkey;
  }
  DBUG_RETURN(error);
}

int ha_mroonga::default_info(uint flag)
{
  MRN_DBUG_ENTER_METHOD();
  ha_rows rows = grn_table_size(ctx, grn_table);
  stats.records = rows;

  if (flag & (HA_STATUS_ERRKEY | HA_STATUS_NO_LOCK)) {
    errkey = dup_key;
  }

  DBUG_RETURN(0);
}

int ha_mroonga::info(uint flag)
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
    DBUG_RETURN(wrapper_info(flag));
  DBUG_RETURN(default_info(flag));
}

uint ha_mroonga::wrapper_lock_count() const
{
  uint lock_count;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  lock_count = wrap_handler->lock_count();
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(lock_count + 1);
}

uint ha_mroonga::default_lock_count() const
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(1);
}

uint ha_mroonga::lock_count() const
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
    DBUG_RETURN(wrapper_lock_count());
  DBUG_RETURN(default_lock_count());
}

THR_LOCK_DATA **ha_mroonga::wrapper_store_lock(THD *thd, THR_LOCK_DATA **to,
                                               enum thr_lock_type lock_type)
{
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  to = wrap_handler->store_lock(thd, to, lock_type);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(to);
}

THR_LOCK_DATA **ha_mroonga::default_store_lock(THD *thd, THR_LOCK_DATA **to,
                                               enum thr_lock_type lock_type)
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(to);
}

THR_LOCK_DATA **ha_mroonga::store_lock(THD *thd, THR_LOCK_DATA **to,
                                       enum thr_lock_type lock_type)
{
  MRN_DBUG_ENTER_METHOD();
  if (lock_type != TL_IGNORE && thr_lock_data.type == TL_UNLOCK) {
    thr_lock_data.type = lock_type;
  }
  *to++ = &thr_lock_data;
  if (share->wrapper_mode)
    to = wrapper_store_lock(thd, to, lock_type);
  else
    to = default_store_lock(thd, to, lock_type);
  DBUG_RETURN(to);
}

int ha_mroonga::wrapper_external_lock(THD *thd, int lock_type)
{
  int error;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  error = wrap_handler->ha_external_lock(thd, lock_type);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::default_external_lock(THD *thd, int lock_type)
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(0);
}

int ha_mroonga::external_lock(THD *thd, int lock_type)
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
    DBUG_RETURN(wrapper_external_lock(thd, lock_type));
  DBUG_RETURN(default_external_lock(thd, lock_type));
}

int ha_mroonga::wrapper_rnd_init(bool scan)
{
  int error;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  if (fulltext_searching) {
    st_mrn_ft_info *info = (st_mrn_ft_info *)wrapper_ft_info;
    info->cursor = grn_table_cursor_open(info->ctx, info->result,
                                         NULL, 0, NULL, 0,
                                         0, -1, 0);
    // TODO: error check.
    error = wrap_handler->ha_index_init(table_share->primary_key, FALSE);
  } else {
    error = wrap_handler->ha_rnd_init(scan);
  }
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::default_rnd_init(bool scan)
{
  MRN_DBUG_ENTER_METHOD();
  cur = grn_table_cursor_open(ctx, grn_table, NULL, 0, NULL, 0, 0, -1, 0);
  if (ctx->rc) {
    my_message(ER_ERROR_ON_READ, ctx->errbuf, MYF(0));
    DBUG_RETURN(ER_ERROR_ON_READ);
  }
  DBUG_RETURN(0);
}

int ha_mroonga::rnd_init(bool scan)
{
  MRN_DBUG_ENTER_METHOD();
  count_skip = FALSE;
  if (share->wrapper_mode)
    DBUG_RETURN(wrapper_rnd_init(scan));
  DBUG_RETURN(default_rnd_init(scan));
}

int ha_mroonga::wrapper_rnd_end()
{
  int error;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  if (fulltext_searching)
    error = wrap_handler->ha_index_end();
  else
    error = wrap_handler->ha_rnd_end();
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::default_rnd_end()
{
  MRN_DBUG_ENTER_METHOD();
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

int ha_mroonga::rnd_end()
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
    DBUG_RETURN(wrapper_rnd_end());
  DBUG_RETURN(default_rnd_end());
}

int ha_mroonga::wrapper_rnd_next(uchar *buf)
{
  int error;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  if (fulltext_searching) {
    st_mrn_ft_info *info = (st_mrn_ft_info *)wrapper_ft_info;
    info->record_id = grn_table_cursor_next(info->ctx, info->cursor);
    // TODO: error check.
    if (info->record_id == GRN_ID_NIL) {
      error = HA_ERR_END_OF_FILE;
    } else {
      grn_id relation_record_id;
      uchar *key;
      grn_table_get_key(ctx, info->result, info->record_id,
                        &relation_record_id, sizeof(grn_id));
      key = (uchar *)malloc(table->key_info->key_length);
      grn_table_get_key(ctx, grn_table, relation_record_id,
                        key, table->key_info->key_length);
      error = wrap_handler->index_read_map(buf,
                                           key,
                                           pk_keypart_map,
                                           HA_READ_KEY_EXACT);
      free(key);
    }
  } else {
    error = wrap_handler->rnd_next(buf);
  }
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::default_rnd_next(uchar *buf)
{
  MRN_DBUG_ENTER_METHOD();
  record_id = grn_table_cursor_next(ctx, cur);
  if (ctx->rc) {
    my_message(ER_ERROR_ON_READ, ctx->errbuf, MYF(0));
    grn_table_cursor_close(ctx, cur);
    cur = NULL;
    DBUG_RETURN(ER_ERROR_ON_READ);
  }
  if (record_id == GRN_ID_NIL) {
    grn_table_cursor_close(ctx, cur);
    cur = NULL;
    table->status = STATUS_NOT_FOUND;
    DBUG_RETURN(HA_ERR_END_OF_FILE);
  }
  store_fields_from_primary_table(buf, record_id);
  table->status = 0;

  DBUG_RETURN(0);
}

int ha_mroonga::rnd_next(uchar *buf)
{
  int error;
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
    DBUG_RETURN(wrapper_rnd_next(buf));
  DBUG_RETURN(default_rnd_next(buf));
}

int ha_mroonga::wrapper_rnd_pos(uchar *buf, uchar *pos)
{
  int error;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  error = wrap_handler->rnd_pos(buf, pos);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::default_rnd_pos(uchar *buf, uchar *pos)
{
  MRN_DBUG_ENTER_METHOD();
  record_id = *((grn_id*) pos);
  store_fields_from_primary_table(buf, record_id);
  DBUG_RETURN(0);
}

int ha_mroonga::rnd_pos(uchar *buf, uchar *pos)
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
    DBUG_RETURN(wrapper_rnd_pos(buf, pos));
  DBUG_RETURN(default_rnd_pos(buf, pos));
}

void ha_mroonga::wrapper_position(const uchar *record)
{
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  wrap_handler->ref = ref;
  wrap_handler->position(record);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_VOID_RETURN;
}

void ha_mroonga::default_position(const uchar *record)
{
  MRN_DBUG_ENTER_METHOD();
  memcpy(ref, &record_id, sizeof(grn_id));
  DBUG_VOID_RETURN;
}

void ha_mroonga::position(const uchar *record)
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
    wrapper_position(record);
  else
    default_position(record);
  DBUG_VOID_RETURN;
}

int ha_mroonga::mrn_extra(enum ha_extra_function operation)
{
  MRN_DBUG_ENTER_METHOD();
  switch (operation) {
  case HA_EXTRA_IGNORE_DUP_KEY:
    ignoring_duplicated_key = true;
    break;
  case HA_EXTRA_NO_IGNORE_DUP_KEY:
    ignoring_duplicated_key = false;
    break;
  default:
    break;
  }
  DBUG_RETURN(0);
}

int ha_mroonga::wrapper_extra(enum ha_extra_function operation)
{
  int error;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  error = wrap_handler->extra(operation);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::default_extra(enum ha_extra_function operation)
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(0);
}

int ha_mroonga::extra(enum ha_extra_function operation)
{
  int error;
  MRN_DBUG_ENTER_METHOD();
  DBUG_PRINT("info",("mroonga this=%p", this));
  if (share->wrapper_mode)
  {
    if ((error = wrapper_extra(operation)))
      DBUG_RETURN(error);
  } else {
    if ((error = default_extra(operation)))
      DBUG_RETURN(error);
  }
  DBUG_RETURN(mrn_extra(operation));
}

int ha_mroonga::wrapper_extra_opt(enum ha_extra_function operation,
                                  ulong cache_size)
{
  int error;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  error = wrap_handler->extra_opt(operation, cache_size);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::default_extra_opt(enum ha_extra_function operation,
                                  ulong cache_size)
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(0);
}

int ha_mroonga::extra_opt(enum ha_extra_function operation, ulong cache_size)
{
  int error;
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
  {
    if ((error = wrapper_extra_opt(operation, cache_size)))
      DBUG_RETURN(error);
  } else {
    if ((error = default_extra_opt(operation, cache_size)))
      DBUG_RETURN(error);
  }
  DBUG_RETURN(mrn_extra(operation));
}

int ha_mroonga::wrapper_write_row(uchar *buf)
{
  int error;
  THD *thd = ha_thd();
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  tmp_disable_binlog(thd);
  error = wrap_handler->ha_write_row(buf);
  reenable_binlog(thd);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);

  if (!error) {
    // TODO: extract as a method.
    grn_id record_id;

    grn_obj key;
    GRN_TEXT_INIT(&key, 0);

    grn_bulk_space(ctx, &key, table->key_info->key_length);
    key_copy((uchar *)(GRN_TEXT_VALUE(&key)),
             table->record[0],
             &(table->key_info[table_share->primary_key]),
             table->key_info[table_share->primary_key].key_length);

    int added;
    record_id = grn_table_add(ctx, grn_table,
                              GRN_TEXT_VALUE(&key), GRN_TEXT_LEN(&key),
                              &added);
    grn_obj_unlink(ctx, &key);

    grn_obj value;
    GRN_VOID_INIT(&value);

#ifndef DBUG_OFF
    my_bitmap_map *tmp_map = dbug_tmp_use_all_columns(table, table->read_set);
#endif
    uint i;
    uint n_keys = table->s->keys;
    for (i = 0; i < n_keys; i++) {
      grn_rc rc;
      KEY key_info = table->key_info[i];

      if (key_info.algorithm != HA_KEY_ALG_FULLTEXT) {
        continue;
      }

      grn_obj *index_column = grn_index_columns[i];

      uint j;
      for (j = 0; j < key_info.key_parts; j++) {
        Field *field = key_info.key_part[j].field;
        const char *column_name = field->field_name;

        if (field->is_null())
          continue;

        int column_size;
        mrn_set_buf(ctx, field, &value, &column_size);
        rc = grn_column_index_update(ctx, index_column, record_id, 1, NULL, &value);
        // TODO: check rc;
      }
    }
#ifndef DBUG_OFF
    dbug_tmp_restore_column_map(table->read_set, tmp_map);
#endif
    grn_obj_unlink(ctx, &value);
  }

  DBUG_RETURN(error);
}

int ha_mroonga::default_write_row(uchar *buf)
{
  MRN_DBUG_ENTER_METHOD();
  grn_obj wrapper;
  void *pkey = NULL;
  int pkey_size = 0;
  uint pkey_nr = table->s->primary_key;
  THD *thd = ha_thd();
  int i, col_size;
  int n_columns = table->s->fields;
  int error = 0;

  if (table->next_number_field && buf == table->record[0])
  {
    if ((error = update_auto_increment()))
      DBUG_RETURN(error);
  }

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
  if (pkey_nr != MAX_INDEXES) {
    KEY key_info = table->s->key_info[pkey_nr];
    // surpose simgle column key
    int field_no = key_info.key_part[0].field->field_index;
    Field *pkey_field = table->field[field_no];
    mrn_set_buf(ctx, pkey_field, &wrapper, &pkey_size);
    pkey = GRN_TEXT_VALUE(&wrapper);
  }

  int added;
  record_id = grn_table_add(ctx, grn_table, pkey, pkey_size, &added);
  if (ctx->rc) {
#ifndef DBUG_OFF
    dbug_tmp_restore_column_map(table->read_set, tmp_map);
#endif
    my_message(ER_ERROR_ON_WRITE, ctx->errbuf, MYF(0));
    DBUG_RETURN(ER_ERROR_ON_WRITE);
  }
  grn_obj_unlink(ctx, &wrapper);
  if (added == 0) {
    // duplicated error
#ifndef DBUG_OFF
    dbug_tmp_restore_column_map(table->read_set, tmp_map);
#endif
    error = HA_ERR_FOUND_DUPP_KEY;
    memcpy(dup_ref, &record_id, sizeof(grn_id));
    dup_key = pkey_nr;
    if (!ignoring_duplicated_key) {
      GRN_LOG(ctx, GRN_LOG_ERROR, "duplicated _id on insert");
    }
    DBUG_RETURN(error);
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
    grn_obj_set_value(ctx, grn_columns[i], record_id, &colbuf, GRN_OBJ_SET);
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
#ifndef DBUG_OFF
      dbug_tmp_restore_column_map(table->read_set, tmp_map);
#endif
      DBUG_RETURN(HA_ERR_OUT_OF_MEM);
    }
    pthread_mutex_unlock(&mrn_allocated_thds_mutex);
  }
  slot_data->last_insert_record_id = record_id;

  DBUG_RETURN(error);
}

int ha_mroonga::write_row(uchar *buf)
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
    DBUG_RETURN(wrapper_write_row(buf));
  DBUG_RETURN(default_write_row(buf));
}

int ha_mroonga::wrapper_update_row(const uchar *old_data, uchar *new_data)
{
  int error;
  THD *thd= ha_thd();
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  tmp_disable_binlog(thd);
  error = wrap_handler->ha_update_row(old_data, new_data);
  reenable_binlog(thd);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  if (!error)
  {
    my_ptrdiff_t ptr_diff = PTR_BYTE_DIFF(old_data, table->record[0]);

    grn_obj key;
    GRN_TEXT_INIT(&key, 0);

    grn_bulk_space(ctx, &key, table->key_info->key_length);
    key_copy((uchar *)(GRN_TEXT_VALUE(&key)),
             old_data,
             &(table->key_info[table_share->primary_key]),
             table->key_info[table_share->primary_key].key_length);

#ifndef DBUG_OFF
    my_bitmap_map *tmp_map = dbug_tmp_use_all_columns(table, table->read_set);
#endif
    uint i;
    uint n_keys = table->s->keys;
    for (i = 0; i < n_keys; i++) {
      grn_rc rc;
      KEY key_info = table->key_info[i];

      if (key_info.algorithm != HA_KEY_ALG_FULLTEXT) {
        continue;
      }

      grn_obj *index_column = grn_index_columns[i];

      uint j;
      for (j = 0; j < key_info.key_parts; j++) {
        Field *field = key_info.key_part[j].field;
        const char *column_name = field->field_name;

        field->move_field_offset(ptr_diff);
        if (field->is_null())
        {
          field->move_field_offset(-ptr_diff);
          continue;
        }

        int column_size;
        mrn_set_buf(ctx, field, &value, &column_size);
        rc = grn_column_index_update(ctx, index_column, record_id, 1, NULL, &value);
        // TODO: check rc;
        field->move_field_offset(-ptr_diff);
      }
    }
#ifndef DBUG_OFF
    dbug_tmp_restore_column_map(table->read_set, tmp_map);
#endif
  }
  DBUG_RETURN(error);
}

int ha_mroonga::default_update_row(const uchar *old_data, uchar *new_data)
{
  MRN_DBUG_ENTER_METHOD();
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
      grn_obj_set_value(ctx, grn_columns[i], record_id, &colbuf, GRN_OBJ_SET);
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

int ha_mroonga::update_row(const uchar *old_data, uchar *new_data)
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
    DBUG_RETURN(wrapper_update_row(old_data, new_data));
  DBUG_RETURN(default_update_row(old_data, new_data));
}

int ha_mroonga::wrapper_delete_row(const uchar *buf)
{
  int error;
  THD *thd= ha_thd();
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  tmp_disable_binlog(thd);
  error = wrap_handler->ha_delete_row(buf);
  reenable_binlog(thd);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::default_delete_row(const uchar *buf)
{
  MRN_DBUG_ENTER_METHOD();
  grn_table_delete_by_id(ctx, grn_table, record_id);
  if (ctx->rc) {
    my_message(ER_ERROR_ON_WRITE, ctx->errbuf, MYF(0));
    DBUG_RETURN(ER_ERROR_ON_WRITE);
  }
  DBUG_RETURN(0);
}

int ha_mroonga::delete_row(const uchar *buf)
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
    DBUG_RETURN(wrapper_delete_row(buf));
  DBUG_RETURN(default_delete_row(buf));
}

ha_rows ha_mroonga::wrapper_records_in_range(uint key_nr, key_range *range_min,
                                             key_range *range_max)
{
  ha_rows row_count;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  row_count = wrap_handler->records_in_range(key_nr, range_min, range_max);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(row_count);
}

ha_rows ha_mroonga::default_records_in_range(uint key_nr, key_range *range_min,
                                             key_range *range_max)
{
  MRN_DBUG_ENTER_METHOD();
  int flags = 0;
  uint size_min = 0, size_max = 0;
  ha_rows row_count = 0;
  void *val_min = NULL, *val_max = NULL;
  KEY key_info = table->s->key_info[key_nr];
  KEY_PART_INFO key_part = key_info.key_part[0];
  Field *field = key_part.field;
  const char *col_name = field->field_name;
  int col_name_size = strlen(col_name);

  if (strncmp(MRN_ID_COL_NAME, col_name, col_name_size) == 0) {
    DBUG_RETURN((ha_rows) 1) ;
  }

  if (range_min != NULL) {
    mrn_set_key_buf(ctx, field, range_min->key, key_min[key_nr], &size_min);
    val_min = key_min[key_nr];
    if (range_min->flag == HA_READ_AFTER_KEY) {
      flags |= GRN_CURSOR_GT;
    }
  }
  if (range_max != NULL) {
    mrn_set_key_buf(ctx, field, range_max->key, key_max[key_nr], &size_max);
    val_max = key_max[key_nr];
    if (range_max->flag == HA_READ_BEFORE_KEY) {
      flags |= GRN_CURSOR_LT;
    }
  }
  uint pkey_nr = table->s->primary_key;

  if (key_nr == pkey_nr) { // primary index
    grn_table_cursor *cur_t =
      grn_table_cursor_open(ctx, grn_table, val_min, size_min, val_max, size_max, 0, -1, flags);
    grn_id gid;
    while ((gid = grn_table_cursor_next(ctx, cur_t)) != GRN_ID_NIL) {
      row_count++;
    }
    grn_table_cursor_close(ctx, cur_t);
  } else { // normal index
    uint table_size = grn_table_size(ctx, grn_table);
    uint cardinality = grn_table_size(ctx, grn_index_tables[key_nr]);
    grn_table_cursor *cur_t0 =
      grn_table_cursor_open(ctx, grn_index_tables[key_nr], val_min, size_min, val_max, size_max, 0, -1, flags);
    grn_table_cursor *cur_t =
      grn_index_cursor_open(ctx, cur_t0, grn_index_columns[key_nr],
                            0, GRN_ID_MAX, 0);
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

ha_rows ha_mroonga::records_in_range(uint key_nr, key_range *range_min, key_range *range_max)
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
    DBUG_RETURN(wrapper_records_in_range(key_nr, range_min, range_max));
  DBUG_RETURN(default_records_in_range(key_nr, range_min, range_max));
}

int ha_mroonga::wrapper_index_init(uint idx, bool sorted)
{
  int error;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  if (fulltext_searching)
    error = wrap_handler->ha_index_init(table_share->primary_key, FALSE);
  else
    error = wrap_handler->ha_index_init(share->wrap_key_nr[idx], sorted);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::default_index_init(uint idx, bool sorted)
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(0);
}

int ha_mroonga::index_init(uint idx, bool sorted)
{
  MRN_DBUG_ENTER_METHOD();
  active_index = idx;
  count_skip = FALSE;
  if (share->wrapper_mode)
    DBUG_RETURN(wrapper_index_init(idx, sorted));
  DBUG_RETURN(default_index_init(idx, sorted));
}

int ha_mroonga::wrapper_index_end()
{
  int error;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  error = wrap_handler->ha_index_end();
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::default_index_end()
{
  MRN_DBUG_ENTER_METHOD();
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

int ha_mroonga::index_end()
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
    DBUG_RETURN(wrapper_index_end());
  DBUG_RETURN(default_index_end());
}

int ha_mroonga::wrapper_index_read_map(uchar * buf, const uchar * key,
                                       key_part_map keypart_map,
                                       enum ha_rkey_function find_flag)
{
  int error;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  error = wrap_handler->index_read_map(buf, key, keypart_map, find_flag);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::default_index_read_map(uchar * buf, const uchar * key,
                                       key_part_map keypart_map,
                                       enum ha_rkey_function find_flag)
{
  MRN_DBUG_ENTER_METHOD();
  uint key_nr = active_index;
  KEY key_info = table->key_info[key_nr];
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
    mrn_set_key_buf(ctx, field, key, key_min[key_nr], &size_min);
    val_min = key_min[key_nr];
    val_max = key_min[key_nr];
    size_max = size_min;

    // for _id
    if (strncmp(MRN_ID_COL_NAME, col_name, col_name_size) == 0) {
      grn_id found_record_id = *(grn_id *)key_min[key_nr];
      if (grn_table_at(ctx, grn_table, found_record_id) != GRN_ID_NIL) { // found
        store_fields_from_primary_table(buf, found_record_id);
        table->status = 0;
        cur = NULL;
        record_id = found_record_id;
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
    mrn_set_key_buf(ctx, field, key, key_max[key_nr], &size_max);
    val_max = key_max[key_nr];
    if (find_flag == HA_READ_BEFORE_KEY) {
      flags |= GRN_CURSOR_LT;
    }
  } else {
    mrn_set_key_buf(ctx, field, key, key_min[key_nr], &size_min);
    val_min = key_min[key_nr];
    if (find_flag == HA_READ_AFTER_KEY) {
      flags |= GRN_CURSOR_GT;
    }
  }

  uint pkey_nr = table->s->primary_key;

  if (key_nr == pkey_nr) { // primary index
    DBUG_PRINT("info",("mroonga use primary key"));
    cur =
      grn_table_cursor_open(ctx, grn_table, val_min, size_min, val_max, size_max,
                            0, -1, flags);
  } else { // normal index
    DBUG_PRINT("info",("mroonga use key%u", key_nr));
    cur0 = grn_table_cursor_open(ctx, grn_index_tables[key_nr],
                                 val_min, size_min,
                                 val_max, size_max,
                                 0, -1, flags);
    cur = grn_index_cursor_open(ctx, cur0, grn_index_columns[key_nr],
                                0, GRN_ID_MAX, 0);
  }
  if (ctx->rc) {
    my_message(ER_ERROR_ON_READ, ctx->errbuf, MYF(0));
    DBUG_RETURN(ER_ERROR_ON_READ);
  }
  record_id = grn_table_cursor_next(ctx, cur);
  if (ctx->rc) {
    my_message(ER_ERROR_ON_READ, ctx->errbuf, MYF(0));
    grn_table_cursor_close(ctx, cur);
    cur = NULL;
    DBUG_RETURN(ER_ERROR_ON_READ);
  }
  if (record_id == GRN_ID_NIL) {
    grn_table_cursor_close(ctx, cur);
    cur = NULL;
    table->status = STATUS_NOT_FOUND;
    DBUG_RETURN(HA_ERR_END_OF_FILE);
  }
  store_fields_from_primary_table(buf, record_id);
  table->status = 0;
  DBUG_RETURN(0);
}

int ha_mroonga::index_read_map(uchar * buf, const uchar * key,
                               key_part_map keypart_map,
                               enum ha_rkey_function find_flag)
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
    DBUG_RETURN(wrapper_index_read_map(buf, key, keypart_map, find_flag));
  DBUG_RETURN(default_index_read_map(buf, key, keypart_map, find_flag));
}

int ha_mroonga::wrapper_index_read_last_map(uchar *buf, const uchar *key,
                                            key_part_map keypart_map)
{
  int error;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  error = wrap_handler->index_read_last_map(buf, key, keypart_map);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::default_index_read_last_map(uchar *buf, const uchar *key,
                                            key_part_map keypart_map)
{
  MRN_DBUG_ENTER_METHOD();
  uint key_nr = active_index;
  KEY key_info = table->key_info[key_nr];
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

  mrn_set_key_buf(ctx, field, key, key_min[key_nr], &size_min);
  val_min = key_min[key_nr];
  val_max = key_min[key_nr];
  size_max = size_min;

  uint pkey_nr = table->s->primary_key;

  if (key_nr == pkey_nr) { // primary index
    DBUG_PRINT("info",("mroonga use primary key"));
    cur =
      grn_table_cursor_open(ctx, grn_table, val_min, size_min, val_max, size_max,
                            0, -1, flags);
  } else { // normal index
    DBUG_PRINT("info",("mroonga use key%u", key_nr));
    cur0 = grn_table_cursor_open(ctx, grn_index_tables[key_nr],
                                 val_min, size_min,
                                 val_max, size_max,
                                 0, -1, flags);
    cur = grn_index_cursor_open(ctx, cur0, grn_index_columns[key_nr],
                                0, GRN_ID_MAX, 0);
  }
  if (ctx->rc) {
    my_message(ER_ERROR_ON_READ, ctx->errbuf, MYF(0));
    DBUG_RETURN(ER_ERROR_ON_READ);
  }
  record_id = grn_table_cursor_next(ctx, cur);
  if (ctx->rc) {
    my_message(ER_ERROR_ON_READ, ctx->errbuf, MYF(0));
    grn_table_cursor_close(ctx, cur);
    cur = NULL;
    DBUG_RETURN(ER_ERROR_ON_READ);
  }
  if (record_id == GRN_ID_NIL) {
    grn_table_cursor_close(ctx, cur);
    cur = NULL;
    table->status = STATUS_NOT_FOUND;
    DBUG_RETURN(HA_ERR_END_OF_FILE);
  }
  store_fields_from_primary_table(buf, record_id);
  table->status = 0;
  DBUG_RETURN(0);
}

int ha_mroonga::index_read_last_map(uchar *buf, const uchar *key,
                                    key_part_map keypart_map)
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
    DBUG_RETURN(wrapper_index_read_last_map(buf, key, keypart_map));
  DBUG_RETURN(default_index_read_last_map(buf, key, keypart_map));
}

int ha_mroonga::wrapper_index_next(uchar *buf)
{
  int error;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  error = wrap_handler->index_next(buf);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::default_index_next(uchar *buf)
{
  MRN_DBUG_ENTER_METHOD();
  record_id = grn_table_cursor_next(ctx, cur);
  if (ctx->rc) {
    my_message(ER_ERROR_ON_READ, ctx->errbuf, MYF(0));
    grn_table_cursor_close(ctx, cur);
    cur = NULL;
    DBUG_RETURN(ER_ERROR_ON_READ);
  }
  if (record_id == GRN_ID_NIL) {
    grn_table_cursor_close(ctx, cur);
    cur = NULL;
    table->status = STATUS_NOT_FOUND;
    DBUG_RETURN(HA_ERR_END_OF_FILE);
  }
  store_fields_from_primary_table(buf, record_id);
  table->status = 0;
  DBUG_RETURN(0);
}

int ha_mroonga::index_next(uchar *buf)
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
    DBUG_RETURN(wrapper_index_next(buf));
  DBUG_RETURN(default_index_next(buf));
}

int ha_mroonga::wrapper_index_prev(uchar *buf)
{
  int error;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  error = wrap_handler->index_prev(buf);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::default_index_prev(uchar *buf)
{
  MRN_DBUG_ENTER_METHOD();
  record_id = grn_table_cursor_next(ctx, cur);
  if (ctx->rc) {
    my_message(ER_ERROR_ON_READ, ctx->errbuf, MYF(0));
    grn_table_cursor_close(ctx, cur);
    cur = NULL;
    DBUG_RETURN(ER_ERROR_ON_READ);
  }
  if (record_id == GRN_ID_NIL) {
    grn_table_cursor_close(ctx, cur);
    cur = NULL;
    table->status = STATUS_NOT_FOUND;
    DBUG_RETURN(HA_ERR_END_OF_FILE);
  }
  store_fields_from_primary_table(buf, record_id);
  table->status = 0;
  DBUG_RETURN(0);
}

int ha_mroonga::index_prev(uchar *buf)
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
    DBUG_RETURN(wrapper_index_prev(buf));
  DBUG_RETURN(default_index_prev(buf));
}

int ha_mroonga::wrapper_index_first(uchar *buf)
{
  int error;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  error = wrap_handler->index_first(buf);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::default_index_first(uchar *buf)
{
  MRN_DBUG_ENTER_METHOD();
  if (cur) {
    grn_table_cursor_close(ctx, cur);
    cur = NULL;
  }
  if (cur0) {
    grn_table_cursor_close(ctx, cur0);
    cur0 = NULL;
  }
  uint pkey_nr = table->s->primary_key;
  if (active_index == pkey_nr) { // primary index
    DBUG_PRINT("info",("mroonga use primary key"));
    cur =
      grn_table_cursor_open(ctx, grn_table, NULL, 0, NULL, 0,
                            0, -1, 0);
  } else { // normal index
    DBUG_PRINT("info",("mroonga use key%u", active_index));
    cur0 = grn_table_cursor_open(ctx, grn_index_tables[active_index],
                                 NULL, 0,
                                 NULL, 0,
                                 0, -1, 0);
    cur = grn_index_cursor_open(ctx, cur0,
                                grn_index_columns[active_index],
                                0, GRN_ID_MAX, 0);
  }
  if (ctx->rc) {
    my_message(ER_ERROR_ON_READ, ctx->errbuf, MYF(0));
    DBUG_RETURN(ER_ERROR_ON_READ);
  }
  record_id = grn_table_cursor_next(ctx, cur);
  if (ctx->rc) {
    my_message(ER_ERROR_ON_READ, ctx->errbuf, MYF(0));
    grn_table_cursor_close(ctx, cur);
    cur = NULL;
    DBUG_RETURN(ER_ERROR_ON_READ);
  }
  if (record_id == GRN_ID_NIL) {
    grn_table_cursor_close(ctx, cur);
    cur = NULL;
    table->status = STATUS_NOT_FOUND;
    DBUG_RETURN(HA_ERR_END_OF_FILE);
  }
  store_fields_from_primary_table(buf, record_id);
  table->status = 0;
  DBUG_RETURN(0);
}

int ha_mroonga::index_first(uchar *buf)
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
    DBUG_RETURN(wrapper_index_first(buf));
  DBUG_RETURN(default_index_first(buf));
}

int ha_mroonga::wrapper_index_last(uchar *buf)
{
  int error;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  error = wrap_handler->index_last(buf);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::default_index_last(uchar *buf)
{
  MRN_DBUG_ENTER_METHOD();
  if (cur) {
    grn_table_cursor_close(ctx, cur);
    cur = NULL;
  }
  if (cur0) {
    grn_table_cursor_close(ctx, cur0);
    cur0 = NULL;
  }
  int flags = GRN_CURSOR_DESCENDING;
  uint pkey_nr = table->s->primary_key;
  if (active_index == pkey_nr) { // primary index
    DBUG_PRINT("info",("mroonga use primary key"));
    cur =
      grn_table_cursor_open(ctx, grn_table, NULL, 0, NULL, 0,
                            0, -1, flags);
  } else { // normal index
    DBUG_PRINT("info",("mroonga use key%u", active_index));
    cur0 =
      grn_table_cursor_open(ctx, grn_index_tables[active_index],
                            NULL, 0,
                            NULL, 0,
                            0, -1, flags);
    cur =
      grn_index_cursor_open(ctx, cur0, grn_index_columns[active_index],
                            0, GRN_ID_MAX, 0);
  }
  if (ctx->rc) {
    my_message(ER_ERROR_ON_READ, ctx->errbuf, MYF(0));
    DBUG_RETURN(ER_ERROR_ON_READ);
  }
  record_id = grn_table_cursor_next(ctx, cur);
  if (ctx->rc) {
    my_message(ER_ERROR_ON_READ, ctx->errbuf, MYF(0));
    grn_table_cursor_close(ctx, cur);
    cur = NULL;
    DBUG_RETURN(ER_ERROR_ON_READ);
  }
  if (record_id == GRN_ID_NIL) {
    grn_table_cursor_close(ctx, cur);
    cur = NULL;
    table->status = STATUS_NOT_FOUND;
    DBUG_RETURN(HA_ERR_END_OF_FILE);
  }
  store_fields_from_primary_table(buf, record_id);
  table->status = 0;
  DBUG_RETURN(0);
}

int ha_mroonga::index_last(uchar *buf)
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
    DBUG_RETURN(wrapper_index_last(buf));
  DBUG_RETURN(default_index_last(buf));
}

int ha_mroonga::wrapper_index_next_same(uchar *buf, const uchar *key,
                                        uint keylen)
{
  int error;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  error = wrap_handler->index_next_same(buf, key, keylen);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::default_index_next_same(uchar *buf, const uchar *key,
                                        uint keylen)
{
  MRN_DBUG_ENTER_METHOD();
  if (cur == NULL) { // for _id
    table->status = STATUS_NOT_FOUND;
    DBUG_RETURN(HA_ERR_END_OF_FILE);
  }
  record_id = grn_table_cursor_next(ctx, cur);
  if (ctx->rc) {
    my_message(ER_ERROR_ON_READ, ctx->errbuf, MYF(0));
    grn_table_cursor_close(ctx, cur);
    cur = NULL;
    DBUG_RETURN(ER_ERROR_ON_READ);
  }
  if (record_id == GRN_ID_NIL) {
    grn_table_cursor_close(ctx, cur);
    cur = NULL;
    table->status = STATUS_NOT_FOUND;
    DBUG_RETURN(HA_ERR_END_OF_FILE);
  }
  if (!count_skip)
    store_fields_from_primary_table(buf, record_id);
  table->status = 0;
  DBUG_RETURN(0);
}

int ha_mroonga::index_next_same(uchar *buf, const uchar *key, uint keylen)
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
    DBUG_RETURN(wrapper_index_next_same(buf, key, keylen));
  DBUG_RETURN(default_index_next_same(buf, key, keylen));
}

int ha_mroonga::wrapper_read_range_first(const key_range *start_key,
                                         const key_range *end_key,
                                         bool eq_range, bool sorted)
{
  int error;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  error = wrap_handler->read_range_first(start_key, end_key, eq_range,
                                            sorted);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::default_read_range_first(const key_range *start_key,
                                         const key_range *end_key,
                                         bool eq_range, bool sorted)
{
  MRN_DBUG_ENTER_METHOD();
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
        grn_id found_record_id = *(grn_id *)key_min[active_index];
        if (grn_table_at(ctx, grn_table, found_record_id) != GRN_ID_NIL) { // found
          store_fields_from_primary_table(table->record[0], found_record_id);
          table->status = 0;
          cur = NULL;
          record_id = found_record_id;
          DBUG_RETURN(0);
        } else {
          table->status = STATUS_NOT_FOUND;
          cur = NULL;
          record_id = GRN_ID_NIL;
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
  uint pkey_nr = table->s->primary_key;

  if (active_index == pkey_nr) { // primary index
    DBUG_PRINT("info",("mroonga use primary key"));
    cur =
      grn_table_cursor_open(ctx, grn_table, val_min, size_min, val_max, size_max,
                            0, -1, flags);
  } else { // normal index
    DBUG_PRINT("info",("mroonga use key%u", active_index));
    cur0 = grn_table_cursor_open(ctx, grn_index_tables[active_index],
                                 val_min, size_min,
                                 val_max, size_max,
                                 0, -1, flags);
    cur = grn_index_cursor_open(ctx, cur0, grn_index_columns[active_index],
                                0, GRN_ID_MAX, 0);
  }
  if (ctx->rc) {
    my_message(ER_ERROR_ON_READ, ctx->errbuf, MYF(0));
    DBUG_RETURN(ER_ERROR_ON_READ);
  }
  record_id = grn_table_cursor_next(ctx, cur);
  if (ctx->rc) {
    my_message(ER_ERROR_ON_READ, ctx->errbuf, MYF(0));
    grn_table_cursor_close(ctx, cur);
    cur = NULL;
    DBUG_RETURN(ER_ERROR_ON_READ);
  }
  if (record_id == GRN_ID_NIL) {
    grn_table_cursor_close(ctx, cur);
    cur = NULL;
    table->status = STATUS_NOT_FOUND;
    DBUG_RETURN(HA_ERR_END_OF_FILE);
  }
  store_fields_from_primary_table(table->record[0], record_id);
  table->status = 0;
  DBUG_RETURN(0);
}

int ha_mroonga::read_range_first(const key_range *start_key,
                                 const key_range *end_key,
                                 bool eq_range, bool sorted)
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
    DBUG_RETURN(wrapper_read_range_first(start_key, end_key, eq_range,
                                         sorted));
  DBUG_RETURN(default_read_range_first(start_key, end_key, eq_range, sorted));
}

int ha_mroonga::wrapper_read_range_next()
{
  int error;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  error = wrap_handler->read_range_next();
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::default_read_range_next()
{
  MRN_DBUG_ENTER_METHOD();

  if (cur == NULL) {
    DBUG_RETURN(HA_ERR_END_OF_FILE);
  }

  record_id = grn_table_cursor_next(ctx, cur);
  if (ctx->rc) {
    my_message(ER_ERROR_ON_READ, ctx->errbuf, MYF(0));
    grn_table_cursor_close(ctx, cur);
    cur = NULL;
    DBUG_RETURN(ER_ERROR_ON_READ);
  }

  if (record_id == GRN_ID_NIL) {
    grn_table_cursor_close(ctx, cur);
    cur = NULL;
    table->status = STATUS_NOT_FOUND;
    DBUG_RETURN(HA_ERR_END_OF_FILE);
  }

  uint pkey_nr = table->s->primary_key;
  if (!count_skip)
    store_fields_from_primary_table(table->record[0], record_id);
  table->status = 0;
  DBUG_RETURN(0);
}

int ha_mroonga::read_range_next()
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
    DBUG_RETURN(wrapper_read_range_next());
  DBUG_RETURN(default_read_range_next());
}

int ha_mroonga::wrapper_ft_init()
{
  int error;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  error = wrap_handler->ha_index_init(table_share->primary_key, FALSE);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::default_ft_init()
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(0);
}

int ha_mroonga::ft_init()
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
    DBUG_RETURN(wrapper_ft_init());
  DBUG_RETURN(default_ft_init());
}

void ha_mroonga::wrapper_ft_end()
{
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  wrap_handler->ha_index_end();
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_VOID_RETURN;
}

void ha_mroonga::default_ft_end()
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_VOID_RETURN;
}

void ha_mroonga::ft_end()
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
    wrapper_ft_end();
  else
    default_ft_end();
  DBUG_VOID_RETURN;
}

FT_INFO *ha_mroonga::wrapper_ft_init_ext(uint flags, uint key_nr, String *key)
{
  MRN_DBUG_ENTER_METHOD();
  struct st_mrn_ft_info *info = new st_mrn_ft_info();
  info->please = &mrn_wrapper_ft_vft;
  info->ctx = ctx;
  info->result = grn_table_create(ctx, NULL, 0, NULL,
                                  GRN_TABLE_HASH_KEY | GRN_OBJ_WITH_SUBREC,
                                  grn_table, 0);
  info->record_id = GRN_ID_NIL;

  grn_obj *index_column = grn_index_columns[key_nr];
  grn_obj query;
  GRN_TEXT_INIT(&query, GRN_OBJ_DO_SHALLOW_COPY);
  GRN_TEXT_SET_REF(&query, key->ptr(), key->length());
  grn_obj_search(info->ctx, index_column, &query, info->result, GRN_OP_OR, NULL);

  wrapper_ft_info = info; // FIXME: support multi init_ext call.

  DBUG_RETURN((FT_INFO *)info);
}

FT_INFO *ha_mroonga::default_ft_init_ext(uint flags, uint key_nr, String *key)
{
  MRN_DBUG_ENTER_METHOD();
  grn_obj *ft = grn_index_columns[key_nr];
  const char *keyword = key->ptr();
  int keyword_size = key->length();
  check_count_skip(0, 0, TRUE);
  if (sort_keys != NULL) {
    free(sort_keys);
    sort_keys = NULL;
  }
  check_fast_order_limit();
  if (result0 != NULL) {
    grn_obj_unlink(ctx, result0);
    result0 = NULL;
  }
  if (result != NULL) {
    grn_obj_unlink(ctx, result);
    _score = NULL;
    result = NULL;
  }

  record_id = GRN_ID_NIL;

  result = grn_table_create(ctx, NULL, 0, NULL,
                            GRN_TABLE_HASH_KEY | GRN_OBJ_WITH_SUBREC,
                            grn_table, 0);

  if (flags & FT_BOOL) {
    // boolean search
    grn_query *query = grn_query_open(ctx, keyword, keyword_size,
                                      GRN_OP_OR, MRN_MAX_EXPRS);
    grn_obj_search(ctx, ft, (grn_obj*) query, result, GRN_OP_OR, NULL);
  } else {
    // nlq search
    grn_obj buf;
    GRN_TEXT_INIT(&buf, 0);
    GRN_TEXT_SET(ctx, &buf, keyword, keyword_size);
    grn_obj_search(ctx, ft, &buf, result, GRN_OP_OR, NULL);
  }
  _score = grn_obj_column(ctx, result,
                          MRN_SCORE_COL_NAME, strlen(MRN_SCORE_COL_NAME));
  int n_rec = grn_table_size(ctx, result);
  if (!fast_order_limit) {
    cur = grn_table_cursor_open(ctx, result, NULL, 0, NULL, 0, 0, -1, 0);
  } else {
    st_select_lex *select_lex = table->pos_in_table_list->select_lex;
    result0 = grn_table_create(ctx, NULL, 0, NULL,
                               GRN_OBJ_TABLE_NO_KEY, NULL, result);
    for (int i = 0; i < n_sort_keys; i++) {
      if (!sort_keys[i].key) {
        sort_keys[i].key = _score;
      }
    }
    grn_table_sort(ctx, result, 0, limit, result0, sort_keys, n_sort_keys);
    cur = grn_table_cursor_open(ctx, result0, NULL, 0, NULL, 0, 0, -1, 0);
  }

  { // for "not match"
    mrn_ft_info.please = &mrn_default_ft_vft;
    mrn_ft_info.ctx = ctx;
    mrn_ft_info.result = result;
    mrn_ft_info.record_id = GRN_ID_NIL;
  }

  DBUG_RETURN((FT_INFO*) &mrn_ft_info);
}

FT_INFO *ha_mroonga::ft_init_ext(uint flags, uint key_nr, String *key)
{
  MRN_DBUG_ENTER_METHOD();
  fulltext_searching = TRUE;
  if (share->wrapper_mode)
    DBUG_RETURN(wrapper_ft_init_ext(flags, key_nr, key));
  DBUG_RETURN(default_ft_init_ext(flags, key_nr, key));
}

int ha_mroonga::wrapper_ft_read(uchar *buf)
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(0);
}

int ha_mroonga::default_ft_read(uchar *buf)
{
  MRN_DBUG_ENTER_METHOD();
  grn_id found_record_id;

  found_record_id = grn_table_cursor_next(ctx, cur);
  if (ctx->rc) {
    my_message(ER_ERROR_ON_READ, ctx->errbuf, MYF(0));
    grn_table_cursor_close(ctx, cur);
    cur = NULL;
    DBUG_RETURN(ER_ERROR_ON_READ);
  }

  if (found_record_id == GRN_ID_NIL) { // res will be closed by reset()
    grn_table_cursor_close(ctx, cur);
    cur = NULL;
    table->status = STATUS_NOT_FOUND;
    DBUG_RETURN(HA_ERR_END_OF_FILE);
  }
  table->status = 0;

  if (count_skip && record_id != GRN_ID_NIL) {
    DBUG_RETURN(0);
  }

  if (!fast_order_limit) {
    grn_table_get_key(ctx, result, found_record_id, &record_id, sizeof(grn_id));
  } else if (fast_order_limit_with_index) {
    grn_table_get_key(ctx, result0, found_record_id, &record_id, sizeof(grn_id));
  } else {
    grn_id found_record_id2;
    grn_table_get_key(ctx, result0, found_record_id,
                      &found_record_id2, sizeof(grn_id));
    grn_table_get_key(ctx, result, found_record_id2, &record_id, sizeof(grn_id));
  }
  store_fields_from_primary_table(buf, record_id);
  DBUG_RETURN(0);
}

int ha_mroonga::ft_read(uchar *buf)
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
    DBUG_RETURN(wrapper_ft_read(buf));
  DBUG_RETURN(default_ft_read(buf));
}

const COND *ha_mroonga::wrapper_cond_push(const COND *cond)
{
  const COND *ret_cond;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  ret_cond = wrap_handler->cond_push(cond);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(ret_cond);
}

const COND *ha_mroonga::default_cond_push(const COND *cond)
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(NULL);
}

const COND *ha_mroonga::cond_push(const COND *cond)
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
    DBUG_RETURN(wrapper_cond_push(cond));
  DBUG_RETURN(default_cond_push(cond));
}

void ha_mroonga::wrapper_cond_pop()
{
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  wrap_handler->cond_pop();
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_VOID_RETURN;
}

void ha_mroonga::default_cond_pop()
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_VOID_RETURN;
}

void ha_mroonga::cond_pop()
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
    wrapper_cond_pop();
  else
    default_cond_pop();
  DBUG_VOID_RETURN;
}

bool ha_mroonga::wrapper_get_error_message(int error, String *buf)
{
  bool res;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  res = wrap_handler->get_error_message(error, buf);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(res);
}

bool ha_mroonga::default_get_error_message(int error, String *buf)
{
  MRN_DBUG_ENTER_METHOD();
  // latest error message
  buf->copy(ctx->errbuf, (uint) strlen(ctx->errbuf), system_charset_info);
  DBUG_RETURN(FALSE);
}

bool ha_mroonga::get_error_message(int error, String *buf)
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
    DBUG_RETURN(wrapper_get_error_message(error, buf));
  DBUG_RETURN(default_get_error_message(error, buf));
}

void ha_mroonga::check_count_skip(key_part_map start_key_part_map,
                                  key_part_map end_key_part_map, bool fulltext)
{
  MRN_DBUG_ENTER_METHOD();
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
      uint key_nr = active_index;
      KEY key_info = table->key_info[key_nr];
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
  MRN_DBUG_ENTER_METHOD();
  st_select_lex *select_lex = table->pos_in_table_list->select_lex;

  if (
    thd_sql_command(ha_thd()) == SQLCOM_SELECT &&
    !select_lex->with_sum_func &&
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
        sort_keys[i].key = grn_obj_column(ctx, grn_table, col_name, col_name_size);
      } else if (strncmp(MRN_SCORE_COL_NAME, col_name, col_name_size) == 0) {
        sort_keys[i].key = NULL;
      } else {
        sort_keys[i].key = grn_columns[field->field_index];
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
        grn_column_index(ctx, grn_columns[col_field_index], GRN_OP_LESS,
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

void ha_mroonga::store_fields_from_primary_table(uchar *buf, grn_id record_id)
{
  MRN_DBUG_ENTER_METHOD();
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
        field->store((int) record_id);
      } else if (strncmp(MRN_SCORE_COL_NAME, col_name, col_name_size) == 0) {
        // for _score column
        if (result && result->header.flags & GRN_OBJ_WITH_SUBREC) {
          float score;
          grn_obj buf;
          GRN_INT32_INIT(&buf,0);
          grn_id res_id = grn_table_get(ctx, result, &record_id, sizeof(record_id));
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
        mrn_store_field(ctx, field, grn_columns[i], record_id);
      }
      field->move_field_offset(-ptr_diff);
#ifndef DBUG_OFF
      dbug_tmp_restore_column_map(table->write_set, tmp_map);
#endif
    }
  }
  // for "not match against"
  mrn_ft_info.record_id = record_id;

  DBUG_VOID_RETURN;
}

int ha_mroonga::wrapper_reset()
{
  int error;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  error = wrap_handler->ha_reset();
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::default_reset()
{
  MRN_DBUG_ENTER_METHOD();
  if (sort_keys != NULL) {
    free(sort_keys);
    sort_keys = NULL;
  }
  if (result0 != NULL) {
    grn_obj_unlink(ctx, result0);
    result0 = NULL;
  }
  if (result != NULL) {
    grn_obj_unlink(ctx, result);
    _score = NULL;
    result = NULL;
  }
  DBUG_RETURN(0);
}

int ha_mroonga::reset()
{
  int error;
  MRN_DBUG_ENTER_METHOD();
  DBUG_PRINT("info",("mroonga this=%p", this));
  if (share->wrapper_mode)
    error = wrapper_reset();
  else
    error = default_reset();
  ignoring_duplicated_key = FALSE;
  fulltext_searching = FALSE;
  DBUG_RETURN(error);
}

#if MYSQL_VERSION_ID >= 50513
handler *ha_mroonga::wrapper_clone(const char *name, MEM_ROOT *mem_root)
{
  handler *cloned_handler;
  MRN_DBUG_ENTER_METHOD();
  if (!(cloned_handler = get_new_handler(table->s, mem_root,
                                         table->s->db_type())))
    DBUG_RETURN(NULL);
  ((ha_mroonga *) cloned_handler)->is_clone = TRUE;
  ((ha_mroonga *) cloned_handler)->parent_for_clone = this;
  ((ha_mroonga *) cloned_handler)->mem_root_for_clone = mem_root;
  if (cloned_handler->ha_open(table, table->s->normalized_path.str,
                              table->db_stat, HA_OPEN_IGNORE_IF_LOCKED))
  {
    delete cloned_handler;
    DBUG_RETURN(NULL);
  }
  DBUG_RETURN(cloned_handler);
}

handler *ha_mroonga::default_clone(const char *name, MEM_ROOT *mem_root)
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(handler::clone(name, mem_root));
}

handler *ha_mroonga::clone(const char *name, MEM_ROOT *mem_root)
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
    DBUG_RETURN(wrapper_clone(name, mem_root));
  DBUG_RETURN(default_clone(name, mem_root));
}
#else
handler *ha_mroonga::wrapper_clone(MEM_ROOT *mem_root)
{
  handler *cloned_handler;
  MRN_DBUG_ENTER_METHOD();
  if (!(cloned_handler = get_new_handler(table->s, mem_root,
                                         table->s->db_type())))
    DBUG_RETURN(NULL);
  ((ha_mroonga *) cloned_handler)->is_clone = TRUE;
  ((ha_mroonga *) cloned_handler)->parent_for_clone = this;
  ((ha_mroonga *) cloned_handler)->mem_root_for_clone = mem_root;
  if (cloned_handler->ha_open(table, table->s->normalized_path.str,
                              table->db_stat, HA_OPEN_IGNORE_IF_LOCKED))
  {
    delete cloned_handler;
    DBUG_RETURN(NULL);
  }
  DBUG_RETURN(cloned_handler);
}

handler *ha_mroonga::default_clone(MEM_ROOT *mem_root)
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(handler::clone(mem_root));
}

handler *ha_mroonga::clone(MEM_ROOT *mem_root)
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
    DBUG_RETURN(wrapper_clone(mem_root));
  DBUG_RETURN(default_clone(mem_root));
}
#endif

uint8 ha_mroonga::wrapper_table_cache_type()
{
  uint8 res;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  res = wrap_handler->table_cache_type();
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(res);
}

uint8 ha_mroonga::default_table_cache_type()
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(handler::table_cache_type());
}

uint8 ha_mroonga::table_cache_type()
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
    DBUG_RETURN(wrapper_table_cache_type());
  DBUG_RETURN(default_table_cache_type());
}

int ha_mroonga::wrapper_read_multi_range_first(KEY_MULTI_RANGE **found_range_p,
                                               KEY_MULTI_RANGE *ranges,
                                               uint range_count,
                                               bool sorted,
                                               HANDLER_BUFFER *buffer)
{
  int error;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  error = wrap_handler->read_multi_range_first(found_range_p, ranges,
                                                  range_count, sorted, buffer);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::default_read_multi_range_first(KEY_MULTI_RANGE **found_range_p,
                                               KEY_MULTI_RANGE *ranges,
                                               uint range_count,
                                               bool sorted,
                                               HANDLER_BUFFER *buffer)
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(handler::read_multi_range_first(found_range_p, ranges,
                                              range_count, sorted, buffer));
}

int ha_mroonga::read_multi_range_first(KEY_MULTI_RANGE **found_range_p,
                                       KEY_MULTI_RANGE *ranges,
                                       uint range_count,
                                       bool sorted,
                                       HANDLER_BUFFER *buffer)
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
    DBUG_RETURN(wrapper_read_multi_range_first(found_range_p, ranges,
                                               range_count, sorted, buffer));
  DBUG_RETURN(default_read_multi_range_first(found_range_p, ranges,
                                             range_count, sorted, buffer));
}

int ha_mroonga::wrapper_read_multi_range_next(KEY_MULTI_RANGE **found_range_p)
{
  int error;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  error = wrap_handler->read_multi_range_next(found_range_p);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::default_read_multi_range_next(KEY_MULTI_RANGE **found_range_p)
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(handler::read_multi_range_next(found_range_p));
}

int ha_mroonga::read_multi_range_next(KEY_MULTI_RANGE **found_range_p)
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
    DBUG_RETURN(wrapper_read_multi_range_next(found_range_p));
  DBUG_RETURN(default_read_multi_range_next(found_range_p));
}

void ha_mroonga::wrapper_start_bulk_insert(ha_rows rows)
{
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  wrap_handler->ha_start_bulk_insert(rows);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_VOID_RETURN;
}

void ha_mroonga::default_start_bulk_insert(ha_rows rows)
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_VOID_RETURN;
}

void ha_mroonga::start_bulk_insert(ha_rows rows)
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
    wrapper_start_bulk_insert(rows);
  else
    default_start_bulk_insert(rows);
  DBUG_VOID_RETURN;
}

int ha_mroonga::wrapper_end_bulk_insert()
{
  int error;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  error = wrap_handler->ha_end_bulk_insert();
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::default_end_bulk_insert()
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(0);
}

int ha_mroonga::end_bulk_insert()
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
    DBUG_RETURN(wrapper_end_bulk_insert());
  DBUG_RETURN(default_end_bulk_insert());
}

int ha_mroonga::wrapper_delete_all_rows()
{
  int error;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  error = wrap_handler->ha_delete_all_rows();
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::default_delete_all_rows()
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN((my_errno = HA_ERR_WRONG_COMMAND));
}

int ha_mroonga::delete_all_rows()
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
    DBUG_RETURN(wrapper_delete_all_rows());
  DBUG_RETURN(default_delete_all_rows());
}

int ha_mroonga::wrapper_truncate()
{
  int error;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  error = wrap_handler->ha_truncate();
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::default_truncate()
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(HA_ERR_WRONG_COMMAND);
}

int ha_mroonga::truncate()
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
    DBUG_RETURN(wrapper_truncate());
  DBUG_RETURN(default_truncate());
}

double ha_mroonga::wrapper_scan_time()
{
  double res;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  res = wrap_handler->scan_time();
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(res);
}

double ha_mroonga::default_scan_time()
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(handler::scan_time());
}

double ha_mroonga::scan_time()
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
    DBUG_RETURN(wrapper_scan_time());
  DBUG_RETURN(default_scan_time());
}

double ha_mroonga::wrapper_read_time(uint index, uint ranges, ha_rows rows)
{
  double res;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  res = wrap_handler->read_time(share->wrap_key_nr[index], ranges, rows);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(res);
}

double ha_mroonga::default_read_time(uint index, uint ranges, ha_rows rows)
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(handler::read_time(index, ranges, rows));
}

double ha_mroonga::read_time(uint index, uint ranges, ha_rows rows)
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
    DBUG_RETURN(wrapper_read_time(index, ranges, rows));
  DBUG_RETURN(default_read_time(index, ranges, rows));
}

const key_map *ha_mroonga::wrapper_keys_to_use_for_scanning()
{
  const key_map *res;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  res = wrap_handler->keys_to_use_for_scanning();
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(res);
}

const key_map *ha_mroonga::default_keys_to_use_for_scanning()
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(handler::keys_to_use_for_scanning());
}

const key_map *ha_mroonga::keys_to_use_for_scanning()
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
    DBUG_RETURN(wrapper_keys_to_use_for_scanning());
  DBUG_RETURN(default_keys_to_use_for_scanning());
}

ha_rows ha_mroonga::wrapper_estimate_rows_upper_bound()
{
  ha_rows res;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  res = wrap_handler->estimate_rows_upper_bound();
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(res);
}

ha_rows ha_mroonga::default_estimate_rows_upper_bound()
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(handler::estimate_rows_upper_bound());
}

ha_rows ha_mroonga::estimate_rows_upper_bound()
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
    DBUG_RETURN(wrapper_estimate_rows_upper_bound());
  DBUG_RETURN(default_estimate_rows_upper_bound());
}

void ha_mroonga::wrapper_update_create_info(HA_CREATE_INFO* create_info)
{
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  wrap_handler->update_create_info(create_info);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_VOID_RETURN;
}

void ha_mroonga::default_update_create_info(HA_CREATE_INFO* create_info)
{
  MRN_DBUG_ENTER_METHOD();
  handler::update_create_info(create_info);
  DBUG_VOID_RETURN;
}

void ha_mroonga::update_create_info(HA_CREATE_INFO* create_info)
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
    wrapper_update_create_info(create_info);
  else
    default_update_create_info(create_info);
  DBUG_VOID_RETURN;
}

int ha_mroonga::wrapper_rename_table(const char *from, const char *to)
{
  int error;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  error = wrap_handler->ha_rename_table(from, to);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::default_rename_table(const char *from, const char *to)
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(handler::rename_table(from, to));
}

int ha_mroonga::rename_table(const char *from, const char *to)
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
    DBUG_RETURN(wrapper_rename_table(from, to));
  DBUG_RETURN(default_rename_table(from, to));
}

bool ha_mroonga::wrapper_is_crashed() const
{
  bool res;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  res = wrap_handler->is_crashed();
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(res);
}

bool ha_mroonga::default_is_crashed() const
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(handler::is_crashed());
}

bool ha_mroonga::is_crashed() const
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
    DBUG_RETURN(wrapper_is_crashed());
  DBUG_RETURN(default_is_crashed());
}

bool ha_mroonga::wrapper_auto_repair() const
{
  bool res;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  res = wrap_handler->auto_repair();
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(res);
}

bool ha_mroonga::default_auto_repair() const
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(handler::auto_repair());
}

bool ha_mroonga::auto_repair() const
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
    DBUG_RETURN(wrapper_auto_repair());
  DBUG_RETURN(default_auto_repair());
}

int ha_mroonga::wrapper_disable_indexes(uint mode)
{
  int error;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  error = wrap_handler->ha_disable_indexes(mode);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::default_disable_indexes(uint mode)
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(HA_ERR_WRONG_COMMAND);
}

int ha_mroonga::disable_indexes(uint mode)
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
    DBUG_RETURN(wrapper_disable_indexes(mode));
  DBUG_RETURN(default_disable_indexes(mode));
}

int ha_mroonga::wrapper_enable_indexes(uint mode)
{
  int error;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  error = wrap_handler->ha_enable_indexes(mode);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::default_enable_indexes(uint mode)
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(HA_ERR_WRONG_COMMAND);
}

int ha_mroonga::enable_indexes(uint mode)
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
    DBUG_RETURN(wrapper_enable_indexes(mode));
  DBUG_RETURN(default_enable_indexes(mode));
}

int ha_mroonga::wrapper_check(THD* thd, HA_CHECK_OPT* check_opt)
{
  int error;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  error = wrap_handler->ha_check(thd, check_opt);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::default_check(THD* thd, HA_CHECK_OPT* check_opt)
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(HA_ADMIN_NOT_IMPLEMENTED);
}

int ha_mroonga::check(THD* thd, HA_CHECK_OPT* check_opt)
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
    DBUG_RETURN(wrapper_check(thd, check_opt));
  DBUG_RETURN(default_check(thd, check_opt));
}

int ha_mroonga::wrapper_repair(THD* thd, HA_CHECK_OPT* check_opt)
{
  int error;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  error = wrap_handler->ha_repair(thd, check_opt);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::default_repair(THD* thd, HA_CHECK_OPT* check_opt)
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(HA_ADMIN_NOT_IMPLEMENTED);
}

int ha_mroonga::repair(THD* thd, HA_CHECK_OPT* check_opt)
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
    DBUG_RETURN(wrapper_repair(thd, check_opt));
  DBUG_RETURN(default_repair(thd, check_opt));
}

bool ha_mroonga::wrapper_check_and_repair(THD *thd)
{
  int error;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  error = wrap_handler->ha_check_and_repair(thd);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

bool ha_mroonga::default_check_and_repair(THD *thd)
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(TRUE);
}

bool ha_mroonga::check_and_repair(THD *thd)
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
    DBUG_RETURN(wrapper_check_and_repair(thd));
  DBUG_RETURN(default_check_and_repair(thd));
}

int ha_mroonga::wrapper_analyze(THD* thd, HA_CHECK_OPT* check_opt)
{
  int error;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  error = wrap_handler->ha_analyze(thd, check_opt);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::default_analyze(THD* thd, HA_CHECK_OPT* check_opt)
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(HA_ADMIN_NOT_IMPLEMENTED);
}

int ha_mroonga::analyze(THD* thd, HA_CHECK_OPT* check_opt)
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
    DBUG_RETURN(wrapper_analyze(thd, check_opt));
  DBUG_RETURN(default_analyze(thd, check_opt));
}

int ha_mroonga::wrapper_optimize(THD* thd, HA_CHECK_OPT* check_opt)
{
  int error;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  error = wrap_handler->ha_optimize(thd, check_opt);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::default_optimize(THD* thd, HA_CHECK_OPT* check_opt)
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(HA_ADMIN_NOT_IMPLEMENTED);
}

int ha_mroonga::optimize(THD* thd, HA_CHECK_OPT* check_opt)
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
    DBUG_RETURN(wrapper_optimize(thd, check_opt));
  DBUG_RETURN(default_optimize(thd, check_opt));
}

bool ha_mroonga::wrapper_is_fatal_error(int error_num, uint flags)
{
  bool res;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  res = wrap_handler->is_fatal_error(error_num, flags);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(res);
}

bool ha_mroonga::default_is_fatal_error(int error_num, uint flags)
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(handler::is_fatal_error(error_num, flags));
}

bool ha_mroonga::is_fatal_error(int error_num, uint flags)
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
    DBUG_RETURN(wrapper_is_fatal_error(error_num, flags));
  DBUG_RETURN(default_is_fatal_error(error_num, flags));
}

#ifdef __cplusplus
}
#endif
