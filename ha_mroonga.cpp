/* -*- c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
  Copyright(C) 2010 Tetsuro IKEDA
  Copyright(C) 2010-2012 Kentoku SHIBA
  Copyright(C) 2011-2012 Kouhei Sutou <kou@clear-code.com>

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

#include "mrn_mysql.h"

#ifdef USE_PRAGMA_IMPLEMENTATION
#pragma implementation
#endif

#if MYSQL_VERSION_ID >= 50500
#  include <sql_plugin.h>
#  include <sql_show.h>
#  include <key.h>
#endif

#include <sql_select.h>
#include <ft_global.h>
#include <spatial.h>
#include <mysql.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifndef WIN32
#  include <dirent.h>
#  include <unistd.h>
#define MRN_MKDIR(pathname, mode) mkdir((pathname), (mode))
#else
#  include <math.h>
inline double round(double x) { return (floor(x + 0.5)); }
#  include <direct.h>
#define MRN_MKDIR(pathname, mode) _mkdir((pathname))
#endif

#include "mrn_err.h"
#include "mrn_table.hpp"
#include "ha_mroonga.hpp"
#include <mrn_path_mapper.hpp>
#include <mrn_index_table_name.hpp>
#include <mrn_debug_column_access.hpp>
#include <mrn_auto_increment_value_lock.hpp>
#include <mrn_external_lock.hpp>
#include <mrn_match_escalation_threshold_scope.hpp>

#define MRN_SHORT_TEXT_SIZE (1 << 12) //  4Kbytes
#define MRN_TEXT_SIZE       (1 << 16) // 64Kbytes
#define MRN_LONG_TEXT_SIZE  (1 << 31) //  2Gbytes

#if MYSQL_VERSION_ID >= 50500
extern mysql_mutex_t LOCK_open;
#  define mrn_open_mutex_lock() mysql_mutex_lock(&LOCK_open)
#  define mrn_open_mutex_unlock() mysql_mutex_unlock(&LOCK_open)
#else
extern pthread_mutex_t LOCK_open;
#  define mrn_open_mutex_lock()
#  define mrn_open_mutex_unlock()
#endif

#if MYSQL_VERSION_ID >= 50603
#  define MRN_ORDER_IS_ASC(order) ((order)->direction == ORDER::ORDER_ASC)
#else
#  define MRN_ORDER_IS_ASC(order) ((order)->asc)
#endif

#define MRN_STRINGIFY(macro_or_string)  MRN_STRINGIFY_ARG(macro_or_string)
#define MRN_STRINGIFY_ARG(contents)     #contents

#ifdef MRN_USE_DEPRECATED_NAME
#  define MRN_PLUGIN_NAME groonga
#else
#  define MRN_PLUGIN_NAME mroonga
#endif
#define MRN_PLUGIN_NAME_STRING MRN_STRINGIFY(MRN_PLUGIN_NAME)

#ifdef MRN_MARIADB_P
#  define st_mysql_plugin          st_maria_plugin
#  define mrn_declare_plugin(NAME) maria_declare_plugin(NAME)
#  define mrn_declare_plugin_end   maria_declare_plugin_end
#  define MRN_PLUGIN_LAST_VALUES   MRN_VERSION, MariaDB_PLUGIN_MATURITY_STABLE
#else
#  define mrn_declare_plugin(NAME) mysql_declare_plugin(NAME)
#  define mrn_declare_plugin_end   mysql_declare_plugin_end
#  ifdef MRN_PLUGIN_HAVE_FLAGS
#    define MRN_PLUGIN_LAST_VALUES NULL, 0
#  else
#    define MRN_PLUGIN_LAST_VALUES NULL
#  endif
#endif

static const char *index_column_name = "index";
static const char *mrn_plugin_author = "The mroonga project";

#ifdef __cplusplus
extern "C" {
#endif

/* groonga's internal functions */
const char *grn_obj_get_value_(grn_ctx *ctx, grn_obj *obj, grn_id id, uint32 *size);

/* global variables */
static pthread_mutex_t mrn_db_mutex;
static pthread_mutex_t mrn_log_mutex;
handlerton *mrn_hton_ptr;
HASH mrn_open_tables;
pthread_mutex_t mrn_open_tables_mutex;

/* internal variables */
static grn_ctx mrn_ctx;
static grn_obj *mrn_db;
static grn_hash *mrn_hash;

static CHARSET_INFO *mrn_charset_utf8 = NULL;
static CHARSET_INFO *mrn_charset_binary = NULL;
static CHARSET_INFO *mrn_charset_ascii = NULL;
static CHARSET_INFO *mrn_charset_latin1_1 = NULL;
static CHARSET_INFO *mrn_charset_latin1_2 = NULL;
static CHARSET_INFO *mrn_charset_cp932 = NULL;
static CHARSET_INFO *mrn_charset_sjis = NULL;
static CHARSET_INFO *mrn_charset_eucjpms = NULL;
static CHARSET_INFO *mrn_charset_ujis = NULL;
static CHARSET_INFO *mrn_charset_koi8r = NULL;

static int32 mrn_utc_diff_in_seconds = 0;

static void mrn_init_encoding_map()
{
  CHARSET_INFO **cs;
  MRN_DBUG_ENTER_FUNCTION();
  for (cs = all_charsets; cs < all_charsets + MY_ALL_CHARSETS_SIZE; cs++)
  {
    if (!cs[0])
      continue;
    if (!strcmp(cs[0]->csname, "utf8"))
    {
      DBUG_PRINT("info", ("mroonga: %s is %s [%p]",
                          cs[0]->name, cs[0]->csname, cs[0]->cset));
      if (!mrn_charset_utf8)
        mrn_charset_utf8 = cs[0];
      else if (mrn_charset_utf8->cset != cs[0]->cset)
        DBUG_ASSERT(0);
      continue;
    }
    if (!strcmp(cs[0]->csname, "binary"))
    {
      DBUG_PRINT("info", ("mroonga: %s is %s [%p]",
                          cs[0]->name, cs[0]->csname, cs[0]->cset));
      if (!mrn_charset_binary)
        mrn_charset_binary = cs[0];
      else if (mrn_charset_binary->cset != cs[0]->cset)
        DBUG_ASSERT(0);
      continue;
    }
    if (!strcmp(cs[0]->csname, "ascii"))
    {
      DBUG_PRINT("info", ("mroonga: %s is %s [%p]",
                          cs[0]->name, cs[0]->csname, cs[0]->cset));
      if (!mrn_charset_ascii)
        mrn_charset_ascii = cs[0];
      else if (mrn_charset_ascii->cset != cs[0]->cset)
        DBUG_ASSERT(0);
      continue;
    }
    if (!strcmp(cs[0]->csname, "latin1"))
    {
      DBUG_PRINT("info", ("mroonga: %s is %s [%p]",
                          cs[0]->name, cs[0]->csname, cs[0]->cset));
      if (!mrn_charset_latin1_1)
        mrn_charset_latin1_1 = cs[0];
      else if (mrn_charset_latin1_1->cset != cs[0]->cset)
      {
        if (!mrn_charset_latin1_2)
          mrn_charset_latin1_2 = cs[0];
        else if (mrn_charset_latin1_2->cset != cs[0]->cset)
          DBUG_ASSERT(0);
      }
      continue;
    }
    if (!strcmp(cs[0]->csname, "cp932"))
    {
      DBUG_PRINT("info", ("mroonga: %s is %s [%p]",
                          cs[0]->name, cs[0]->csname, cs[0]->cset));
      if (!mrn_charset_cp932)
        mrn_charset_cp932 = cs[0];
      else if (mrn_charset_cp932->cset != cs[0]->cset)
        DBUG_ASSERT(0);
      continue;
    }
    if (!strcmp(cs[0]->csname, "sjis"))
    {
      DBUG_PRINT("info", ("mroonga: %s is %s [%p]",
                          cs[0]->name, cs[0]->csname, cs[0]->cset));
      if (!mrn_charset_sjis)
        mrn_charset_sjis = cs[0];
      else if (mrn_charset_sjis->cset != cs[0]->cset)
        DBUG_ASSERT(0);
      continue;
    }
    if (!strcmp(cs[0]->csname, "eucjpms"))
    {
      DBUG_PRINT("info", ("mroonga: %s is %s [%p]",
                          cs[0]->name, cs[0]->csname, cs[0]->cset));
      if (!mrn_charset_eucjpms)
        mrn_charset_eucjpms = cs[0];
      else if (mrn_charset_eucjpms->cset != cs[0]->cset)
        DBUG_ASSERT(0);
      continue;
    }
    if (!strcmp(cs[0]->csname, "ujis"))
    {
      DBUG_PRINT("info", ("mroonga: %s is %s [%p]",
                          cs[0]->name, cs[0]->csname, cs[0]->cset));
      if (!mrn_charset_ujis)
        mrn_charset_ujis = cs[0];
      else if (mrn_charset_ujis->cset != cs[0]->cset)
        DBUG_ASSERT(0);
      continue;
    }
    if (!strcmp(cs[0]->csname, "koi8r"))
    {
      DBUG_PRINT("info", ("mroonga: %s is %s [%p]",
                          cs[0]->name, cs[0]->csname, cs[0]->cset));
      if (!mrn_charset_koi8r)
        mrn_charset_koi8r = cs[0];
      else if (mrn_charset_koi8r->cset != cs[0]->cset)
        DBUG_ASSERT(0);
      continue;
    }
    DBUG_PRINT("info", ("mroonga: %s[%s][%p] is not supported",
                        cs[0]->name, cs[0]->csname, cs[0]->cset));
  }
  DBUG_VOID_RETURN;
}

static int mrn_change_encoding(grn_ctx *ctx, const CHARSET_INFO *charset)
{
  MRN_DBUG_ENTER_FUNCTION();
  if (!charset)
  {
    GRN_CTX_SET_ENCODING(ctx, GRN_ENC_NONE);
    DBUG_RETURN(0);
  }
  if (charset->cset == mrn_charset_utf8->cset)
  {
    GRN_CTX_SET_ENCODING(ctx, GRN_ENC_UTF8);
    DBUG_RETURN(0);
  }
  if (charset->cset == mrn_charset_cp932->cset)
  {
    GRN_CTX_SET_ENCODING(ctx, GRN_ENC_SJIS);
    DBUG_RETURN(0);
  }
  if (charset->cset == mrn_charset_eucjpms->cset)
  {
    GRN_CTX_SET_ENCODING(ctx, GRN_ENC_EUC_JP);
    DBUG_RETURN(0);
  }
  if (charset->cset == mrn_charset_latin1_1->cset)
  {
    GRN_CTX_SET_ENCODING(ctx, GRN_ENC_LATIN1);
    DBUG_RETURN(0);
  }
  if (charset->cset == mrn_charset_latin1_2->cset)
  {
    GRN_CTX_SET_ENCODING(ctx, GRN_ENC_LATIN1);
    DBUG_RETURN(0);
  }
  if (charset->cset == mrn_charset_koi8r->cset)
  {
    GRN_CTX_SET_ENCODING(ctx, GRN_ENC_KOI8R);
    DBUG_RETURN(0);
  }
  if (charset->cset == mrn_charset_binary->cset)
  {
    GRN_CTX_SET_ENCODING(ctx, GRN_ENC_NONE);
    DBUG_RETURN(0);
  }
  if (charset->cset == mrn_charset_ascii->cset)
  {
    GRN_CTX_SET_ENCODING(ctx, GRN_ENC_UTF8);
    DBUG_RETURN(0);
  }
  if (charset->cset == mrn_charset_sjis->cset)
  {
    GRN_CTX_SET_ENCODING(ctx, GRN_ENC_SJIS);
    DBUG_RETURN(0);
  }
  if (charset->cset == mrn_charset_ujis->cset)
  {
    GRN_CTX_SET_ENCODING(ctx, GRN_ENC_EUC_JP);
    DBUG_RETURN(0);
  }
  GRN_CTX_SET_ENCODING(ctx, GRN_ENC_NONE);
  my_printf_error(ER_MRN_CHARSET_NOT_SUPPORT_NUM,
    ER_MRN_CHARSET_NOT_SUPPORT_STR,
    MYF(0), charset->name, charset->csname);
  DBUG_RETURN(ER_MRN_CHARSET_NOT_SUPPORT_NUM);
}

#if !defined(DBUG_OFF) && !defined(_lint)
static const char *mrn_inspect_thr_lock_type(enum thr_lock_type lock_type)
{
  const char *inspected = "<unknown>";
  switch (lock_type) {
  case TL_IGNORE:
    inspected = "TL_IGNORE";
    break;
  case TL_UNLOCK:
    inspected = "TL_UNLOCK";
    break;
  case TL_READ_DEFAULT:
    inspected = "TL_READ_DEFAULT";
    break;
  case TL_READ:
    inspected = "TL_READ";
    break;
  case TL_READ_WITH_SHARED_LOCKS:
    inspected = "TL_READ_WITH_SHARED_LOCKS";
    break;
  case TL_READ_HIGH_PRIORITY:
    inspected = "TL_READ_HIGH_PRIORITY";
    break;
  case TL_READ_NO_INSERT:
    inspected = "TL_READ_NO_INSERT";
    break;
  case TL_WRITE_ALLOW_WRITE:
    inspected = "TL_WRITE_ALLOW_WRITE";
    break;
#ifdef MRN_HAVE_TL_WRITE_ALLOW_READ
  case TL_WRITE_ALLOW_READ:
    inspected = "TL_WRITE_ALLOW_READ";
    break;
#endif
  case TL_WRITE_CONCURRENT_INSERT:
    inspected = "TL_WRITE_CONCURRENT_INSERT";
    break;
  case TL_WRITE_DELAYED:
    inspected = "TL_WRITE_DELAYED";
    break;
  case TL_WRITE_DEFAULT:
    inspected = "TL_WRITE_DEFAULT";
    break;
  case TL_WRITE_LOW_PRIORITY:
    inspected = "TL_WRITE_LOW_PRIORITY";
    break;
  case TL_WRITE:
    inspected = "TL_WRITE";
    break;
  case TL_WRITE_ONLY:
    inspected = "TL_WRITE_ONLY";
    break;
  }
  return inspected;
}

static const char *mrn_inspect_extra_function(enum ha_extra_function operation)
{
  const char *inspected = "<unknown>";
  switch (operation) {
  case HA_EXTRA_NORMAL:
    inspected = "HA_EXTRA_NORMAL";
    break;
  case HA_EXTRA_QUICK:
    inspected = "HA_EXTRA_QUICK";
    break;
  case HA_EXTRA_NOT_USED:
    inspected = "HA_EXTRA_NOT_USED";
    break;
  case HA_EXTRA_CACHE:
    inspected = "HA_EXTRA_CACHE";
    break;
  case HA_EXTRA_NO_CACHE:
    inspected = "HA_EXTRA_NO_CACHE";
    break;
  case HA_EXTRA_NO_READCHECK:
    inspected = "HA_EXTRA_NO_READCHECK";
    break;
  case HA_EXTRA_READCHECK:
    inspected = "HA_EXTRA_READCHECK";
    break;
  case HA_EXTRA_KEYREAD:
    inspected = "HA_EXTRA_KEYREAD";
    break;
  case HA_EXTRA_NO_KEYREAD:
    inspected = "HA_EXTRA_NO_KEYREAD";
    break;
  case HA_EXTRA_NO_USER_CHANGE:
    inspected = "HA_EXTRA_NO_USER_CHANGE";
    break;
  case HA_EXTRA_KEY_CACHE:
    inspected = "HA_EXTRA_KEY_CACHE";
    break;
  case HA_EXTRA_NO_KEY_CACHE:
    inspected = "HA_EXTRA_NO_KEY_CACHE";
    break;
  case HA_EXTRA_WAIT_LOCK:
    inspected = "HA_EXTRA_WAIT_LOCK";
    break;
  case HA_EXTRA_NO_WAIT_LOCK:
    inspected = "HA_EXTRA_NO_WAIT_LOCK";
    break;
  case HA_EXTRA_WRITE_CACHE:
    inspected = "HA_EXTRA_WRITE_CACHE";
    break;
  case HA_EXTRA_FLUSH_CACHE:
    inspected = "HA_EXTRA_FLUSH_CACHE";
    break;
  case HA_EXTRA_NO_KEYS:
    inspected = "HA_EXTRA_NO_KEYS";
    break;
  case HA_EXTRA_KEYREAD_CHANGE_POS:
    inspected = "HA_EXTRA_KEYREAD_CHANGE_POS";
    break;
  case HA_EXTRA_REMEMBER_POS:
    inspected = "HA_EXTRA_REMEMBER_POS";
    break;
  case HA_EXTRA_RESTORE_POS:
    inspected = "HA_EXTRA_RESTORE_POS";
    break;
  case HA_EXTRA_REINIT_CACHE:
    inspected = "HA_EXTRA_REINIT_CACHE";
    break;
  case HA_EXTRA_FORCE_REOPEN:
    inspected = "HA_EXTRA_FORCE_REOPEN";
    break;
  case HA_EXTRA_FLUSH:
    inspected = "HA_EXTRA_FLUSH";
    break;
  case HA_EXTRA_NO_ROWS:
    inspected = "HA_EXTRA_NO_ROWS";
    break;
  case HA_EXTRA_RESET_STATE:
    inspected = "HA_EXTRA_RESET_STATE";
    break;
  case HA_EXTRA_IGNORE_DUP_KEY:
    inspected = "HA_EXTRA_IGNORE_DUP_KEY";
    break;
  case HA_EXTRA_NO_IGNORE_DUP_KEY:
    inspected = "HA_EXTRA_NO_IGNORE_DUP_KEY";
    break;
  case HA_EXTRA_PREPARE_FOR_DROP:
    inspected = "HA_EXTRA_PREPARE_FOR_DROP";
    break;
  case HA_EXTRA_PREPARE_FOR_UPDATE:
    inspected = "HA_EXTRA_PREPARE_FOR_UPDATE";
    break;
  case HA_EXTRA_PRELOAD_BUFFER_SIZE:
    inspected = "HA_EXTRA_PRELOAD_BUFFER_SIZE";
    break;
  case HA_EXTRA_CHANGE_KEY_TO_UNIQUE:
    inspected = "HA_EXTRA_CHANGE_KEY_TO_UNIQUE";
    break;
  case HA_EXTRA_CHANGE_KEY_TO_DUP:
    inspected = "HA_EXTRA_CHANGE_KEY_TO_DUP";
    break;
  case HA_EXTRA_KEYREAD_PRESERVE_FIELDS:
    inspected = "HA_EXTRA_KEYREAD_PRESERVE_FIELDS";
    break;
  case HA_EXTRA_MMAP:
    inspected = "HA_EXTRA_MMAP";
    break;
  case HA_EXTRA_IGNORE_NO_KEY:
    inspected = "HA_EXTRA_IGNORE_NO_KEY";
    break;
  case HA_EXTRA_NO_IGNORE_NO_KEY:
    inspected = "HA_EXTRA_NO_IGNORE_NO_KEY";
    break;
  case HA_EXTRA_MARK_AS_LOG_TABLE:
    inspected = "HA_EXTRA_MARK_AS_LOG_TABLE";
    break;
  case HA_EXTRA_WRITE_CAN_REPLACE:
    inspected = "HA_EXTRA_WRITE_CAN_REPLACE";
    break;
  case HA_EXTRA_WRITE_CANNOT_REPLACE:
    inspected = "HA_EXTRA_WRITE_CANNOT_REPLACE";
    break;
  case HA_EXTRA_DELETE_CANNOT_BATCH:
    inspected = "HA_EXTRA_DELETE_CANNOT_BATCH";
    break;
  case HA_EXTRA_UPDATE_CANNOT_BATCH:
    inspected = "HA_EXTRA_UPDATE_CANNOT_BATCH";
    break;
  case HA_EXTRA_INSERT_WITH_UPDATE:
    inspected = "HA_EXTRA_INSERT_WITH_UPDATE";
    break;
  case HA_EXTRA_PREPARE_FOR_RENAME:
    inspected = "HA_EXTRA_PREPARE_FOR_RENAME";
    break;
#ifdef MRN_HAVE_HA_EXTRA_ADD_CHILDREN_LIST
  case HA_EXTRA_ADD_CHILDREN_LIST:
    inspected = "HA_EXTRA_ADD_CHILDREN_LIST";
    break;
#endif
  case HA_EXTRA_ATTACH_CHILDREN:
    inspected = "HA_EXTRA_ATTACH_CHILDREN";
    break;
#ifdef MRN_HAVE_HA_EXTRA_IS_ATTACHED_CHILDREN
  case HA_EXTRA_IS_ATTACHED_CHILDREN:
    inspected = "HA_EXTRA_IS_ATTACHED_CHILDREN";
    break;
#endif
  case HA_EXTRA_DETACH_CHILDREN:
    inspected = "HA_EXTRA_DETACH_CHILDREN";
    break;
#ifdef MRN_HAVE_HA_EXTRA_DETACH_CHILD
  case HA_EXTRA_DETACH_CHILD:
    inspected = "HA_EXTRA_DETACH_CHILD";
    break;
#endif
#ifdef MRN_HAVE_HA_EXTRA_PREPARE_FOR_FORCED_CLOSE
  case HA_EXTRA_PREPARE_FOR_FORCED_CLOSE:
    inspected = "HA_EXTRA_PREPARE_FOR_FORCED_CLOSE";
    break;
#endif
  }
  return inspected;
}
#endif

static uchar *mrn_open_tables_get_key(const uchar *record,
                                      size_t *length,
                                      my_bool not_used __attribute__ ((unused)))
{
  MRN_DBUG_ENTER_FUNCTION();
  MRN_SHARE *share = reinterpret_cast<MRN_SHARE *>(const_cast<uchar *>(record));
  *length = share->table_name_length;
  DBUG_RETURN(reinterpret_cast<uchar *>(share->table_name));
}

/* status */
static long mrn_count_skip = 0;
static long mrn_fast_order_limit = 0;

/* logging */
static char *mrn_log_file_path = NULL;
static FILE *mrn_log_file = NULL;
static bool mrn_log_file_opened = false;
static grn_log_level mrn_log_level_default = GRN_LOG_DEFAULT_LEVEL;
static ulong mrn_log_level = mrn_log_level_default;
char *mrn_default_parser = NULL;
char *mrn_default_wrapper_engine = NULL;
static char *mrn_database_path_prefix = NULL;
static char *mrn_libgroonga_version = const_cast<char *>(grn_get_version());
static char *mrn_version = const_cast<char *>(MRN_VERSION);

static void mrn_logger_func(int level, const char *time, const char *title,
                            const char *msg, const char *location,
                            void *func_arg)
{
  const char slev[] = " EACewnid-";
  if (mrn_log_file_opened) {
    pthread_mutex_lock(&mrn_log_mutex);
    fprintf(mrn_log_file, "%s|%c|%08x|%s\n", time,
            *(slev + level),
            static_cast<uint>((ulong)(pthread_self())),
            msg);
    fflush(mrn_log_file);
    pthread_mutex_unlock(&mrn_log_mutex);
  }
}

static grn_logger_info mrn_logger_info = {
  mrn_log_level_default,
  GRN_LOG_TIME|GRN_LOG_MESSAGE,
  mrn_logger_func,
  NULL
};

/* global hashes and mutexes */
HASH mrn_allocated_thds;
pthread_mutex_t mrn_allocated_thds_mutex;
static uchar *mrn_allocated_thds_get_key(const uchar *record,
                                         size_t *length,
                                         my_bool not_used __attribute__ ((unused)))
{
  MRN_DBUG_ENTER_FUNCTION();
  *length = sizeof(THD *);
  DBUG_RETURN(const_cast<uchar *>(record));
}

/* system functions */

static struct st_mysql_storage_engine storage_engine_structure =
{ MYSQL_HANDLERTON_INTERFACE_VERSION };

static struct st_mysql_show_var mrn_status_variables[] =
{
  {MRN_PLUGIN_NAME_STRING "_count_skip",
   (char *)&mrn_count_skip, SHOW_LONG},
  {MRN_PLUGIN_NAME_STRING "_fast_order_limit",
   (char *)&mrn_fast_order_limit, SHOW_LONG},
  {NullS, NullS, SHOW_LONG}
};

static const char *mrn_log_level_type_names[] = { "NONE", "EMERG", "ALERT",
                                                  "CRIT", "ERROR", "WARNING",
                                                  "NOTICE", "INFO", "DEBUG",
                                                  "DUMP", NullS };
static TYPELIB mrn_log_level_typelib =
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
  mrn_change_encoding(ctx, system_charset_info);
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

static void mrn_log_file_update(THD *thd, struct st_mysql_sys_var *var,
                                void *var_ptr, const void *save)
{
  MRN_DBUG_ENTER_FUNCTION();
  const char *new_value = *((const char **)save);
  char **old_value_ptr = (char **)var_ptr;
  grn_ctx ctx;

  grn_ctx_init(&ctx, 0);
  mrn_change_encoding(&ctx, system_charset_info);
  if (strcmp(*old_value_ptr, new_value) == 0) {
    GRN_LOG(&ctx, GRN_LOG_NOTICE,
            "log file isn't changed "
            "because the requested path isn't different: <%s>",
            new_value);
  } else {
    GRN_LOG(&ctx, GRN_LOG_NOTICE,
            "log file is changed: <%s> -> <%s>",
            *old_value_ptr, new_value);

    pthread_mutex_lock(&mrn_log_mutex);
    FILE *new_log_file;
    int log_file_open_errno = 0;
    new_log_file = fopen(new_value, "a");
    if (new_log_file) {
      if (mrn_log_file_opened) {
        fclose(mrn_log_file);
      }
      mrn_log_file = new_log_file;
      mrn_log_file_opened = true;
    } else {
      log_file_open_errno = errno;
    }
    pthread_mutex_unlock(&mrn_log_mutex);

    if (log_file_open_errno == 0) {
      GRN_LOG(&ctx, GRN_LOG_NOTICE,
              "log file is changed: <%s> -> <%s>",
              *old_value_ptr, new_value);
      *old_value_ptr = my_strdup(new_value, MYF(MY_WME));
    } else {
      if (mrn_log_file) {
        GRN_LOG(&ctx, GRN_LOG_ERROR,
                "log file isn't changed "
                "because the requested path can't be opened: <%s>: <%s>",
                new_value, strerror(log_file_open_errno));
      } else {
        GRN_LOG(&ctx, GRN_LOG_ERROR,
                "log file can't be opened: <%s>: <%s>",
                new_value, strerror(log_file_open_errno));
      }
      *old_value_ptr = my_strdup(*old_value_ptr, MYF(MY_WME));
    }
  }
  grn_ctx_fin(&ctx);

  DBUG_VOID_RETURN;
}

static MYSQL_SYSVAR_STR(log_file, mrn_log_file_path,
                        PLUGIN_VAR_RQCMDARG | PLUGIN_VAR_MEMALLOC,
                        "log file for " MRN_PLUGIN_NAME_STRING,
                        NULL,
                        mrn_log_file_update,
                        MRN_LOG_FILE_PATH);

static void mrn_default_parser_update(THD *thd, struct st_mysql_sys_var *var,
                                      void *var_ptr, const void *save)
{
  MRN_DBUG_ENTER_FUNCTION();
  const char *new_value = *((const char **)save);
  char **old_value_ptr = (char **)var_ptr;
  grn_ctx ctx;

  grn_ctx_init(&ctx, 0);
  mrn_change_encoding(&ctx, system_charset_info);
  if (strcmp(*old_value_ptr, new_value) == 0) {
    GRN_LOG(&ctx, GRN_LOG_NOTICE,
            "default parser isn't changed "
            "because the requested default parser isn't different: <%s>",
            new_value);
  } else {
    GRN_LOG(&ctx, GRN_LOG_NOTICE,
            "default fulltext parser is changed: <%s> -> <%s>",
            *old_value_ptr, new_value);

#ifdef MRN_NEED_FREE_STRING_MEMALLOC_PLUGIN_VAR
    my_free(*old_value_ptr, MYF(0));
    *old_value_ptr = my_strdup(new_value, MYF(MY_WME));
#else
    *old_value_ptr = (char *)new_value;
#endif
  }
  grn_ctx_fin(&ctx);

  DBUG_VOID_RETURN;
}

static MYSQL_SYSVAR_STR(default_parser, mrn_default_parser,
                        PLUGIN_VAR_RQCMDARG | PLUGIN_VAR_MEMALLOC,
                        "default fulltext parser",
                        NULL,
                        mrn_default_parser_update,
                        MRN_PARSER_DEFAULT);

static MYSQL_THDVAR_BOOL(
  dry_write, /* name */
  PLUGIN_VAR_OPCMDARG, /* options */
  "If dry_write is true, any write operations are ignored.", /* comment */
  NULL, /* check */
  NULL, /* update */
  false /* default */
);

static MYSQL_THDVAR_BOOL(
  enable_optimization, /* name */
  PLUGIN_VAR_OPCMDARG, /* options */
  "If enable_optimization is true, some optimizations will be applied.", /* comment */
  NULL, /* check */
  NULL, /* update */
  true /* default */
);

static MYSQL_THDVAR_LONGLONG(match_escalation_threshold,
                             PLUGIN_VAR_RQCMDARG,
                             "The threshold to determin whether search method is escalated",
                             NULL,
                             NULL,
                             grn_get_default_match_escalation_threshold(),
                             -1,
                             LONGLONG_MAX,
                             0);

static MYSQL_SYSVAR_STR(database_path_prefix, mrn_database_path_prefix,
                        PLUGIN_VAR_RQCMDARG | PLUGIN_VAR_MEMALLOC,
                        "The database path prefix",
                        NULL,
                        NULL,
                        NULL);

static MYSQL_SYSVAR_STR(default_wrapper_engine, mrn_default_wrapper_engine,
                        PLUGIN_VAR_RQCMDARG | PLUGIN_VAR_READONLY,
                        "The default engine for wrapper mode",
                        NULL,
                        NULL,
                        NULL);

static MYSQL_SYSVAR_STR(libgroonga_version, mrn_libgroonga_version,
                        PLUGIN_VAR_NOCMDOPT | PLUGIN_VAR_READONLY,
                        "The version of libgroonga",
                        NULL,
                        NULL,
                        grn_get_version());

static MYSQL_SYSVAR_STR(version, mrn_version,
                        PLUGIN_VAR_NOCMDOPT | PLUGIN_VAR_READONLY,
                        "The version of mroonga",
                        NULL,
                        NULL,
                        MRN_VERSION);

static struct st_mysql_sys_var *mrn_system_variables[] =
{
  MYSQL_SYSVAR(log_level),
  MYSQL_SYSVAR(log_file),
  MYSQL_SYSVAR(default_parser),
  MYSQL_SYSVAR(dry_write),
  MYSQL_SYSVAR(enable_optimization),
  MYSQL_SYSVAR(match_escalation_threshold),
  MYSQL_SYSVAR(database_path_prefix),
  MYSQL_SYSVAR(default_wrapper_engine),
  MYSQL_SYSVAR(libgroonga_version),
  MYSQL_SYSVAR(version),
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

longlong last_insert_grn_id(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error)
{
  THD *thd = current_thd;
  st_mrn_slot_data *slot_data = mrn_get_slot_data(thd, false);
  if (slot_data == NULL) {
    return 0;
  }
  longlong last_insert_record_id = slot_data->last_insert_record_id;
  return last_insert_record_id;
}

void last_insert_grn_id_deinit(UDF_INIT *initid)
{
}

struct st_mrn_snip_info
{
  grn_ctx ctx;
  grn_snip *snippet;
  String result_str;
};

my_bool mrn_snippet_prepare(st_mrn_snip_info *snip_info, UDF_ARGS *args,
                            char *message, grn_snip **snippet)
{
  unsigned int i;
  CHARSET_INFO *cs;
  grn_ctx *ctx = &snip_info->ctx;
  long long snip_max_len;
  long long snip_max_num;
  long long skip_leading_spaces;
  long long html_escape;
  int flags = GRN_SNIP_COPY_TAG;
  grn_snip_mapping *mapping = NULL;
  grn_rc rc;
  String *result_str = &snip_info->result_str;

  *snippet = NULL;
  snip_max_len = *((long long *) args->args[1]);
  snip_max_num = *((long long *) args->args[2]);

  if (args->arg_type[3] == STRING_RESULT) {
    if (!(cs = get_charset_by_name(args->args[3], MYF(0)))) {
      my_error(ER_UNKNOWN_COLLATION, MYF(0), args->args[3]);
      goto error;
    }
  } else {
    if (!(cs = get_charset(*((long long *) args->args[3]), MYF(0)))) {
      my_error(ER_UNKNOWN_COLLATION, MYF(0), "");
      goto error;
    }
  }
  if (mrn_change_encoding(ctx, cs)) {
    goto error;
  }

  if (!(cs->state & (MY_CS_BINSORT | MY_CS_CSSORT))) {
    flags |= GRN_SNIP_NORMALIZE;
  }

  skip_leading_spaces = *((long long *) args->args[4]);
  if (skip_leading_spaces) {
    flags |= GRN_SNIP_SKIP_LEADING_SPACES;
  }

  html_escape = *((long long *) args->args[5]);
  if (html_escape) {
    mapping = (grn_snip_mapping *) -1;
  }

  *snippet = grn_snip_open(ctx, flags, snip_max_len, snip_max_num,
                           "", 0, "", 0, mapping);
  if (ctx->rc) {
    my_printf_error(ER_MRN_ERROR_FROM_GROONGA_NUM,
                    ER_MRN_ERROR_FROM_GROONGA_STR, MYF(0), ctx->errbuf);
    goto error;
  }

  for (i = 8; i < args->arg_count; i += 3) {
    rc = grn_snip_add_cond(ctx, *snippet,
                           args->args[i], args->lengths[i],
                           args->args[i + 1], args->lengths[i + 1],
                           args->args[i + 2], args->lengths[i + 2]);
    if (rc) {
      my_printf_error(ER_MRN_ERROR_FROM_GROONGA_NUM,
                      ER_MRN_ERROR_FROM_GROONGA_STR, MYF(0), ctx->errbuf);
      goto error;
    }
  }

  result_str->set_charset(cs);
  return FALSE;

error:
  if (*snippet) {
    grn_snip_close(ctx, *snippet);
  }
  if (message) {
    strcpy(message, MRN_GET_ERROR_MESSAGE);
  }
  return TRUE;
}

my_bool mroonga_snippet_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
  uint i;
  st_mrn_snip_info *snip_info = NULL;
  bool can_open_snippet = TRUE;
  initid->ptr = NULL;
  if (args->arg_count < 11 || (args->arg_count - 11) % 3)
  {
    sprintf(message, "Incorrect number of arguments for mroonga_snippet(): %u",
            args->arg_count);
    goto error;
  }
  if (args->arg_type[0] != STRING_RESULT) {
    strcpy(message, "mroonga_snippet() requires string for 1st argument");
    goto error;
  }
  if (args->arg_type[1] != INT_RESULT) {
    strcpy(message, "mroonga_snippet() requires int for 2nd argument");
    goto error;
  }
  if (args->arg_type[2] != INT_RESULT) {
    strcpy(message, "mroonga_snippet() requires int for 3rd argument");
    goto error;
  }
  if (
    args->arg_type[3] != STRING_RESULT &&
    args->arg_type[3] != INT_RESULT
  ) {
    strcpy(message,
      "mroonga_snippet() requires string or int for 4th argument");
    goto error;
  }
  if (args->arg_type[4] != INT_RESULT) {
    strcpy(message, "mroonga_snippet() requires int for 5th argument");
    goto error;
  }
  if (args->arg_type[5] != INT_RESULT) {
    strcpy(message, "mroonga_snippet() requires int for 6th argument");
    goto error;
  }
  for (i = 6; i < args->arg_count; i++) {
    if (args->arg_type[i] != STRING_RESULT) {
      sprintf(message, "mroonga_snippet() requires string for %uth argument",
              i);
      goto error;
    }
  }
  initid->maybe_null = 1;
  initid->const_item = 1;

  if (!(snip_info = (st_mrn_snip_info *) my_malloc(sizeof(st_mrn_snip_info),
                                                   MYF(MY_WME | MY_ZEROFILL))))
  {
    strcpy(message, "mroonga_snippet() out of memory");
    goto error;
  }
  grn_ctx_init(&snip_info->ctx, 0);

  for (i = 1; i < args->arg_count; i++) {
    if (!args->args[i]) {
      can_open_snippet = FALSE;
      break;
    }
  }
  if (can_open_snippet) {
    if (mrn_snippet_prepare(snip_info, args, message, &snip_info->snippet)) {
      goto error;
    }
  }
  initid->ptr = (char *) snip_info;

  return FALSE;

error:
  if (snip_info) {
    grn_ctx_fin(&snip_info->ctx);
    my_free(snip_info, MYF(0));
  }
  return TRUE;
}

char *mroonga_snippet(UDF_INIT *initid, UDF_ARGS *args, char *result,
                      unsigned long *length, char *is_null, char *error)
{
  st_mrn_snip_info *snip_info = (st_mrn_snip_info *) initid->ptr;
  grn_ctx *ctx = &snip_info->ctx;
  String *result_str = &snip_info->result_str;
  char *target;
  unsigned int target_length;
  grn_snip *snippet = NULL;
  grn_rc rc;
  unsigned int i, n_results, max_tagged_length, result_length;

  if (!args->args[0]) {
    *is_null = 1;
    return NULL;
  }
  *is_null = 0;
  target = args->args[0];
  target_length = args->lengths[0];

  if (!snip_info->snippet) {
    for (i = 1; i < args->arg_count; i++) {
      if (!args->args[i]) {
        my_printf_error(ER_MRN_INVALID_NULL_VALUE_NUM,
                        ER_MRN_INVALID_NULL_VALUE_STR, MYF(0),
                        "mroonga_snippet() arguments");
        goto error;
      }
    }

    if (mrn_snippet_prepare(snip_info, args, NULL, &snippet)) {
      goto error;
    }
  } else {
    snippet = snip_info->snippet;
  }

  rc = grn_snip_exec(ctx, snippet, target, target_length,
                     &n_results, &max_tagged_length);
  if (rc) {
    my_printf_error(ER_MRN_ERROR_FROM_GROONGA_NUM,
                    ER_MRN_ERROR_FROM_GROONGA_STR, MYF(0), ctx->errbuf);
    goto error;
  }

  result_str->length(0);
  if (result_str->reserve((args->lengths[6] + args->lengths[7] +
                          max_tagged_length) * n_results)) {
    my_error(ER_OUT_OF_RESOURCES, MYF(0), HA_ERR_OUT_OF_MEM);
    goto error;
  }
  for (i = 0; i < n_results; i++) {
    result_str->q_append(args->args[6], args->lengths[6]);
    rc = grn_snip_get_result(ctx, snippet, i,
                             (char *) result_str->ptr() + result_str->length(),
                             &result_length);
    if (rc) {
      my_printf_error(ER_MRN_ERROR_FROM_GROONGA_NUM,
                      ER_MRN_ERROR_FROM_GROONGA_STR, MYF(0), ctx->errbuf);
      goto error;
    }
    result_str->length(result_str->length() + result_length);
    result_str->q_append(args->args[7], args->lengths[7]);
  }

  if (!snip_info->snippet) {
    rc = grn_snip_close(ctx, snippet);
    if (rc) {
      my_printf_error(ER_MRN_ERROR_FROM_GROONGA_NUM,
                      ER_MRN_ERROR_FROM_GROONGA_STR, MYF(0), ctx->errbuf);
      goto error;
    }
  }

  *length = result_str->length();
  return (char *) result_str->ptr();

error:
  *error = 1;
  return NULL;
}

void mroonga_snippet_deinit(UDF_INIT *initid)
{
  st_mrn_snip_info *snip_info = (st_mrn_snip_info *) initid->ptr;
  if (snip_info) {
    if (snip_info->snippet) {
      grn_snip_close(&snip_info->ctx, snip_info->snippet);
    }
    snip_info->result_str.free();
    grn_ctx_fin(&snip_info->ctx);
    my_free(snip_info, MYF(0));
  }
}

/* mroonga information schema */
static struct st_mysql_information_schema i_s_info =
{
  MYSQL_INFORMATION_SCHEMA_INTERFACE_VERSION
};

static ST_FIELD_INFO i_s_mrn_stats_fields_info[] =
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

static int i_s_mrn_stats_deinit(void* p)
{
  MRN_DBUG_ENTER_FUNCTION();
  DBUG_RETURN(0);
}

static int i_s_mrn_stats_fill(
  THD* thd, TABLE_LIST* tables, Item* cond)
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

static int i_s_mrn_stats_init(void* p)
{
  MRN_DBUG_ENTER_FUNCTION();
  ST_SCHEMA_TABLE* schema = (ST_SCHEMA_TABLE*) p;
  schema->fields_info = i_s_mrn_stats_fields_info;
  schema->fill_table = i_s_mrn_stats_fill;
  DBUG_RETURN(0);
}

struct st_mysql_plugin i_s_mrn_stats =
{
  MYSQL_INFORMATION_SCHEMA_PLUGIN,
  &i_s_info,
  MRN_PLUGIN_NAME_STRING "_stats",
  mrn_plugin_author,
  "Statistics for " MRN_PLUGIN_NAME_STRING,
  PLUGIN_LICENSE_GPL,
  i_s_mrn_stats_init,
  i_s_mrn_stats_deinit,
  MRN_VERSION_IN_HEX,
  NULL,
  NULL,
  MRN_PLUGIN_LAST_VALUES
};
/* End of mroonga information schema implementations */

static handler *mrn_handler_create(handlerton *hton, TABLE_SHARE *share, MEM_ROOT *root)
{
  MRN_DBUG_ENTER_FUNCTION();
  handler *new_handler = new (root) ha_mroonga(hton, share);
  DBUG_RETURN(new_handler);
}

static void mrn_drop_db(const char *path)
{
  MRN_DBUG_ENTER_FUNCTION();
  mrn::PathMapper mapper(path, mrn_database_path_prefix);
  pthread_mutex_lock(&mrn_db_mutex);
  grn_obj *db = NULL;
  if (!mrn_hash_get(&mrn_ctx, mrn_hash, mapper.db_name(), &db)) {
    struct stat dummy;
    if (stat(mapper.db_path(), &dummy) == 0) {
      db = grn_db_open(&mrn_ctx, mapper.db_path());
    }
  }
  if (db) {
    if (grn_obj_remove(&mrn_ctx, db)) {
      GRN_LOG(&mrn_ctx, GRN_LOG_ERROR,
              "cannot drop database (%s)", mapper.db_path());
    }
  }
  mrn_hash_remove(&mrn_ctx, mrn_hash, mapper.db_name());
  pthread_mutex_unlock(&mrn_db_mutex);
  DBUG_VOID_RETURN;
}

static void mrn_drop_database(handlerton *hton, char *path)
{
  MRN_DBUG_ENTER_FUNCTION();
  mrn_drop_db(path);
  DBUG_VOID_RETURN;
}

static int mrn_close_connection(handlerton *hton, THD *thd)
{
  MRN_DBUG_ENTER_FUNCTION();
  void *p = *thd_ha_data(thd, mrn_hton_ptr);
  if (p) {
    mrn_clear_alter_share(thd);
    free(p);
    *thd_ha_data(thd, mrn_hton_ptr) = (void *) NULL;
    pthread_mutex_lock(&mrn_allocated_thds_mutex);
    my_hash_delete(&mrn_allocated_thds, (uchar*) thd);
    pthread_mutex_unlock(&mrn_allocated_thds_mutex);
  }
  DBUG_RETURN(0);
}

static bool mrn_flush_logs(handlerton *hton)
{
  MRN_DBUG_ENTER_FUNCTION();
  bool result = 0;
  if (mrn_log_file_opened) {
    pthread_mutex_lock(&mrn_log_mutex);
    fclose(mrn_log_file);
    mrn_log_file = fopen(mrn_log_file_path, "a");
    pthread_mutex_unlock(&mrn_log_mutex);
  }
  DBUG_RETURN(result);
}

static grn_builtin_type mrn_grn_type_from_field(grn_ctx *ctx, Field *field,
                                                bool for_index_key)
{
  grn_builtin_type type = GRN_DB_VOID;
  enum_field_types mysql_field_type = field->real_type();
  switch (mysql_field_type) {
  case MYSQL_TYPE_DECIMAL:      // DECIMAL; <= 65bytes
    type = GRN_DB_SHORT_TEXT;   // 4Kbytes
    break;
  case MYSQL_TYPE_TINY:         // TINYINT; 1byte
    if (static_cast<Field_num *>(field)->unsigned_flag) {
      type = GRN_DB_UINT8;      // 1byte
    } else {
      type = GRN_DB_INT8;       // 1byte
    }
    break;
  case MYSQL_TYPE_SHORT:        // SMALLINT; 2bytes
    if (static_cast<Field_num *>(field)->unsigned_flag) {
      type = GRN_DB_UINT16;     // 2bytes
    } else {
      type = GRN_DB_INT16;      // 2bytes
    }
    break;
  case MYSQL_TYPE_LONG:         // INT; 4bytes
    if (static_cast<Field_num *>(field)->unsigned_flag) {
      type = GRN_DB_UINT32;     // 4bytes
    } else {
      type = GRN_DB_INT32;      // 4bytes
    }
    break;
  case MYSQL_TYPE_FLOAT:        // FLOAT; 4 or 8bytes
  case MYSQL_TYPE_DOUBLE:       // DOUBLE; 8bytes
    type = GRN_DB_FLOAT;        // 8bytes
    break;
  case MYSQL_TYPE_NULL:         // NULL; 1byte
    type = GRN_DB_INT8;         // 1byte
    break;
  case MYSQL_TYPE_TIMESTAMP:    // TIMESTAMP; 4bytes
    type = GRN_DB_TIME;         // 8bytes
    break;
  case MYSQL_TYPE_LONGLONG:     // BIGINT; 8bytes
    if (static_cast<Field_num *>(field)->unsigned_flag) {
      type = GRN_DB_UINT64;     // 8bytes
    } else {
      type = GRN_DB_INT64;      // 8bytes
    }
    break;
  case MYSQL_TYPE_INT24:        // MEDIUMINT; 3bytes
    if (static_cast<Field_num *>(field)->unsigned_flag) {
      type = GRN_DB_UINT32;     // 4bytes
    } else {
      type = GRN_DB_INT32;      // 4bytes
    }
    break;
  case MYSQL_TYPE_DATE:         // DATE; 4bytes
  case MYSQL_TYPE_TIME:         // TIME; 3bytes
  case MYSQL_TYPE_DATETIME:     // DATETIME; 8bytes
  case MYSQL_TYPE_YEAR:         // YEAR; 1byte
  case MYSQL_TYPE_NEWDATE:      // DATE; 3bytes
    type = GRN_DB_TIME;         // 8bytes
    break;
  case MYSQL_TYPE_VARCHAR:      // VARCHAR; <= 64KB * 4 + 2bytes
    if (for_index_key) {
      type = GRN_DB_SHORT_TEXT; // 4Kbytes
    } else {
      if (field->field_length <= MRN_SHORT_TEXT_SIZE) {
        type = GRN_DB_SHORT_TEXT; //  4Kbytes
      } else if (field->field_length <= MRN_TEXT_SIZE) {
        type = GRN_DB_TEXT;       // 64Kbytes
      } else {
        type = GRN_DB_LONG_TEXT;  //  2Gbytes
      }
    }
    break;
  case MYSQL_TYPE_BIT:          // BIT; <= 8bytes
    type = GRN_DB_INT64;        // 8bytes
    break;
#ifdef MRN_HAVE_MYSQL_TYPE_TIMESTAMP2
  case MYSQL_TYPE_TIMESTAMP2:   // TIMESTAMP; 4bytes
    type = GRN_DB_TIME;         // 8bytes
    break;
#endif
#ifdef MRN_HAVE_MYSQL_TYPE_DATETIME2
  case MYSQL_TYPE_DATETIME2:    // DATETIME; 8bytes
    type = GRN_DB_TIME;         // 8bytes
    break;
#endif
#ifdef MRN_HAVE_MYSQL_TYPE_TIME2
  case MYSQL_TYPE_TIME2:        // TIME(FSP); 3 + (FSP + 1) / 2 bytes
                                // 0 <= FSP <= 6; 3-6bytes
    type = GRN_DB_TIME;         // 8bytes
    break;
#endif
  case MYSQL_TYPE_NEWDECIMAL:   // DECIMAL; <= 9bytes
    type = GRN_DB_SHORT_TEXT;   // 4Kbytes
    break;
  case MYSQL_TYPE_ENUM:         // ENUM; <= 2bytes
    if (field->pack_length() == 1) {
      type = GRN_DB_UINT8;      // 1bytes
    } else {
      type = GRN_DB_UINT16;     // 2bytes
    }
    break;
  case MYSQL_TYPE_SET:          // SET; <= 8bytes
    switch (field->pack_length()) {
    case 1:
      type = GRN_DB_UINT8;      // 1byte
      break;
    case 2:
      type = GRN_DB_UINT16;     // 2bytes
      break;
    case 3:
    case 4:
      type = GRN_DB_UINT32;     // 3bytes
      break;
    case 8:
    default:
      type = GRN_DB_UINT64;     // 8bytes
      break;
    }
    break;
  case MYSQL_TYPE_TINY_BLOB:    // TINYBLOB; <= 256bytes + 1byte
    type = GRN_DB_SHORT_TEXT;   // 4Kbytes
    break;
  case MYSQL_TYPE_MEDIUM_BLOB:  // MEDIUMBLOB; <= 16Mbytes + 3bytes
    if (for_index_key) {
      type = GRN_DB_SHORT_TEXT; // 4Kbytes
    } else {
      type = GRN_DB_LONG_TEXT;  // 2Gbytes
    }
    break;
  case MYSQL_TYPE_LONG_BLOB:    // LONGBLOB; <= 4Gbytes + 4bytes
    if (for_index_key) {
      type = GRN_DB_SHORT_TEXT; // 4Kbytes
    } else {
      type = GRN_DB_LONG_TEXT;  // 2Gbytes
    }
    break;
  case MYSQL_TYPE_BLOB:         // BLOB; <= 64Kbytes + 2bytes
    if (for_index_key) {
      type = GRN_DB_SHORT_TEXT; // 4Kbytes
    } else {
      type = GRN_DB_LONG_TEXT;  // 2Gbytes
    }
    break;
  case MYSQL_TYPE_VAR_STRING:   // VARCHAR; <= 255byte * 4 + 1bytes
    if (for_index_key) {
      type = GRN_DB_SHORT_TEXT; // 4Kbytes
    } else {
      if (field->field_length <= MRN_SHORT_TEXT_SIZE) {
        type = GRN_DB_SHORT_TEXT; //  4Kbytes
      } else if (field->field_length <= MRN_TEXT_SIZE) {
        type = GRN_DB_TEXT;       // 64Kbytes
      } else {
        type = GRN_DB_LONG_TEXT;  //  2Gbytes
      }
    }
    break;
  case MYSQL_TYPE_STRING:       // CHAR; < 1Kbytes =~ (255 * 4)bytes
                                //              4 is the maximum size of a character
    type = GRN_DB_SHORT_TEXT;   // 4Kbytes
    break;
  case MYSQL_TYPE_GEOMETRY:     // case-by-case
    type = GRN_DB_WGS84_GEO_POINT; // 8bytes
    break;
  }
  return type;
}

#ifdef HAVE_SPATIAL
static int mrn_set_geometry(grn_ctx *ctx, grn_obj *buf,
                            const char *wkb, uint wkb_size)
{
  int error = 0;
  Geometry_buffer buffer;
  Geometry *geometry;

  geometry = Geometry::construct(&buffer, wkb, wkb_size);
  switch (geometry->get_class_info()->m_type_id) {
  case Geometry::wkb_point:
    {
      Gis_point *point = (Gis_point *)geometry;
      double latitude = 0.0, longitude = 0.0;
      point->get_xy(&longitude, &latitude);
      grn_obj_reinit(ctx, buf, GRN_DB_WGS84_GEO_POINT, 0);
      GRN_GEO_POINT_SET(ctx, buf,
                        GRN_GEO_DEGREE2MSEC(latitude),
                        GRN_GEO_DEGREE2MSEC(longitude));
      break;
    }
  default:
    my_printf_error(ER_MRN_GEOMETRY_NOT_SUPPORT_NUM,
      ER_MRN_GEOMETRY_NOT_SUPPORT_STR, MYF(0));
    error = ER_MRN_GEOMETRY_NOT_SUPPORT_NUM;
    break;
  }
  delete geometry;

  return error;
}
#endif

#ifdef WORDS_BIGENDIAN
#define mrn_byte_order_host_to_network(buf, key, size)  \
{                                                       \
  uint32 size_ = (uint32)(size);                        \
  uint8 *buf_ = (uint8 *)(buf);                         \
  uint8 *key_ = (uint8 *)(key);                         \
  while (size_--) { *buf_++ = *key_++; }                \
}
#else /* WORDS_BIGENDIAN */
#define mrn_byte_order_host_to_network(buf, key, size)  \
{                                                       \
  uint32 size_ = (uint32)(size);                        \
  uint8 *buf_ = (uint8 *)(buf);                         \
  uint8 *key_ = (uint8 *)(key) + size_;                 \
  while (size_--) { *buf_++ = *(--key_); }              \
}
#endif /* WORDS_BIGENDIAN */

static long long int mrn_tm_to_grn_time(struct tm *time, int usec)
{
  MRN_DBUG_ENTER_FUNCTION();
  long long int sec = mktime(time) + mrn_utc_diff_in_seconds;
  long long int grn_time = GRN_TIME_PACK(sec, usec);
  DBUG_RETURN(grn_time);
}

static long long int mrn_mysql_time_to_grn_time(MYSQL_TIME *mysql_time)
{
  MRN_DBUG_ENTER_FUNCTION();
  int usec = mysql_time->second_part;
  long long int grn_time = 0;
  switch (mysql_time->time_type) {
  case MYSQL_TIMESTAMP_DATE:
    {
      struct tm date;
      memset(&date, 0, sizeof(struct tm));
      date.tm_year = mysql_time->year - 1900;
      date.tm_mon = mysql_time->month > 0 ? mysql_time->month - 1 : 0;
      date.tm_mday = mysql_time->day > 0 ? mysql_time->day : 1;
      grn_time = mrn_tm_to_grn_time(&date, usec);
    }
    break;
  case MYSQL_TIMESTAMP_DATETIME:
    {
      struct tm datetime;
      memset(&datetime, 0, sizeof(struct tm));
      datetime.tm_year = mysql_time->year - 1900;
      datetime.tm_mon = mysql_time->month > 0 ? mysql_time->month - 1 : 0;
      datetime.tm_mday = mysql_time->day > 0 ? mysql_time->day : 1;
      datetime.tm_hour = mysql_time->hour;
      datetime.tm_min = mysql_time->minute;
      datetime.tm_sec = mysql_time->second;
      grn_time = mrn_tm_to_grn_time(&datetime, usec);
    }
    break;
  case MYSQL_TIMESTAMP_TIME:
    {
      int sec =
        mysql_time->hour * 60 * 60 +
        mysql_time->minute * 60 +
        mysql_time->second;
      grn_time = GRN_TIME_PACK(sec, usec);
      if (mysql_time->neg) {
        grn_time = -grn_time;
      }
    }
    break;
  default:
    grn_time = 0;
    break;
  }
  DBUG_RETURN(grn_time);
}

static void mrn_grn_time_to_mysql_time(long long int grn_time,
                                       MYSQL_TIME *mysql_time)
{
  MRN_DBUG_ENTER_FUNCTION();
  long long int sec;
  int usec;
  GRN_TIME_UNPACK(grn_time, sec, usec);
  switch (mysql_time->time_type) {
  case MYSQL_TIMESTAMP_DATE:
    {
      struct tm date;
      time_t sec_t = sec;
      gmtime_r(&sec_t, &date);
      mysql_time->year = date.tm_year + 1900;
      mysql_time->month = date.tm_mon + 1;
      mysql_time->day = date.tm_mday;
    }
    break;
  case MYSQL_TIMESTAMP_DATETIME:
    {
      struct tm date;
      time_t sec_t = sec;
      gmtime_r(&sec_t, &date);
      mysql_time->year = date.tm_year + 1900;
      mysql_time->month = date.tm_mon + 1;
      mysql_time->day = date.tm_mday;
      mysql_time->hour = date.tm_hour;
      mysql_time->minute = date.tm_min;
      mysql_time->second = date.tm_sec;
      mysql_time->second_part = usec;
    }
    break;
  case MYSQL_TIMESTAMP_TIME:
    if (sec < 0) {
      mysql_time->neg = true;
      sec = -sec;
    }
    mysql_time->hour = sec / 60 / 60;
    mysql_time->minute = sec / 60 % 60;
    mysql_time->second = sec % 60;
    mysql_time->second_part = usec;
    break;
  default:
    break;
  }
  DBUG_VOID_RETURN;
}

static uint mrn_alter_table_flags(uint flags) {
  uint ret_flags = 0;
  ret_flags |=
    HA_INPLACE_ADD_INDEX_NO_READ_WRITE |
    HA_INPLACE_DROP_INDEX_NO_READ_WRITE |
    HA_INPLACE_ADD_UNIQUE_INDEX_NO_READ_WRITE |
    HA_INPLACE_DROP_UNIQUE_INDEX_NO_READ_WRITE |
    HA_INPLACE_ADD_PK_INDEX_NO_READ_WRITE |
    HA_INPLACE_DROP_PK_INDEX_NO_READ_WRITE |
    HA_INPLACE_ADD_INDEX_NO_WRITE |
    HA_INPLACE_DROP_INDEX_NO_WRITE |
    HA_INPLACE_ADD_UNIQUE_INDEX_NO_WRITE |
    HA_INPLACE_DROP_UNIQUE_INDEX_NO_WRITE |
    HA_INPLACE_ADD_PK_INDEX_NO_WRITE |
    HA_INPLACE_DROP_PK_INDEX_NO_WRITE;
  return ret_flags;
}

static void mrn_init_time(void)
{
  struct tm now_tm;
  time_t now;
  time(&now);
  gmtime_r(&now, &now_tm);
  mrn_utc_diff_in_seconds = now - mktime(&now_tm);
}

static int mrn_init(void *p)
{
  // init handlerton
  grn_ctx *ctx = NULL;
  handlerton *hton;
  hton = (handlerton *)p;
  hton->state = SHOW_OPTION_YES;
  hton->create = mrn_handler_create;
  hton->flags = HTON_NO_PARTITION;
  hton->drop_database = mrn_drop_database;
  hton->close_connection = mrn_close_connection;
  hton->flush_logs = mrn_flush_logs;
  hton->alter_table_flags = mrn_alter_table_flags;
  mrn_hton_ptr = hton;

  // init groonga
  if (grn_init() != GRN_SUCCESS) {
    goto err_grn_init;
  }

  mrn_init_encoding_map();

  grn_ctx_init(&mrn_ctx, 0);
  ctx = &mrn_ctx;
  if (mrn_change_encoding(ctx, system_charset_info))
    goto err_mrn_change_encoding;

  if (pthread_mutex_init(&mrn_log_mutex, NULL) != 0) {
    goto err_log_mutex_init;
  }

  grn_logger_info_set(ctx, &mrn_logger_info);
  if (!(mrn_log_file = fopen(mrn_log_file_path, "a"))) {
    goto err_log_file_open;
  }
  mrn_log_file_opened = true;
  GRN_LOG(ctx, GRN_LOG_NOTICE, "%s started.", MRN_PACKAGE_STRING);
  GRN_LOG(ctx, GRN_LOG_NOTICE, "log level is '%s'",
          mrn_log_level_type_names[mrn_log_level]);

  // init meta-info database
  if (!(mrn_db = grn_db_create(ctx, NULL, NULL))) {
    GRN_LOG(ctx, GRN_LOG_ERROR, "cannot create system database, exiting");
    goto err_db_create;
  }
  grn_ctx_use(ctx, mrn_db);

  // init hash
  if (!(mrn_hash = grn_hash_create(ctx, NULL,
                                   MRN_MAX_KEY_SIZE, sizeof(grn_obj *),
                                   GRN_OBJ_KEY_VAR_SIZE))) {
    GRN_LOG(ctx, GRN_LOG_ERROR, "cannot init hash, exiting");
    goto err_hash_create;
  }

  // init lock
  if ((pthread_mutex_init(&mrn_db_mutex, NULL) != 0)) {
    goto err_db_mutex_init;
  }
  if ((pthread_mutex_init(&mrn_allocated_thds_mutex, NULL) != 0)) {
    goto err_allocated_thds_mutex_init;
  }
  if (my_hash_init(&mrn_allocated_thds, system_charset_info, 32, 0, 0,
                   mrn_allocated_thds_get_key, 0, 0)) {
    goto error_allocated_thds_hash_init;
  }
  if ((pthread_mutex_init(&mrn_open_tables_mutex, NULL) != 0)) {
    goto err_allocated_open_tables_mutex_init;
  }
  if (my_hash_init(&mrn_open_tables, system_charset_info, 32, 0, 0,
                   mrn_open_tables_get_key, 0, 0)) {
    goto error_allocated_open_tables_hash_init;
  }

  mrn_init_time();

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
  grn_hash_close(ctx, mrn_hash);
err_hash_create:
  grn_obj_unlink(ctx, mrn_db);
err_db_create:
  if (mrn_log_file_opened) {
    fclose(mrn_log_file);
    mrn_log_file_opened = false;
  }
err_log_file_open:
  pthread_mutex_destroy(&mrn_log_mutex);
err_log_mutex_init:
err_mrn_change_encoding:
  grn_ctx_fin(ctx);
  grn_fin();
err_grn_init:
  return -1;
}

static int mrn_deinit(void *p)
{
  THD *thd = current_thd, *tmp_thd;
  grn_ctx *ctx = &mrn_ctx;
  void *value;

  GRN_LOG(ctx, GRN_LOG_NOTICE, "%s deinit", MRN_PACKAGE_STRING);

  if (thd && thd_sql_command(thd) == SQLCOM_UNINSTALL_PLUGIN) {
    pthread_mutex_lock(&mrn_allocated_thds_mutex);
    while ((tmp_thd = (THD *) my_hash_element(&mrn_allocated_thds, 0)))
    {
      mrn_clear_alter_share(tmp_thd);
      void *slot_ptr = mrn_get_slot_data(tmp_thd, false);
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
  pthread_mutex_destroy(&mrn_db_mutex);
  GRN_HASH_EACH(ctx, mrn_hash, id, NULL, 0, &value, {
    grn_obj *db;
    memcpy(&db, value, sizeof(grn_obj *));
    grn_obj_unlink(ctx, db);
  });
  grn_hash_close(ctx, mrn_hash);
  grn_obj_unlink(ctx, mrn_db);

  grn_ctx_fin(ctx);
  grn_fin();

  if (mrn_log_file_opened) {
    fclose(mrn_log_file);
    mrn_log_file_opened = false;
  }
  pthread_mutex_destroy(&mrn_log_mutex);

  return 0;
}

mrn_declare_plugin(MRN_PLUGIN_NAME)
{
  MYSQL_STORAGE_ENGINE_PLUGIN,
  &storage_engine_structure,
  MRN_PLUGIN_NAME_STRING,
  mrn_plugin_author,
  "CJK-ready fulltext search, column store",
  PLUGIN_LICENSE_GPL,
  mrn_init,
  mrn_deinit,
  MRN_VERSION_IN_HEX,
  mrn_status_variables,
  mrn_system_variables,
  MRN_PLUGIN_LAST_VALUES
},
i_s_mrn_stats
mrn_declare_plugin_end;

static void mrn_generic_ft_close_search(FT_INFO *handler)
{
  MRN_DBUG_ENTER_FUNCTION();
  st_mrn_ft_info *info = (st_mrn_ft_info *)handler;
  grn_obj_unlink(info->ctx, info->result);
  grn_obj_unlink(info->ctx, info->score_column);
  grn_obj_unlink(info->ctx, &(info->key));
  grn_obj_unlink(info->ctx, &(info->score));
  grn_ctx_fin(info->ctx);
  delete info;
  DBUG_VOID_RETURN;
}

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
  float score = 0.0;
  grn_id record_id;

  mrn_change_encoding(info->ctx, NULL);
  key_copy((uchar *)(GRN_TEXT_VALUE(&(info->key))), record,
           info->primary_key_info, info->primary_key_info->key_length);
  record_id = grn_table_get(info->ctx,
                            info->table,
                            GRN_TEXT_VALUE(&(info->key)),
                            GRN_TEXT_LEN(&(info->key)));

  if (record_id != GRN_ID_NIL) {
    grn_id result_record_id;
    result_record_id = grn_table_get(info->ctx, info->result,
                                     &record_id, sizeof(grn_id));
    if (result_record_id != GRN_ID_NIL) {
      GRN_BULK_REWIND(&(info->score));
      grn_obj_get_value(info->ctx, info->score_column,
                        result_record_id, &(info->score));
      score = (float)GRN_INT32_VALUE(&(info->score));
    }
  }

  DBUG_PRINT("info",
             ("mroonga: record_id=%d score=%g", record_id, score));

  DBUG_RETURN(score);
}

static void mrn_wrapper_ft_close_search(FT_INFO *handler)
{
  MRN_DBUG_ENTER_FUNCTION();
  mrn_generic_ft_close_search(handler);
  DBUG_VOID_RETURN;
}

static float mrn_wrapper_ft_get_relevance(FT_INFO *handler)
{
  MRN_DBUG_ENTER_FUNCTION();
  st_mrn_ft_info *info = (st_mrn_ft_info *)handler;
  float score = 0.0;
  grn_id record_id;
  ha_mroonga *mroonga = info->mroonga;
  mrn_change_encoding(info->ctx, NULL);
  record_id = grn_table_get(info->ctx,
                            info->table,
                            GRN_TEXT_VALUE(&(mroonga->key_buffer)),
                            GRN_TEXT_LEN(&(mroonga->key_buffer)));

  if (record_id != GRN_ID_NIL) {
    grn_id result_record_id;
    result_record_id = grn_table_get(info->ctx, info->result,
                                     &record_id, sizeof(grn_id));
    if (result_record_id != GRN_ID_NIL) {
      GRN_BULK_REWIND(&(info->score));
      grn_obj_get_value(info->ctx, info->score_column,
                        result_record_id, &(info->score));
      score = (float)GRN_INT32_VALUE(&(info->score));
    }
  }

  DBUG_PRINT("info",
             ("mroonga: record_id=%d score=%g", record_id, score));

  DBUG_RETURN(score);
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

static int mrn_storage_ft_read_next(FT_INFO *handler, char *record)
{
  MRN_DBUG_ENTER_FUNCTION();
  DBUG_RETURN(HA_ERR_END_OF_FILE);
}

static float mrn_storage_ft_find_relevance(FT_INFO *handler, uchar *record,
                                           uint length)
{
  MRN_DBUG_ENTER_FUNCTION();
  st_mrn_ft_info *info = (st_mrn_ft_info *)handler;
  ha_mroonga *mroonga = info->mroonga;
  mrn_change_encoding(info->ctx, NULL);

  float score = 0.0;
  if (mroonga->record_id != GRN_ID_NIL) {
    grn_id result_record_id;
    result_record_id = grn_table_get(info->ctx, info->result,
                                     &(mroonga->record_id), sizeof(grn_id));
    if (result_record_id != GRN_ID_NIL) {
      GRN_BULK_REWIND(&(info->score));
      grn_obj_get_value(info->ctx, info->score_column,
                        result_record_id, &(info->score));
      score = (float)GRN_INT32_VALUE(&(info->score));
    }
  }
  DBUG_PRINT("info", ("mroonga: record_id=%d score=%g",
                      mroonga->record_id, score));

  DBUG_RETURN(score);
}

static void mrn_storage_ft_close_search(FT_INFO *handler)
{
  MRN_DBUG_ENTER_FUNCTION();
  mrn_generic_ft_close_search(handler);
  DBUG_VOID_RETURN;
}

static float mrn_storage_ft_get_relevance(FT_INFO *handler)
{
  MRN_DBUG_ENTER_FUNCTION();
  st_mrn_ft_info *info = (st_mrn_ft_info *)handler;
  ha_mroonga *mroonga = info->mroonga;
  mrn_change_encoding(info->ctx, NULL);

  float score = 0.0;
  if (mroonga->record_id != GRN_ID_NIL) {
    grn_id result_record_id;
    result_record_id = grn_table_get(info->ctx, info->result,
                                     &(mroonga->record_id), sizeof(grn_id));
    if (result_record_id != GRN_ID_NIL) {
      GRN_BULK_REWIND(&(info->score));
      grn_obj_get_value(info->ctx, info->score_column,
                        result_record_id, &(info->score));
      score = (float)GRN_INT32_VALUE(&(info->score));
    }
  }
  DBUG_PRINT("info",
             ("mroonga: record_id=%d score=%g", mroonga->record_id, score));

  DBUG_RETURN(score);
}

static void mrn_storage_ft_reinit_search(FT_INFO *handler)
{
  MRN_DBUG_ENTER_FUNCTION();
  DBUG_VOID_RETURN;
}

static _ft_vft mrn_storage_ft_vft = {
  mrn_storage_ft_read_next,
  mrn_storage_ft_find_relevance,
  mrn_storage_ft_close_search,
  mrn_storage_ft_get_relevance,
  mrn_storage_ft_reinit_search
};

static int mrn_no_such_key_ft_read_next(FT_INFO *handler, char *record)
{
  MRN_DBUG_ENTER_FUNCTION();
  DBUG_RETURN(HA_ERR_END_OF_FILE);
}

static float mrn_no_such_key_ft_find_relevance(FT_INFO *handler, uchar *record,
                                               uint length)
{
  MRN_DBUG_ENTER_FUNCTION();
  DBUG_RETURN(0.0);
}

static void mrn_no_such_key_ft_close_search(FT_INFO *handler)
{
  MRN_DBUG_ENTER_FUNCTION();
  st_mrn_ft_info *info = (st_mrn_ft_info *)handler;
  delete info;
  DBUG_VOID_RETURN;
}

static float mrn_no_such_key_ft_get_relevance(FT_INFO *handler)
{
  MRN_DBUG_ENTER_FUNCTION();
  DBUG_RETURN(0.0);
}

static void mrn_no_such_key_ft_reinit_search(FT_INFO *handler)
{
  MRN_DBUG_ENTER_FUNCTION();
  DBUG_VOID_RETURN;
}

static _ft_vft mrn_no_such_key_ft_vft = {
  mrn_no_such_key_ft_read_next,
  mrn_no_such_key_ft_find_relevance,
  mrn_no_such_key_ft_close_search,
  mrn_no_such_key_ft_get_relevance,
  mrn_no_such_key_ft_reinit_search
};

/* handler implementation */
ha_mroonga::ha_mroonga(handlerton *hton, TABLE_SHARE *share_arg)
  :handler(hton, share_arg),
   wrap_handler(NULL),
   is_clone(false),
   parent_for_clone(NULL),
   mem_root_for_clone(NULL),
   record_id(GRN_ID_NIL),
   key_id(NULL),
   del_key_id(NULL),

   share(NULL),
   wrap_key_info(NULL),
   base_key_info(NULL),

   analyzed_for_create(false),
   wrap_handler_for_create(NULL),
#ifdef MRN_HANDLER_HAVE_FINAL_ADD_INDEX
   hnd_add_index(NULL),
#endif
   mrn_lock_type(F_UNLCK),

   ctx_entity_(),
   ctx(&ctx_entity_),
   grn_table(NULL),
   grn_columns(NULL),
   grn_index_tables(NULL),
   grn_index_columns(NULL),
   grn_source_column_geo(NULL),
   cursor_geo(NULL),

   cursor(NULL),
   index_table_cursor(NULL),
   id_accessor(NULL),
   key_accessor(NULL),

   sorted_result(NULL),
   matched_record_keys(NULL),

   dup_key(0),

   count_skip(false),
   fast_order_limit(false),
   fast_order_limit_with_index(false),

   ignoring_duplicated_key(false),
   inserting_with_update(false),
   fulltext_searching(false),
   ignoring_no_key_columns(false),
   replacing_(false),
   written_by_row_based_binlog(0),
   current_ft_item(NULL)
{
  MRN_DBUG_ENTER_METHOD();
  grn_ctx_init(ctx, 0);
  mrn_change_encoding(ctx, system_charset_info);
  grn_ctx_use(ctx, mrn_db);
  GRN_WGS84_GEO_POINT_INIT(&top_left_point, 0);
  GRN_WGS84_GEO_POINT_INIT(&bottom_right_point, 0);
  GRN_WGS84_GEO_POINT_INIT(&source_point, 0);
  GRN_TEXT_INIT(&key_buffer, 0);
  GRN_TEXT_INIT(&encoded_key_buffer, 0);
  GRN_VOID_INIT(&old_value_buffer);
  GRN_VOID_INIT(&new_value_buffer);
  DBUG_VOID_RETURN;
}

ha_mroonga::~ha_mroonga()
{
  MRN_DBUG_ENTER_METHOD();
  if (analyzed_for_create) {
    if (wrap_handler_for_create) {
      delete wrap_handler_for_create;
    }
    if (share_for_create.wrapper_mode) {
      plugin_unlock(NULL, share_for_create.plugin);
    }
    mrn_free_share_alloc(&share_for_create);
    free_root(&mem_root_for_create, MYF(0));
  }
  grn_obj_unlink(ctx, &top_left_point);
  grn_obj_unlink(ctx, &bottom_right_point);
  grn_obj_unlink(ctx, &source_point);
  grn_obj_unlink(ctx, &key_buffer);
  grn_obj_unlink(ctx, &encoded_key_buffer);
  grn_obj_unlink(ctx, &old_value_buffer);
  grn_obj_unlink(ctx, &new_value_buffer);
  grn_ctx_fin(ctx);
  DBUG_VOID_RETURN;
}

const char *ha_mroonga::table_type() const
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(MRN_PLUGIN_NAME_STRING);
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

static const char *ha_mroonga_exts[] = {
  NullS
};
const char **ha_mroonga::bas_ext() const
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(ha_mroonga_exts);
}

uint ha_mroonga::wrapper_max_supported_record_length() const
{
  uint res;
  MRN_DBUG_ENTER_METHOD();
  if (analyzed_for_create && share_for_create.wrapper_mode) {
    res = wrap_handler_for_create->max_supported_record_length();
  } else {
    MRN_SET_WRAP_SHARE_KEY(share, table->s);
    MRN_SET_WRAP_TABLE_KEY(this, table);
    res = wrap_handler->max_supported_record_length();
    MRN_SET_BASE_SHARE_KEY(share, table->s);
    MRN_SET_BASE_TABLE_KEY(this, table);
  }
  DBUG_RETURN(res);
}

uint ha_mroonga::storage_max_supported_record_length() const
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(HA_MAX_REC_LENGTH);
}

uint ha_mroonga::max_supported_record_length() const
{
  MRN_DBUG_ENTER_METHOD();

  uint res;
  if (!share && !analyzed_for_create &&
    (
      thd_sql_command(ha_thd()) == SQLCOM_CREATE_TABLE ||
      thd_sql_command(ha_thd()) == SQLCOM_CREATE_INDEX ||
      thd_sql_command(ha_thd()) == SQLCOM_ALTER_TABLE
    )
  ) {
    create_share_for_create();
  }
  if (analyzed_for_create && share_for_create.wrapper_mode) {
    res = wrapper_max_supported_record_length();
  } else if (wrap_handler && share && share->wrapper_mode) {
    res = wrapper_max_supported_record_length();
  } else {
    res = storage_max_supported_record_length();
  }

  DBUG_RETURN(res);
}

uint ha_mroonga::wrapper_max_supported_keys() const
{
  uint res;
  MRN_DBUG_ENTER_METHOD();
  if (analyzed_for_create && share_for_create.wrapper_mode) {
    res = wrap_handler_for_create->max_supported_keys();
  } else {
    MRN_SET_WRAP_SHARE_KEY(share, table->s);
    MRN_SET_WRAP_TABLE_KEY(this, table);
    res = wrap_handler->max_supported_keys();
    MRN_SET_BASE_SHARE_KEY(share, table->s);
    MRN_SET_BASE_TABLE_KEY(this, table);
  }
  DBUG_RETURN(res);
}

uint ha_mroonga::storage_max_supported_keys() const
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(HA_MAX_REC_LENGTH);
}

uint ha_mroonga::max_supported_keys() const
{
  MRN_DBUG_ENTER_METHOD();

  uint res;
  if (!share && !analyzed_for_create &&
    (
      thd_sql_command(ha_thd()) == SQLCOM_CREATE_TABLE ||
      thd_sql_command(ha_thd()) == SQLCOM_CREATE_INDEX ||
      thd_sql_command(ha_thd()) == SQLCOM_ALTER_TABLE
    )
  ) {
    create_share_for_create();
  }
  if (analyzed_for_create && share_for_create.wrapper_mode) {
    res = wrapper_max_supported_keys();
  } else if (wrap_handler && share && share->wrapper_mode) {
    res = wrapper_max_supported_keys();
  } else {
    res = storage_max_supported_keys();
  }

  DBUG_RETURN(res);
}

uint ha_mroonga::wrapper_max_supported_key_length() const
{
  uint res;
  MRN_DBUG_ENTER_METHOD();
  if (analyzed_for_create && share_for_create.wrapper_mode) {
    res = wrap_handler_for_create->max_supported_key_length();
  } else {
    MRN_SET_WRAP_SHARE_KEY(share, table->s);
    MRN_SET_WRAP_TABLE_KEY(this, table);
    res = wrap_handler->max_supported_key_length();
    MRN_SET_BASE_SHARE_KEY(share, table->s);
    MRN_SET_BASE_TABLE_KEY(this, table);
  }
  DBUG_RETURN(res);
}

uint ha_mroonga::storage_max_supported_key_length() const
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(HA_MAX_REC_LENGTH);
}

uint ha_mroonga::max_supported_key_length() const
{
  MRN_DBUG_ENTER_METHOD();

  uint res;
  if (!share && !analyzed_for_create &&
    (
      thd_sql_command(ha_thd()) == SQLCOM_CREATE_TABLE ||
      thd_sql_command(ha_thd()) == SQLCOM_CREATE_INDEX ||
      thd_sql_command(ha_thd()) == SQLCOM_ALTER_TABLE
    )
  ) {
    create_share_for_create();
  }
  if (analyzed_for_create && share_for_create.wrapper_mode) {
    res = wrapper_max_supported_key_length();
  } else if (wrap_handler && share && share->wrapper_mode) {
    res = wrapper_max_supported_key_length();
  } else {
    res = storage_max_supported_key_length();
  }

  DBUG_RETURN(res);
}

uint ha_mroonga::wrapper_max_supported_key_part_length() const
{
  uint res;
  MRN_DBUG_ENTER_METHOD();
  if (analyzed_for_create && share_for_create.wrapper_mode) {
    res = wrap_handler_for_create->max_supported_key_part_length();
  } else {
    MRN_SET_WRAP_SHARE_KEY(share, table->s);
    MRN_SET_WRAP_TABLE_KEY(this, table);
    res = wrap_handler->max_supported_key_part_length();
    MRN_SET_BASE_SHARE_KEY(share, table->s);
    MRN_SET_BASE_TABLE_KEY(this, table);
  }
  DBUG_RETURN(res);
}

uint ha_mroonga::storage_max_supported_key_part_length() const
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(HA_MAX_REC_LENGTH);
}

uint ha_mroonga::max_supported_key_part_length() const
{
  MRN_DBUG_ENTER_METHOD();

  uint res;
  if (!share && !analyzed_for_create &&
    (
      thd_sql_command(ha_thd()) == SQLCOM_CREATE_TABLE ||
      thd_sql_command(ha_thd()) == SQLCOM_CREATE_INDEX ||
      thd_sql_command(ha_thd()) == SQLCOM_ALTER_TABLE
    )
  ) {
    create_share_for_create();
  }
  if (analyzed_for_create && share_for_create.wrapper_mode) {
    res = wrapper_max_supported_key_part_length();
  } else if (wrap_handler && share && share->wrapper_mode) {
    res = wrapper_max_supported_key_part_length();
  } else {
    res = storage_max_supported_key_part_length();
  }

  DBUG_RETURN(res);
}

ulonglong ha_mroonga::wrapper_table_flags() const
{
  ulonglong table_flags;
  MRN_DBUG_ENTER_METHOD();
  if (analyzed_for_create && share_for_create.wrapper_mode) {
    table_flags = wrap_handler_for_create->ha_table_flags();
  } else {
    MRN_SET_WRAP_SHARE_KEY(share, table->s);
    MRN_SET_WRAP_TABLE_KEY(this, table);
    table_flags = wrap_handler->ha_table_flags();
    MRN_SET_BASE_SHARE_KEY(share, table->s);
    MRN_SET_BASE_TABLE_KEY(this, table);
  }
  table_flags |= HA_CAN_FULLTEXT | HA_PRIMARY_KEY_REQUIRED_FOR_DELETE |
    HA_CAN_RTREEKEYS;
#ifdef HA_CAN_REPAIR
  table_flags |= HA_CAN_REPAIR;
#endif
  DBUG_RETURN(table_flags);
}

ulonglong ha_mroonga::storage_table_flags() const
{
  MRN_DBUG_ENTER_METHOD();
  ulonglong flags =
    HA_NO_TRANSACTIONS |
    HA_PARTIAL_COLUMN_READ |
    HA_REC_NOT_IN_SEQ |
    HA_NULL_IN_KEY |
    HA_CAN_INDEX_BLOBS |
    HA_STATS_RECORDS_IS_EXACT |
    HA_CAN_FULLTEXT |
    HA_CAN_INSERT_DELAYED |
    HA_BINLOG_FLAGS |
    HA_CAN_BIT_FIELD |
    HA_DUPLICATE_POS |
    HA_CAN_GEOMETRY |
    HA_CAN_RTREEKEYS;
    //HA_HAS_RECORDS;
#ifdef HA_MUST_USE_TABLE_CONDITION_PUSHDOWN
  flags |= HA_MUST_USE_TABLE_CONDITION_PUSHDOWN;
#endif
#ifdef HA_CAN_REPAIR
  flags |= HA_CAN_REPAIR;
#endif
  DBUG_RETURN(flags);
}

ulonglong ha_mroonga::table_flags() const
{
  MRN_DBUG_ENTER_METHOD();

  ulonglong flags;
  if (!share && !analyzed_for_create &&
    (
      thd_sql_command(ha_thd()) == SQLCOM_CREATE_TABLE ||
      thd_sql_command(ha_thd()) == SQLCOM_CREATE_INDEX ||
      thd_sql_command(ha_thd()) == SQLCOM_ALTER_TABLE
    )
  ) {
    create_share_for_create();
  }
  if (analyzed_for_create && share_for_create.wrapper_mode) {
    flags = wrapper_table_flags();
  } else if (wrap_handler && share && share->wrapper_mode) {
    flags = wrapper_table_flags();
  } else {
    flags = storage_table_flags();
  }

  DBUG_RETURN(flags);
}

ulong ha_mroonga::wrapper_index_flags(uint idx, uint part, bool all_parts) const
{
  ulong index_flags;
  KEY key = table_share->key_info[idx];
  MRN_DBUG_ENTER_METHOD();
  if (key.algorithm == HA_KEY_ALG_BTREE || key.algorithm == HA_KEY_ALG_UNDEF) {
    MRN_SET_WRAP_SHARE_KEY(share, table->s);
    MRN_SET_WRAP_TABLE_KEY(this, table);
    index_flags = wrap_handler->index_flags(idx, part, all_parts);
    MRN_SET_BASE_SHARE_KEY(share, table->s);
    MRN_SET_BASE_TABLE_KEY(this, table);
  } else {
    index_flags = HA_ONLY_WHOLE_INDEX | HA_KEY_SCAN_NOT_ROR;
  }
  DBUG_RETURN(index_flags);
}

ulong ha_mroonga::storage_index_flags(uint idx, uint part, bool all_parts) const
{
  MRN_DBUG_ENTER_METHOD();
  ulong flags;
  KEY key = table_share->key_info[idx];
  if (key.algorithm == HA_KEY_ALG_BTREE || key.algorithm == HA_KEY_ALG_UNDEF) {
    flags = HA_READ_NEXT | HA_READ_PREV | HA_READ_RANGE | HA_KEYREAD_ONLY;
    if (key.key_parts > 1 || !is_need_normalize(&key.key_part->field[0])) {
      flags |= HA_READ_ORDER;
    }
  } else {
    flags = HA_ONLY_WHOLE_INDEX | HA_KEY_SCAN_NOT_ROR;
  }
  DBUG_RETURN(flags);
}

ulong ha_mroonga::index_flags(uint idx, uint part, bool all_parts) const
{
  MRN_DBUG_ENTER_METHOD();

  KEY key = table_share->key_info[idx];
  if (key.algorithm == HA_KEY_ALG_FULLTEXT) {
    DBUG_RETURN(HA_ONLY_WHOLE_INDEX | HA_KEY_SCAN_NOT_ROR);
  }

  int error = 0;
  if (wrap_handler && share && share->wrapper_mode)
  {
    error = wrapper_index_flags(idx, part, all_parts);
  } else {
    error = storage_index_flags(idx, part, all_parts);
  }
  DBUG_RETURN(error);
}

int ha_mroonga::create_share_for_create() const
{
  int error;
  THD *thd = ha_thd();
  LEX *lex = thd->lex;
  HA_CREATE_INFO *create_info = &lex->create_info;
  TABLE_LIST *table_list = lex->select_lex.table_list.first;
  MRN_DBUG_ENTER_METHOD();
  wrap_handler_for_create = NULL;
  memset(&table_for_create, 0, sizeof(TABLE));
  memset(&share_for_create, 0, sizeof(MRN_SHARE));
  memset(&table_share_for_create, 0, sizeof(TABLE_SHARE));
  if (table_share) {
    table_share_for_create.comment = table_share->comment;
    table_share_for_create.connect_string = table_share->connect_string;
  } else {
    table_share_for_create.comment = create_info->comment;
    table_share_for_create.connect_string = create_info->connect_string;
    if (thd_sql_command(ha_thd()) == SQLCOM_ALTER_TABLE ||
        thd_sql_command(ha_thd()) == SQLCOM_CREATE_INDEX) {
      st_mrn_slot_data *slot_data = mrn_get_slot_data(thd, false);
      if (slot_data && slot_data->alter_create_info) {
        create_info = slot_data->alter_create_info;
        if (slot_data->alter_connect_string) {
          table_share_for_create.connect_string.str =
            slot_data->alter_connect_string;
          table_share_for_create.connect_string.length =
            strlen(slot_data->alter_connect_string);
        } else {
          table_share_for_create.connect_string.str = NULL;
          table_share_for_create.connect_string.length = 0;
        }
        if (slot_data->alter_comment) {
          table_share_for_create.comment.str =
            slot_data->alter_comment;
          table_share_for_create.comment.length =
            strlen(slot_data->alter_comment);
        } else {
          table_share_for_create.comment.str = NULL;
          table_share_for_create.comment.length = 0;
        }
      }
    }
  }
  init_alloc_root(&mem_root_for_create, 1024, 0);
  analyzed_for_create = true;
  if (table_list) {
    share_for_create.table_name = table_list->table_name;
    share_for_create.table_name_length = table_list->table_name_length;
  }
  share_for_create.table_share = &table_share_for_create;
  table_for_create.s = &table_share_for_create;
#ifdef WITH_PARTITION_STORAGE_ENGINE
  table_for_create.part_info = thd->work_part_info;
#endif
  if ((error = mrn_parse_table_param(&share_for_create, &table_for_create)))
    goto error;

  if (share_for_create.wrapper_mode)
  {
    wrap_handler_for_create =
      share_for_create.hton->create(share_for_create.hton, NULL,
                                    &mem_root_for_create);
    if (!wrap_handler_for_create) {
      error = HA_ERR_OUT_OF_MEM;
      goto error;
    }
    wrap_handler_for_create->init();
  }
  DBUG_RETURN(0);

error:
  if (share_for_create.wrapper_mode) {
    plugin_unlock(NULL, share_for_create.plugin);
  }
  mrn_free_share_alloc(&share_for_create);
  free_root(&mem_root_for_create, MYF(0));
  analyzed_for_create = false;
  thd->clear_error();
  DBUG_RETURN(error);
}

int ha_mroonga::wrapper_create(const char *name, TABLE *table,
                               HA_CREATE_INFO *info, MRN_SHARE *tmp_share)
{
  int error = 0;
  handler *hnd;
  MRN_DBUG_ENTER_METHOD();

  if (table_share->primary_key == MAX_KEY)
  {
    my_message(ER_REQUIRES_PRIMARY_KEY, ER(ER_REQUIRES_PRIMARY_KEY), MYF(0));
    DBUG_RETURN(ER_REQUIRES_PRIMARY_KEY);
  }

  mrn::PathMapper mapper(name, mrn_database_path_prefix);
  error = wrapper_create_index(name, table, info, tmp_share,
                               mapper.table_name());
  if (error)
    DBUG_RETURN(error);

  wrap_key_info = mrn_create_key_info_for_table(tmp_share, table, &error);
  if (error)
    DBUG_RETURN(error);
  base_key_info = table->key_info;

  share = tmp_share;
  MRN_SET_WRAP_SHARE_KEY(tmp_share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  if (!(hnd =
      tmp_share->hton->create(tmp_share->hton, table->s,
        current_thd->mem_root)))
  {
    MRN_SET_BASE_SHARE_KEY(tmp_share, table->s);
    MRN_SET_BASE_TABLE_KEY(this, table);
    share = NULL;
    if (wrap_key_info)
    {
      my_free(wrap_key_info, MYF(0));
      wrap_key_info = NULL;
    }
    base_key_info = NULL;
    DBUG_RETURN(HA_ERR_OUT_OF_MEM);
  }
  hnd->init();
  error = hnd->ha_create(name, table, info);
  MRN_SET_BASE_SHARE_KEY(tmp_share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  share = NULL;
  delete hnd;

  if (error) {
    wrapper_delete_index(name, tmp_share, mapper.table_name());
  }

  if (wrap_key_info)
  {
    my_free(wrap_key_info, MYF(0));
    wrap_key_info = NULL;
  }
  base_key_info = NULL;
  DBUG_RETURN(error);
}

int ha_mroonga::wrapper_create_index_fulltext_validate(KEY *key_info)
{
  MRN_DBUG_ENTER_METHOD();

  int error = 0;
  uint i;
  for (i = 0; i < key_info->key_parts; i++) {
    Field *field = key_info->key_part[i].field;

    grn_builtin_type gtype = mrn_grn_type_from_field(ctx, field, true);
    if (gtype != GRN_DB_SHORT_TEXT)
    {
      error = ER_CANT_CREATE_TABLE;
      GRN_LOG(ctx, GRN_LOG_ERROR,
              "key type must be text: <%d> "
              "(TODO: We should show type name not type ID.)",
              field->type());
      my_message(ER_CANT_CREATE_TABLE,
                 "key type must be text. (TODO: We should show type name.)",
                 MYF(0));
      DBUG_RETURN(error);
    }
  }

  DBUG_RETURN(error);
}

int ha_mroonga::wrapper_create_index_fulltext(const char *grn_table_name,
                                              int i,
                                              KEY *key_info,
                                              grn_obj **index_tables,
                                              grn_obj **index_columns,
                                              MRN_SHARE *tmp_share)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;

  error = wrapper_create_index_fulltext_validate(key_info);
  if (error) {
    DBUG_RETURN(error);
  }

  error = mrn_change_encoding(ctx, system_charset_info);
  if (error)
    DBUG_RETURN(error);

  grn_obj_flags index_table_flags =
    GRN_OBJ_TABLE_PAT_KEY |
    GRN_OBJ_PERSISTENT;
  if (is_need_normalize(&key_info->key_part->field[0]))
  {
    index_table_flags |= GRN_OBJ_KEY_NORMALIZE;
  }
  grn_obj *index_table;

  grn_obj_flags index_column_flags =
    GRN_OBJ_COLUMN_INDEX | GRN_OBJ_WITH_POSITION | GRN_OBJ_PERSISTENT;
  if (key_info->key_parts > 1) {
    index_column_flags |= GRN_OBJ_WITH_SECTION;
  }

  grn_obj *lexicon_key_type = grn_ctx_at(ctx, GRN_DB_SHORT_TEXT);
  error = mrn_change_encoding(ctx, key_info->key_part->field->charset());
  if (error) {
    grn_obj_unlink(ctx, lexicon_key_type);
    DBUG_RETURN(error);
  }
  mrn::IndexTableName index_table_name(grn_table_name, key_info->name);
  index_table = grn_table_create(ctx,
                                 index_table_name.c_str(),
                                 index_table_name.length(),
                                 NULL,
                                 index_table_flags, lexicon_key_type, 0);
  if (ctx->rc) {
    error = ER_CANT_CREATE_TABLE;
    my_message(ER_CANT_CREATE_TABLE, ctx->errbuf, MYF(0));
    grn_obj_unlink(ctx, lexicon_key_type);
    DBUG_RETURN(error);
  }
  mrn_change_encoding(ctx, system_charset_info);
  grn_obj_unlink(ctx, lexicon_key_type);
  index_tables[i] = index_table;

  grn_info_type info_type = GRN_INFO_DEFAULT_TOKENIZER;
  grn_obj *tokenizer = find_tokenizer(tmp_share->key_parser[i],
                                      tmp_share->key_parser_length[i]);
  grn_obj_set_info(ctx, index_table, info_type, tokenizer);
  grn_obj_unlink(ctx, tokenizer);

  grn_obj *index_column = grn_column_create(ctx, index_table,
                                            index_column_name,
                                            strlen(index_column_name),
                                            NULL,
                                            index_column_flags,
                                            grn_table);
  if (ctx->rc) {
    error = ER_CANT_CREATE_TABLE;
    my_message(error, ctx->errbuf, MYF(0));
    DBUG_RETURN(error);
  }
  if (index_columns) {
    index_columns[i] = index_column;
  } else {
    grn_obj_unlink(ctx, index_column);
  }

  DBUG_RETURN(error);
}

int ha_mroonga::wrapper_create_index_geo(const char *grn_table_name,
                                         int i,
                                         KEY *key_info,
                                         grn_obj **index_tables,
                                         grn_obj **index_columns,
                                         MRN_SHARE *tmp_share)
{
  MRN_DBUG_ENTER_METHOD();
  int error;

  error = mrn_change_encoding(ctx, system_charset_info);
  if (error)
    DBUG_RETURN(error);

  mrn::IndexTableName index_table_name(grn_table_name, key_info->name);

  grn_obj_flags index_table_flags =
    GRN_OBJ_TABLE_PAT_KEY |
    GRN_OBJ_PERSISTENT;
  grn_obj *index_table;

  grn_obj_flags index_column_flags =
    GRN_OBJ_COLUMN_INDEX | GRN_OBJ_PERSISTENT;

  grn_obj *lexicon_key_type = grn_ctx_at(ctx, GRN_DB_WGS84_GEO_POINT);
  index_table = grn_table_create(ctx,
                                 index_table_name.c_str(),
                                 index_table_name.length(),
                                 NULL,
                                 index_table_flags, lexicon_key_type, 0);
  if (ctx->rc) {
    error = ER_CANT_CREATE_TABLE;
    my_message(ER_CANT_CREATE_TABLE, ctx->errbuf, MYF(0));
    grn_obj_unlink(ctx, lexicon_key_type);
    DBUG_RETURN(error);
  }
  grn_obj_unlink(ctx, lexicon_key_type);
  index_tables[i] = index_table;

  grn_obj *index_column = grn_column_create(ctx, index_table,
                                            index_column_name,
                                            strlen(index_column_name),
                                            NULL,
                                            index_column_flags,
                                            grn_table);
  if (ctx->rc) {
    error = ER_CANT_CREATE_TABLE;
    my_message(error, ctx->errbuf, MYF(0));
    DBUG_RETURN(error);
  }
  if (index_columns) {
    index_columns[i] = index_column;
  } else {
    grn_obj_unlink(ctx, index_column);
  }

  DBUG_RETURN(error);
}

int ha_mroonga::wrapper_create_index(const char *name, TABLE *table,
                                     HA_CREATE_INFO *info,
                                     MRN_SHARE *tmp_share,
                                     const char *grn_table_name)
{
  MRN_DBUG_ENTER_METHOD();

  int error = 0;
  error = ensure_database_create(name);
  if (error)
    DBUG_RETURN(error);

  error = mrn_change_encoding(ctx, system_charset_info);
  if (error)
    DBUG_RETURN(error);

  grn_obj *grn_index_table;
  char *grn_table_path = NULL;     // we don't specify path
  grn_obj *pkey_type = grn_ctx_at(ctx, GRN_DB_SHORT_TEXT);
  grn_obj *pkey_value_type = NULL; // we don't use this
  grn_obj_flags grn_table_flags = GRN_OBJ_PERSISTENT | GRN_OBJ_TABLE_HASH_KEY;

  grn_index_table = grn_table_create(ctx, grn_table_name, strlen(grn_table_name),
                                     grn_table_path, grn_table_flags,
                                     pkey_type, pkey_value_type);
  if (ctx->rc) {
    error = ER_CANT_CREATE_TABLE;
    my_message(error, ctx->errbuf, MYF(0));
    DBUG_RETURN(error);
  }
  if (grn_table) {
    grn_obj_unlink(ctx, grn_table);
  }
  grn_table = grn_index_table;

  uint i;
  uint n_keys = table->s->keys;
  grn_obj *index_tables[n_keys];
  if (!tmp_share->disable_keys) {
    for (i = 0; i < n_keys; i++) {
      index_tables[i] = NULL;

      KEY key_info = table->s->key_info[i];
      if (key_info.algorithm == HA_KEY_ALG_FULLTEXT) {
        error = wrapper_create_index_fulltext(grn_table_name,
                                              i, &key_info,
                                              index_tables, NULL, tmp_share);
      } else if (mrn_is_geo_key(&key_info)) {
        error = wrapper_create_index_geo(grn_table_name,
                                         i, &key_info,
                                         index_tables, NULL, tmp_share);
      }
    }
  }

  if (error)
  {
    for (uint j = 0; j < i; j++) {
      if (index_tables[j])
      {
        grn_obj_remove(ctx, index_tables[j]);
      }
    }
    grn_obj_remove(ctx, grn_table);
    grn_table = NULL;
  }

  DBUG_RETURN(error);
}

int ha_mroonga::storage_create(const char *name, TABLE *table,
                               HA_CREATE_INFO *info, MRN_SHARE *tmp_share)
{
  int error;
  MRN_DBUG_ENTER_METHOD();

  error = storage_create_validate_pseudo_column(table);
  if (error)
    DBUG_RETURN(error);

  error = storage_create_validate_index(table);
  if (error)
    DBUG_RETURN(error);

  error = ensure_database_create(name);
  if (error)
    DBUG_RETURN(error);

  error = mrn_change_encoding(ctx, system_charset_info);
  if (error)
    DBUG_RETURN(error);

  grn_obj_flags table_flags = GRN_OBJ_PERSISTENT;

  /* primary key must be handled before creating table */
  grn_obj *pkey_type;
  uint pkey_nr = table->s->primary_key;
  if (pkey_nr != MAX_INDEXES) {
    KEY key_info = table->s->key_info[pkey_nr];
    bool is_id;

    int key_parts = key_info.key_parts;
    if (key_parts == 1) {
      Field *pkey_field = key_info.key_part[0].field;
      const char *column_name = pkey_field->field_name;
      int column_name_size = strlen(column_name);
      is_id = (strncmp(MRN_COLUMN_NAME_ID, column_name, column_name_size) == 0);

      grn_builtin_type gtype = mrn_grn_type_from_field(ctx, pkey_field, false);
      pkey_type = grn_ctx_at(ctx, gtype);
    } else {
      is_id = false;
      pkey_type = grn_ctx_at(ctx, GRN_DB_SHORT_TEXT);
    }

    // default algorithm is BTREE ==> PAT
    if (!is_id && key_info.algorithm == HA_KEY_ALG_HASH) {
      table_flags |= GRN_OBJ_TABLE_HASH_KEY;
    } else if (!is_id) {
      table_flags |= GRN_OBJ_TABLE_PAT_KEY;
      if (
        key_parts == 1 &&
        is_need_normalize(&key_info.key_part->field[0])
      ) {
        table_flags |= GRN_OBJ_KEY_NORMALIZE;
      }
    } else {
      // for _id
      table_flags |= GRN_OBJ_TABLE_NO_KEY;
      pkey_type = NULL;
    }

  } else {
    // primary key doesn't exists
    table_flags |= GRN_OBJ_TABLE_NO_KEY;
    pkey_type = NULL;
  }

  /* create table */
  grn_obj *table_obj;
  mrn::PathMapper mapper(name, mrn_database_path_prefix);

  char *table_path = NULL;           // we don't specify path
  grn_obj *pkey_value_type = NULL; // we don't use this

  table_obj = grn_table_create(ctx,
                               mapper.table_name(), strlen(mapper.table_name()),
                               table_path,
                               table_flags, pkey_type, pkey_value_type);
  if (ctx->rc) {
    error = ER_CANT_CREATE_TABLE;
    my_message(error, ctx->errbuf, MYF(0));
    DBUG_RETURN(error);
  }

  /* create columns */
  uint n_columns = table->s->fields;
  for (uint i = 0; i < n_columns; i++) {
    grn_obj *col_type;
    Field *field = table->s->field[i];
    const char *column_name = field->field_name;
    int column_name_size = strlen(column_name);

    if (strncmp(MRN_COLUMN_NAME_ID, column_name, column_name_size) == 0) {
      continue;
    }

    grn_obj_flags col_flags = GRN_OBJ_PERSISTENT | GRN_OBJ_COLUMN_SCALAR;
    grn_builtin_type gtype = mrn_grn_type_from_field(ctx, field, false);
    col_type = grn_ctx_at(ctx, gtype);
    char *col_path = NULL; // we don't specify path

    grn_column_create(ctx, table_obj, column_name, column_name_size,
                      col_path, col_flags, col_type);
    if (ctx->rc) {
      grn_obj_remove(ctx, table_obj);
      error = ER_CANT_CREATE_TABLE;
      my_message(error, ctx->errbuf, MYF(0));
      DBUG_RETURN(error);
    }
  }

  error = storage_create_indexes(table, mapper.table_name(), table_obj,
                                 tmp_share);
  if (error) {
    grn_obj_remove(ctx, table_obj);
    table_obj = NULL;
  }

  if (table_obj) {
    grn_obj_unlink(ctx, table_obj);
  }

  DBUG_RETURN(error);
}

int ha_mroonga::storage_create_validate_pseudo_column(TABLE *table)
{
  int error = 0;
  uint i, n_columns;

  MRN_DBUG_ENTER_METHOD();
  n_columns = table->s->fields;
  for (i = 0; i < n_columns; i++) {
    Field *field = table->s->field[i];
    const char *column_name = field->field_name;
    int column_name_size = strlen(column_name);
    if (strncmp(MRN_COLUMN_NAME_ID, column_name, column_name_size) == 0) {
      switch (field->type()) {
      case MYSQL_TYPE_TINY :
      case MYSQL_TYPE_SHORT :
      case MYSQL_TYPE_INT24 :
      case MYSQL_TYPE_LONG :
      case MYSQL_TYPE_LONGLONG :
        break;
      default:
        GRN_LOG(ctx, GRN_LOG_ERROR, "_id must be numeric data type");
        error = ER_CANT_CREATE_TABLE;
        my_message(error, "_id must be numeric data type", MYF(0));
        DBUG_RETURN(error);
      }
    } else if (strncmp(MRN_COLUMN_NAME_SCORE, column_name, column_name_size) == 0) {
      switch (field->type()) {
      case MYSQL_TYPE_FLOAT :
      case MYSQL_TYPE_DOUBLE :
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

int ha_mroonga::storage_create_validate_index(TABLE *table)
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
      continue;
    }
    Field *field = key_info.key_part[0].field;
    const char *column_name = field->field_name;
    int column_name_size = strlen(column_name);
    if (strncmp(MRN_COLUMN_NAME_ID, column_name, column_name_size) == 0) {
      if (key_info.algorithm == HA_KEY_ALG_HASH) {
        continue; // hash index is ok
      }
      GRN_LOG(ctx, GRN_LOG_ERROR, "only hash index can be defined for _id");
      error = ER_CANT_CREATE_TABLE;
      my_message(error, "only hash index can be defined for _id", MYF(0));
      DBUG_RETURN(error);
    }
  }

  DBUG_RETURN(error);
}

int ha_mroonga::storage_create_index(TABLE *table, const char *grn_table_name,
                                     grn_obj *grn_table, MRN_SHARE *tmp_share,
                                     KEY *key_info, grn_obj **index_tables,
                                     grn_obj **index_columns, uint i)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  grn_obj *index_table, *index_column;
  grn_obj *column = NULL;
  grn_obj *index_type = NULL;

  error = mrn_change_encoding(ctx, system_charset_info);
  if (error)
    DBUG_RETURN(error);

  mrn::IndexTableName index_table_name(grn_table_name, key_info->name);
  bool is_multiple_column_index = key_info->key_parts > 1;
  grn_obj_flags index_table_flags = GRN_OBJ_PERSISTENT;
  grn_obj_flags index_column_flags =
    GRN_OBJ_COLUMN_INDEX | GRN_OBJ_WITH_POSITION | GRN_OBJ_PERSISTENT;
  if (is_multiple_column_index) {
    index_type = grn_ctx_at(ctx, GRN_DB_SHORT_TEXT);
    index_column_flags |= GRN_OBJ_WITH_SECTION;
  } else {
    Field *field = key_info->key_part[0].field;
    const char *column_name = field->field_name;
    int column_name_size = strlen(column_name);

    if (strncmp(MRN_COLUMN_NAME_ID, column_name, column_name_size) == 0) {
      // skipping _id virtual column
      DBUG_RETURN(0);
    }

    column = grn_obj_column(ctx, grn_table, column_name, column_name_size);
    grn_builtin_type groonga_type = mrn_grn_type_from_field(ctx, field, true);
    index_type = grn_ctx_at(ctx, groonga_type);
  }

  int key_alg = key_info->algorithm;
  if (key_info->flags & HA_FULLTEXT) {
    index_table_flags |= GRN_OBJ_TABLE_PAT_KEY;
    if (is_need_normalize(&key_info->key_part->field[0]))
    {
      index_table_flags |= GRN_OBJ_KEY_NORMALIZE;
    }
    error = mrn_change_encoding(ctx, key_info->key_part->field->charset());
    if (error) {
      grn_obj_remove(ctx, grn_table);
      DBUG_RETURN(error);
    }
  } else if (key_alg == HA_KEY_ALG_HASH) {
    index_table_flags |= GRN_OBJ_TABLE_HASH_KEY;
  } else {
    index_table_flags |= GRN_OBJ_TABLE_PAT_KEY;
    if (!is_multiple_column_index &&
        is_need_normalize(&key_info->key_part->field[0])) {
      index_table_flags |= GRN_OBJ_KEY_NORMALIZE;
    }
  }

  index_table = grn_table_create(ctx,
                                 index_table_name.c_str(),
                                 index_table_name.length(),
                                 NULL,
                                 index_table_flags,
                                 index_type,
                                 NULL);
  if (ctx->rc) {
    grn_obj_remove(ctx, grn_table);
    error = ER_CANT_CREATE_TABLE;
    my_message(ER_CANT_CREATE_TABLE, ctx->errbuf, MYF(0));
    DBUG_RETURN(error);
  }

  if (key_info->flags & HA_FULLTEXT) {
    grn_info_type info_type = GRN_INFO_DEFAULT_TOKENIZER;
    grn_obj *tokenizer = find_tokenizer(tmp_share->key_parser[i],
                                        tmp_share->key_parser_length[i]);
    grn_obj_set_info(ctx, index_table, info_type, tokenizer);
    grn_obj_unlink(ctx, tokenizer);
  }

  index_column = grn_column_create(ctx,
                                  index_table,
                                  index_column_name,
                                  strlen(index_column_name),
                                  NULL,
                                  index_column_flags,
                                  grn_table);

  if (ctx->rc) {
    grn_obj_remove(ctx, index_table);
    error = ER_CANT_CREATE_TABLE;
    my_message(error, ctx->errbuf, MYF(0));
    DBUG_RETURN(error);
  }

  mrn_change_encoding(ctx, system_charset_info);
  if (is_multiple_column_index) {
    if (key_info->flags & HA_FULLTEXT) {
      grn_obj source_ids;
      GRN_UINT32_INIT(&source_ids, GRN_OBJ_VECTOR);

      int j, n_key_parts = key_info->key_parts;
      for (j = 0; j < n_key_parts; j++) {
        Field *field = key_info->key_part[j].field;
        const char *column_name = field->field_name;
        int column_name_size = strlen(column_name);
        grn_obj *source_column = grn_obj_column(ctx, grn_table,
                                                column_name, column_name_size);
        grn_id source_id = grn_obj_id(ctx, source_column);
        GRN_UINT32_PUT(ctx, &source_ids, source_id);
        grn_obj_unlink(ctx, source_column);
      }
      grn_obj_set_info(ctx, index_column, GRN_INFO_SOURCE, &source_ids);
      grn_obj_unlink(ctx, &source_ids);
    }
  } else {
    if (column) {
      grn_obj source_ids;
      grn_id source_id = grn_obj_id(ctx, column);
      GRN_UINT32_INIT(&source_ids, GRN_OBJ_VECTOR);
      GRN_UINT32_PUT(ctx, &source_ids, source_id);
      grn_obj_set_info(ctx, index_column, GRN_INFO_SOURCE, &source_ids);
      grn_obj_unlink(ctx, &source_ids);
    }
  }

  index_tables[i] = index_table;
  if (index_columns)
    index_columns[i] = index_column;
  if (column) {
    grn_obj_unlink(ctx, column);
  }
  DBUG_RETURN(error);
}

int ha_mroonga::storage_create_indexes(TABLE *table, const char *grn_table_name,
                                       grn_obj *grn_table, MRN_SHARE *tmp_share)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;

  uint n_keys = table->s->keys;
  uint i;
  grn_obj *index_tables[n_keys];
  for (i = 0; i < n_keys; i++) {
    index_tables[i] = NULL;
    if (i == table->s->primary_key) {
      continue; // pkey is already handled
    }
    KEY *key_info = &table->s->key_info[i];
    if (tmp_share->disable_keys && !(key_info->flags & HA_NOSAME)) {
      continue; // key is disabled
    }
    if ((error = storage_create_index(table, grn_table_name, grn_table,
                                      tmp_share, key_info,
                                      index_tables, NULL, i))) {
      break;
    }
  }
  if (error) {
    while (true) {
      if (index_tables[i]) {
        grn_obj_remove(ctx, index_tables[i]);
      }
      if (!i)
        break;
      i--;
    }
  }

  DBUG_RETURN(error);
}

int ha_mroonga::close_databases()
{
  MRN_DBUG_ENTER_METHOD();

  int error = 0;
  pthread_mutex_lock(&mrn_db_mutex);

  grn_hash_cursor *hash_cursor;
  hash_cursor = grn_hash_cursor_open(&mrn_ctx, mrn_hash,
                                     NULL, 0, NULL, 0,
                                     0, -1, 0);
  if (mrn_ctx.rc) {
    my_message(ER_ERROR_ON_READ, mrn_ctx.errbuf, MYF(0));
    DBUG_RETURN(ER_ERROR_ON_READ);
  }

  while (grn_hash_cursor_next(&mrn_ctx, hash_cursor) != GRN_ID_NIL) {
    if (mrn_ctx.rc) {
      error = ER_ERROR_ON_READ;
      my_message(error, mrn_ctx.errbuf, MYF(0));
      break;
    }
    void *value;
    grn_obj *db;
    grn_hash_cursor_get_value(&mrn_ctx, hash_cursor, &value);
    memcpy(&db, value, sizeof(grn_obj *));
    grn_rc rc = grn_hash_cursor_delete(&mrn_ctx, hash_cursor, NULL);
    if (rc)
    {
      error = ER_ERROR_ON_READ;
      my_message(error, mrn_ctx.errbuf, MYF(0));
      break;
    }
    grn_obj_close(&mrn_ctx, db);
  }
  grn_hash_cursor_close(&mrn_ctx, hash_cursor);

  pthread_mutex_unlock(&mrn_db_mutex);
  DBUG_RETURN(error);
}

void ha_mroonga::ensure_database_directory()
{
  MRN_DBUG_ENTER_METHOD();

  if (!mrn_database_path_prefix)
    DBUG_VOID_RETURN;

  const char *last_path_separator = strrchr(mrn_database_path_prefix, '/');
  if (!last_path_separator)
    DBUG_VOID_RETURN;
  if (mrn_database_path_prefix == last_path_separator)
    DBUG_VOID_RETURN;

  char database_directory[MRN_MAX_PATH_SIZE];
  size_t database_directory_length =
    last_path_separator - mrn_database_path_prefix;
  strncpy(database_directory, mrn_database_path_prefix,
          database_directory_length);
  database_directory[database_directory_length] = '\0';
  mkdir_p(database_directory);

  DBUG_VOID_RETURN;
}

int ha_mroonga::ensure_database_create(const char *name)
{
  int error;

  MRN_DBUG_ENTER_METHOD();
  /* before creating table, we must check if database is alreadly opened, created */
  grn_obj *db;
  struct stat db_stat;

  error = mrn_change_encoding(ctx, system_charset_info);
  if (error)
    DBUG_RETURN(error);

  mrn::PathMapper mapper(name, mrn_database_path_prefix);
  pthread_mutex_lock(&mrn_db_mutex);
  if (!mrn_hash_get(&mrn_ctx, mrn_hash, mapper.db_name(), &db)) {
    if (stat(mapper.db_path(), &db_stat)) {
      // creating new database
      GRN_LOG(ctx, GRN_LOG_INFO,
              "database not found. creating...(%s)", mapper.db_path());
      if (strncmp(name, "./", 2) == 0) {
        ensure_database_directory();
      }
      db = grn_db_create(&mrn_ctx, mapper.db_path(), NULL);
      if (mrn_ctx.rc) {
        pthread_mutex_unlock(&mrn_db_mutex);
        error = ER_CANT_CREATE_TABLE;
        my_message(error, mrn_ctx.errbuf, MYF(0));
        DBUG_RETURN(error);
      }
    } else {
      // opening existing database
      db = grn_db_open(&mrn_ctx, mapper.db_path());
      if (mrn_ctx.rc) {
        pthread_mutex_unlock(&mrn_db_mutex);
        error = ER_CANT_OPEN_FILE;
        my_message(error, mrn_ctx.errbuf, MYF(0));
        DBUG_RETURN(error);
      }
    }
    mrn_hash_put(&mrn_ctx, mrn_hash, mapper.db_name(), db);
  }
  pthread_mutex_unlock(&mrn_db_mutex);
  grn_ctx_use(ctx, db);

  DBUG_RETURN(error);
}

int ha_mroonga::ensure_database_open(const char *name)
{
  int error;

  MRN_DBUG_ENTER_METHOD();
  grn_obj *db;

  error = mrn_change_encoding(ctx, system_charset_info);
  if (error)
    DBUG_RETURN(error);

  mrn::PathMapper mapper(name, mrn_database_path_prefix);
  pthread_mutex_lock(&mrn_db_mutex);
  if (!mrn_hash_get(&mrn_ctx, mrn_hash, mapper.db_name(), &db)) {
    db = grn_db_open(&mrn_ctx, mapper.db_path());
    if (ctx->rc) {
      pthread_mutex_unlock(&mrn_db_mutex);
      error = ER_CANT_OPEN_FILE;
      my_message(error, ctx->errbuf, MYF(0));
      DBUG_RETURN(error);
    }
    mrn_hash_put(&mrn_ctx, mrn_hash, mapper.db_name(), db);
  }
  pthread_mutex_unlock(&mrn_db_mutex);
  grn_ctx_use(ctx, db);

  DBUG_RETURN(error);
}


int ha_mroonga::create(const char *name, TABLE *table, HA_CREATE_INFO *info)
{
  int error = 0;
  MRN_SHARE *tmp_share;
  MRN_DBUG_ENTER_METHOD();
  /* checking data type of virtual columns */

  if (!(tmp_share = mrn_get_share(name, table, &error)))
    DBUG_RETURN(error);

  st_mrn_slot_data *slot_data = mrn_get_slot_data(ha_thd(), FALSE);
  if (slot_data && slot_data->disable_keys_create_info == info) {
    tmp_share->disable_keys = TRUE;
  }

  if (tmp_share->wrapper_mode)
  {
    error = wrapper_create(name, table, info, tmp_share);
  } else {
    error = storage_create(name, table, info, tmp_share);
  }

  mrn_free_share(tmp_share);
  DBUG_RETURN(error);
}

int ha_mroonga::wrapper_open(const char *name, int mode, uint test_if_locked)
{
  int error = 0;
  MRN_DBUG_ENTER_METHOD();

  error = ensure_database_open(name);
  if (error)
    DBUG_RETURN(error);

  error = open_table(name);
  if (error)
    DBUG_RETURN(error);

  error = wrapper_open_indexes(name, thd_sql_command(ha_thd()) == SQLCOM_REPAIR);
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
    wrap_handler->init();
    error = wrap_handler->ha_open(table, name, mode, test_if_locked);
  } else {
#ifdef MRN_HANDLER_CLONE_NEED_NAME
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
  key_used_on_scan = wrap_handler->key_used_on_scan;
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  init();
  wrapper_overwrite_index_bits();
  wrapper_set_keys_in_use();

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

int ha_mroonga::wrapper_open_indexes(const char *name, bool ignore_open_error)
{
  int error;

  MRN_DBUG_ENTER_METHOD();

  error = mrn_change_encoding(ctx, system_charset_info);
  if (error)
    DBUG_RETURN(error);

  uint n_keys = table->s->keys;
  uint n_primary_keys = table->s->primary_key;
  if (n_keys > 0) {
    // TODO: reduce allocate memories. We only need just
    // for HA_KEY_ALG_FULLTEXT keys.
    grn_index_tables = (grn_obj **)malloc(sizeof(grn_obj *) * n_keys);
    grn_index_columns = (grn_obj **)malloc(sizeof(grn_obj *) * n_keys);
  } else {
    grn_index_tables = grn_index_columns = NULL;
  }

  mrn::PathMapper mapper(name, mrn_database_path_prefix);
  uint i = 0;
  for (i = 0; i < n_keys; i++) {
    KEY key_info = table->s->key_info[i];

    grn_index_tables[i] = NULL;
    grn_index_columns[i] = NULL;

    if (!(wrapper_is_target_index(&key_info))) {
      continue;
    }

    if (i == n_primary_keys) {
      continue;
    }

    mrn::IndexTableName index_table_name(mapper.table_name(), key_info.name);
    grn_index_tables[i] = grn_ctx_get(ctx,
                                      index_table_name.c_str(),
                                      index_table_name.length());
    if (ctx->rc) {
      DBUG_PRINT("info",
                 ("mroonga: sql_command=%u", thd_sql_command(ha_thd())));
      if (ignore_open_error)
      {
        DBUG_PRINT("info", ("mroonga: continue"));
        grn_index_tables[i] = NULL;
        continue;
      }
      error = ER_CANT_OPEN_FILE;
      my_message(error, ctx->errbuf, MYF(0));
      goto error;
    }

    grn_index_columns[i] = grn_obj_column(ctx, grn_index_tables[i],
                                          index_column_name,
                                          strlen(index_column_name));
    if (!grn_index_columns[i]) {
      /* just for backward compatibility before 1.0. */
      Field *field = key_info.key_part[0].field;
      grn_index_columns[i] = grn_obj_column(ctx, grn_index_tables[i],
                                            field->field_name,
                                            strlen(field->field_name));
    }

    if (ctx->rc) {
      DBUG_PRINT("info",
        ("mroonga: sql_command=%u", thd_sql_command(ha_thd())));
      if (ignore_open_error)
      {
        DBUG_PRINT("info", ("mroonga: continue"));
        grn_index_columns[i] = NULL;
        continue;
      }
      error = ER_CANT_OPEN_FILE;
      my_message(error, ctx->errbuf, MYF(0));
      grn_obj_unlink(ctx, grn_index_tables[i]);
      goto error;
    }
  }

  grn_bulk_space(ctx, &key_buffer, table->key_info->key_length);

error:
  if (error) {
    while (i-- > 0) {
      grn_obj *index_column = grn_index_columns[i];
      if (index_column) {
        grn_obj_unlink(ctx, index_column);
      }
      grn_obj *index_table = grn_index_tables[i];
      if (index_table) {
        grn_obj_unlink(ctx, index_table);
      }
    }
    free(grn_index_columns);
    free(grn_index_tables);
    grn_index_columns = NULL;
    grn_index_tables = NULL;
  }

  DBUG_RETURN(error);
}

void ha_mroonga::wrapper_overwrite_index_bits()
{
  uint i, j;
  longlong table_option = table_flags();
  MRN_DBUG_ENTER_METHOD();
  table_share->keys_for_keyread.clear_all();
  for (i = 0; i < table_share->fields; i++)
  {
    Field *field = table_share->field[i];
    field->part_of_key.clear_all();
    field->part_of_key_not_clustered.clear_all();
    field->part_of_sortkey.clear_all();
  }
  for (i = 0; i < table_share->keys; i++) {
    KEY *key_info = &table->s->key_info[i];
    KEY_PART_INFO *key_part = key_info->key_part;
    for (j = 0 ; j < key_info->key_parts; key_part++, j++)
    {
      Field *field = key_part->field;
      if (field->key_length() == key_part->length &&
          !(field->flags & BLOB_FLAG))
      {
        if (index_flags(i, j, 0) & HA_KEYREAD_ONLY)
        {
          table_share->keys_for_keyread.set_bit(i);
          field->part_of_key.set_bit(i);
          field->part_of_key_not_clustered.set_bit(i);
        }
        if (index_flags(i, j, 1) & HA_READ_ORDER)
          field->part_of_sortkey.set_bit(i);
      }
      if (i == table_share->primary_key &&
          (table_option & HA_PRIMARY_KEY_IN_READ_INDEX))
      {
        if (field->key_length() == key_part->length &&
            !(field->flags & BLOB_FLAG))
          field->part_of_key = table_share->keys_in_use;
        if (field->part_of_sortkey.is_set(i))
          field->part_of_sortkey = table_share->keys_in_use;
      }
    }
  }
  DBUG_VOID_RETURN;
}

int ha_mroonga::storage_open(const char *name, int mode, uint test_if_locked)
{
  int error = 0;
  MRN_DBUG_ENTER_METHOD();

  error = ensure_database_open(name);
  if (error)
    DBUG_RETURN(error);

  error = open_table(name);
  if (error)
    DBUG_RETURN(error);

  error = storage_open_columns();
  if (error) {
    grn_obj_unlink(ctx, grn_table);
    grn_table = NULL;
    DBUG_RETURN(error);
  }

  error = storage_open_indexes(name);
  if (error) {
    // TODO: free grn_columns and set NULL;
    grn_obj_unlink(ctx, grn_table);
    grn_table = NULL;
    DBUG_RETURN(error);
  }

  storage_set_keys_in_use();

  ref_length = sizeof(grn_id);
  DBUG_RETURN(0);
}

int ha_mroonga::open_table(const char *name)
{
  int error;
  MRN_DBUG_ENTER_METHOD();

  error = mrn_change_encoding(ctx, system_charset_info);
  if (error)
    DBUG_RETURN(error);

  mrn::PathMapper mapper(name, mrn_database_path_prefix);
  grn_table = grn_ctx_get(ctx, mapper.table_name(), strlen(mapper.table_name()));
  if (ctx->rc) {
    error = ER_CANT_OPEN_FILE;
    my_message(error, ctx->errbuf, MYF(0));
    DBUG_RETURN(error);
  }

  DBUG_RETURN(0);
}

int ha_mroonga::storage_open_columns(void)
{
  int error;
  MRN_DBUG_ENTER_METHOD();

  error = mrn_change_encoding(ctx, system_charset_info);
  if (error)
    DBUG_RETURN(error);

  int n_columns = table->s->fields;
  grn_columns = (grn_obj **)malloc(sizeof(grn_obj *) * n_columns);

  int i;
  for (i = 0; i < n_columns; i++) {
    Field *field = table->field[i];
    const char *column_name = field->field_name;
    int column_name_size = strlen(column_name);

    if (strncmp(MRN_COLUMN_NAME_ID, column_name, column_name_size) == 0) {
      grn_columns[i] = NULL;
      continue;
    }
    if (strncmp(MRN_COLUMN_NAME_SCORE, column_name, column_name_size) == 0) {
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

int ha_mroonga::storage_open_indexes(const char *name)
{
  int error;

  MRN_DBUG_ENTER_METHOD();

  error = mrn_change_encoding(ctx, system_charset_info);
  if (error)
    DBUG_RETURN(error);

  uint n_keys = table->s->keys;
  uint pkey_nr = table->s->primary_key;
  if (n_keys > 0) {
    grn_index_tables = (grn_obj **)malloc(sizeof(grn_obj *) * n_keys);
    grn_index_columns = (grn_obj **)malloc(sizeof(grn_obj *) * n_keys);
    key_id = (grn_id *)malloc(sizeof(grn_id) * n_keys);
    del_key_id = (grn_id *)malloc(sizeof(grn_id) * n_keys);
  } else {
    grn_index_tables = grn_index_columns = NULL;
    key_id = NULL;
    del_key_id = NULL;
  }

  mrn::PathMapper mapper(name, mrn_database_path_prefix);
  uint i, j;
  for (i = 0; i < n_keys; i++) {
    if (i == pkey_nr) {
      grn_index_tables[i] = grn_index_columns[i] = NULL;
      continue;
    }

    KEY key_info = table->s->key_info[i];
    if (key_info.key_parts > 1) {
      KEY_PART_INFO *key_part = key_info.key_part;
      for (j = 0; j < key_info.key_parts; j++) {
        bitmap_set_bit(&multiple_column_key_bitmap,
                       key_part[j].field->field_index);
      }
    }

    mrn::IndexTableName index_table_name(mapper.table_name(), key_info.name);
    grn_index_tables[i] = grn_ctx_get(ctx,
                                      index_table_name.c_str(),
                                      index_table_name.length());
    if (ctx->rc) {
      error = ER_CANT_OPEN_FILE;
      my_message(error, ctx->errbuf, MYF(0));
      goto error;
    }

    grn_index_columns[i] = grn_obj_column(ctx,
                                          grn_index_tables[i],
                                          index_column_name,
                                          strlen(index_column_name));
    if (!grn_index_columns[i]) {
      /* just for backward compatibility before 1.0. */
      Field *field = key_info.key_part[0].field;
      grn_index_columns[i] = grn_obj_column(ctx, grn_index_tables[i],
                                            field->field_name,
                                            strlen(field->field_name));
    }

    if (ctx->rc) {
      error = ER_CANT_OPEN_FILE;
      my_message(error, ctx->errbuf, MYF(0));
      goto error;
    }
  }

error:
  if (error) {
    if (i) {
      while (true) {
        grn_obj *index_column = grn_index_columns[i];
        if (index_column) {
          grn_obj_unlink(ctx, index_column);
        }
        grn_obj *index_table = grn_index_tables[i];
        if (index_table) {
          grn_obj_unlink(ctx, index_table);
        }
        if (!i)
          break;
        i--;
      }
    }
    free(key_id);
    free(del_key_id);
    free(grn_index_columns);
    free(grn_index_tables);
    key_id = NULL;
    del_key_id = NULL;
    grn_index_columns = NULL;
    grn_index_tables = NULL;
  }

  DBUG_RETURN(error);
}

int ha_mroonga::open(const char *name, int mode, uint test_if_locked)
{
  int error = 0;
  MRN_DBUG_ENTER_METHOD();

  if (!(share = mrn_get_share(name, table, &error)))
    DBUG_RETURN(error);
  thr_lock_data_init(&share->lock,&thr_lock_data,NULL);

  if (bitmap_init(&multiple_column_key_bitmap, NULL, table->s->fields, false))
  {
    mrn_free_share(share);
    share = NULL;
    DBUG_RETURN(HA_ERR_OUT_OF_MEM);
  }

  if (share->wrapper_mode)
  {
    error = wrapper_open(name, mode, test_if_locked);
  } else {
    error = storage_open(name, mode, test_if_locked);
  }

  if (error)
  {
    bitmap_free(&multiple_column_key_bitmap);
    mrn_free_share(share);
    share = NULL;
  }
  DBUG_RETURN(error);
}

int ha_mroonga::wrapper_close()
{
  int error = 0;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
#ifdef MRN_HANDLER_HAVE_HA_CLOSE
  error = wrap_handler->ha_close();
#else
  error = wrap_handler->close();
#endif
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

int ha_mroonga::storage_close()
{
  MRN_DBUG_ENTER_METHOD();
  grn_obj_unlink(ctx, grn_table);
  free(grn_columns);
  DBUG_RETURN(0);
}

int ha_mroonga::close()
{
  int error = 0;
  THD *thd = ha_thd();
  MRN_DBUG_ENTER_METHOD();

  clear_indexes();

  if (share->wrapper_mode)
  {
    error = wrapper_close();
  } else {
    error = storage_close();
  }

  if (is_temporary_table_name(share->table_name)) {
    TABLE_LIST table_list;
    TABLE_SHARE *tmp_table_share;
    int tmp_error;
    /* no need to decode */
    mrn::PathMapper mapper(share->table_name, mrn_database_path_prefix);
#ifdef MRN_TABLE_LIST_INIT_REQUIRE_ALIAS
    table_list.init_one_table(mapper.db_name(), strlen(mapper.db_name()),
                              mapper.mysql_table_name(),
                              strlen(mapper.mysql_table_name()),
                              mapper.mysql_table_name(),
                              TL_WRITE);
#else
    table_list.init_one_table(mapper.db_name(), mapper.mysql_table_name(),
                              TL_WRITE);
#endif
    mrn_open_mutex_lock();
    tmp_table_share =
      mrn_create_tmp_table_share(&table_list, share->table_name, &tmp_error);
    mrn_open_mutex_unlock();
    if (!tmp_table_share) {
      error = tmp_error;
    } else if ((tmp_error = alter_share_add(share->table_name,
                                            tmp_table_share))) {
      error = tmp_error;
      mrn_open_mutex_lock();
      mrn_free_tmp_table_share(tmp_table_share);
      mrn_open_mutex_unlock();
    }
  }
  bitmap_free(&multiple_column_key_bitmap);
  mrn_free_share(share);
  share = NULL;
  is_clone = false;
  if (
    thd &&
    thd->lex &&
    thd->lex->sql_command == SQLCOM_FLUSH &&
    (thd->lex->type & REFRESH_TABLES)
  ) {
    /* flush tables */
    pthread_mutex_lock(&mrn_open_tables_mutex);
    if (!mrn_open_tables.records)
    {
      int tmp_error = close_databases();
      if (tmp_error)
        error = tmp_error;
    }
    pthread_mutex_unlock(&mrn_open_tables_mutex);
  }
  DBUG_RETURN(error);
}

int ha_mroonga::wrapper_delete_table(const char *name, MRN_SHARE *tmp_share,
                                     const char *table_name)
{
  int error = 0;
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
  hnd->init();
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
  int error = 0;
  MRN_DBUG_ENTER_METHOD();

  error = ensure_database_open(name);
  if (error)
    DBUG_RETURN(error);

  error = mrn_change_encoding(ctx, system_charset_info);
  if (error)
    DBUG_RETURN(error);

  TABLE_SHARE *tmp_table_share = tmp_share->table_share;

  uint i;
  for (i = 0; i < tmp_table_share->keys; i++) {
    mrn::IndexTableName index_table_name(table_name,
                                         tmp_table_share->key_info[i].name);
    grn_obj *index_table = grn_ctx_get(ctx,
                                       index_table_name.c_str(),
                                       index_table_name.length());
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

int ha_mroonga::storage_delete_table(const char *name, MRN_SHARE *tmp_share,
                                     const char *table_name)
{
  int error = 0;
  TABLE_SHARE *tmp_table_share = tmp_share->table_share;
  MRN_DBUG_ENTER_METHOD();

  error = ensure_database_open(name);
  if (error)
    DBUG_RETURN(error);

  error = mrn_change_encoding(ctx, system_charset_info);
  if (error)
    DBUG_RETURN(error);

  uint i;
  for (i = 0; i < tmp_table_share->keys; i++) {
    mrn::IndexTableName index_table_name(table_name,
                                         tmp_table_share->key_info[i].name);
    grn_obj *idx_table_obj = grn_ctx_get(ctx,
                                         index_table_name.c_str(),
                                         index_table_name.length());
    if (idx_table_obj != NULL) {
      grn_obj_remove(ctx, idx_table_obj);
    }
  }

  grn_obj *table_obj = grn_ctx_get(ctx, table_name, strlen(table_name));
  if (ctx->rc) {
    error = ER_CANT_OPEN_FILE;
    my_message(error, ctx->errbuf, MYF(0));
    DBUG_RETURN(error);
  }
  error = grn_obj_remove(ctx, table_obj);
  DBUG_RETURN(error);
}

int ha_mroonga::delete_table(const char *name)
{
  int error = 0;
  THD *thd = ha_thd();
  TABLE_LIST table_list;
  TABLE_SHARE *tmp_table_share = NULL;
  TABLE tmp_table;
  MRN_SHARE *tmp_share;
  st_mrn_alter_share *alter_share, *tmp_alter_share;
  MRN_DBUG_ENTER_METHOD();
  mrn::PathMapper mapper(name, mrn_database_path_prefix);
  st_mrn_slot_data *slot_data = mrn_get_slot_data(thd, false);
  if (slot_data && slot_data->first_alter_share)
  {
    tmp_alter_share = NULL;
    alter_share = slot_data->first_alter_share;
    while (alter_share)
    {
      if (!strcmp(alter_share->path, name))
      {
        /* found */
        tmp_table_share = alter_share->alter_share;
        if (tmp_alter_share)
          tmp_alter_share->next = alter_share->next;
        else
          slot_data->first_alter_share = alter_share->next;
        free(alter_share);
        break;
      }
      tmp_alter_share = alter_share;
      alter_share = alter_share->next;
    }
  }
  if (!tmp_table_share)
  {
    mrn::PathMapper mapper(name, mrn_database_path_prefix);
#ifdef MRN_TABLE_LIST_INIT_REQUIRE_ALIAS
    table_list.init_one_table(mapper.db_name(), strlen(mapper.db_name()),
                              mapper.mysql_table_name(),
                              strlen(mapper.mysql_table_name()),
                              mapper.mysql_table_name(),
                              TL_WRITE);
#else
    table_list.init_one_table(mapper.db_name(), mapper.mysql_table_name(),
                              TL_WRITE);
#endif
    mrn_open_mutex_lock();
    tmp_table_share = mrn_create_tmp_table_share(&table_list, name, &error);
    mrn_open_mutex_unlock();
    if (!tmp_table_share) {
      DBUG_RETURN(error);
    }
  }
  tmp_table.s = tmp_table_share;
#ifdef WITH_PARTITION_STORAGE_ENGINE
  tmp_table.part_info = NULL;
#endif
  if (!(tmp_share = mrn_get_share(name, &tmp_table, &error)))
  {
    mrn_open_mutex_lock();
    mrn_free_tmp_table_share(tmp_table_share);
    mrn_open_mutex_unlock();
    DBUG_RETURN(error);
  }

  if (tmp_share->wrapper_mode)
  {
    error = wrapper_delete_table(name, tmp_share, mapper.table_name());
  } else {
    error = storage_delete_table(name, tmp_share, mapper.table_name());
  }

  mrn_free_share(tmp_share);
  mrn_open_mutex_lock();
  mrn_free_tmp_table_share(tmp_table_share);
  mrn_open_mutex_unlock();
  if (is_temporary_table_name(name)) {
    mrn_drop_db(name);
  }
  DBUG_RETURN(error);
}

void ha_mroonga::wrapper_set_keys_in_use()
{
  uint i, j;
  MRN_DBUG_ENTER_METHOD();
  mrn::AutoIncrementValueLock lock_(table_share);
  table_share->keys_in_use.set_prefix(table_share->keys);
  share->disable_keys = FALSE;
  for (i = 0; i < table_share->keys; i++) {
    j = share->wrap_key_nr[i];
    if (j < MAX_KEY) {
      if (!share->wrap_table_share->keys_in_use.is_set(j)) {
        /* copy bitmap */
        table_share->keys_in_use.clear_bit(i);
        share->disable_keys = TRUE;
      }
    } else {
      if (!grn_index_tables[i]) {
        /* disabled */
        table_share->keys_in_use.clear_bit(i);
        share->disable_keys = TRUE;
      }
    }
  }
  table_share->keys_for_keyread.set_prefix(table_share->keys);
  table_share->keys_for_keyread.intersect(table_share->keys_in_use);
  DBUG_VOID_RETURN;
}

void ha_mroonga::storage_set_keys_in_use()
{
  uint i;
  MRN_DBUG_ENTER_METHOD();
  mrn::AutoIncrementValueLock lock_(table_share);
  table_share->keys_in_use.set_prefix(table_share->keys);
  share->disable_keys = FALSE;
  for (i = 0; i < table_share->keys; i++) {
    if (i == table_share->primary_key) {
      continue;
    }
    if (!grn_index_tables[i]) {
      /* disabled */
      table_share->keys_in_use.clear_bit(i);
      DBUG_PRINT("info", ("mroonga: key %u disabled", i));
      share->disable_keys = TRUE;
    }
  }
  table_share->keys_for_keyread.set_prefix(table_share->keys);
  table_share->keys_for_keyread.intersect(table_share->keys_in_use);
  DBUG_VOID_RETURN;
}

int ha_mroonga::wrapper_info(uint flag)
{
  int error = 0;
  MRN_DBUG_ENTER_METHOD();

  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  error = wrap_handler->info(flag);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  if (flag & HA_STATUS_ERRKEY) {
    errkey = wrap_handler->errkey;
    memcpy(dup_ref, wrap_handler->dup_ref, wrap_handler->ref_length);
  }
  if (flag & HA_STATUS_TIME) {
    stats.update_time = wrap_handler->stats.update_time;
  }
  if (flag & HA_STATUS_CONST) {
    stats.max_data_file_length = wrap_handler->stats.max_data_file_length;
    stats.create_time = wrap_handler->stats.create_time;
    stats.block_size = wrap_handler->stats.block_size;
    wrapper_set_keys_in_use();
  }
  if (flag & HA_STATUS_VARIABLE) {
    stats.data_file_length = wrap_handler->stats.data_file_length;
    stats.index_file_length = wrap_handler->stats.index_file_length;
    stats.records = wrap_handler->stats.records;
    stats.mean_rec_length = wrap_handler->stats.mean_rec_length;
    stats.check_time = wrap_handler->stats.check_time;
  }
  if (flag & HA_STATUS_AUTO) {
    stats.auto_increment_value = wrap_handler->stats.auto_increment_value;
  }
  DBUG_RETURN(error);
}

int ha_mroonga::storage_info(uint flag)
{
  MRN_DBUG_ENTER_METHOD();
  mrn_change_encoding(ctx, NULL);
  ha_rows rows = grn_table_size(ctx, grn_table);
  stats.records = rows;

  if (flag & (HA_STATUS_ERRKEY | HA_STATUS_NO_LOCK)) {
    errkey = dup_key;
  }

  if ((flag & HA_STATUS_AUTO) && table->found_next_number_field) {
    THD *thd = ha_thd();
    struct system_variables *variables = &thd->variables;
    ulonglong nb_reserved_values;
    bool next_number_field_is_null = !table->next_number_field;
    mrn::ExternalLock mrn_external_lock(ha_thd(), this,
                                        mrn_lock_type == F_UNLCK ?
                                        F_RDLCK : F_UNLCK);
    if (mrn_external_lock.error()) {
      DBUG_RETURN(mrn_external_lock.error());
    }
    if (next_number_field_is_null) {
      table->next_number_field = table->found_next_number_field;
    }
    storage_get_auto_increment(variables->auto_increment_offset,
                               variables->auto_increment_increment, 1,
                               &stats.auto_increment_value,
                               &nb_reserved_values);
    if (next_number_field_is_null) {
      table->next_number_field = NULL;
    }
  }

  if (flag & HA_STATUS_CONST) {
    storage_set_keys_in_use();
  }

  DBUG_RETURN(0);
}

int ha_mroonga::info(uint flag)
{
  MRN_DBUG_ENTER_METHOD();

  int error = 0;
  if (share->wrapper_mode)
  {
    error = wrapper_info(flag);
  } else {
    error = storage_info(flag);
  }
  DBUG_RETURN(error);
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
  DBUG_RETURN(lock_count);
}

uint ha_mroonga::storage_lock_count() const
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(1);
}

uint ha_mroonga::lock_count() const
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  if (share->wrapper_mode)
  {
    error = wrapper_lock_count();
  } else {
    error = storage_lock_count();
  }
  DBUG_RETURN(error);
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

THR_LOCK_DATA **ha_mroonga::storage_store_lock(THD *thd, THR_LOCK_DATA **to,
                                               enum thr_lock_type lock_type)
{
  MRN_DBUG_ENTER_METHOD();
  if (lock_type != TL_IGNORE && thr_lock_data.type == TL_UNLOCK &&
    !thd_in_lock_tables(thd)) {
    if (lock_type == TL_READ_NO_INSERT) {
      lock_type = TL_READ;
    } else if (lock_type >= TL_WRITE_CONCURRENT_INSERT &&
               lock_type <= TL_WRITE && !thd_tablespace_op(thd)) {
      lock_type = TL_WRITE_ALLOW_WRITE;
    }

    thr_lock_data.type = lock_type;
  }
  *to++ = &thr_lock_data;
  DBUG_RETURN(to);
}

THR_LOCK_DATA **ha_mroonga::store_lock(THD *thd, THR_LOCK_DATA **to,
                                       enum thr_lock_type lock_type)
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_PRINT("info", ("mroonga: lock_type=%s",
                      mrn_inspect_thr_lock_type(lock_type)));
  if (share->wrapper_mode)
    to = wrapper_store_lock(thd, to, lock_type);
  else
    to = storage_store_lock(thd, to, lock_type);
  DBUG_RETURN(to);
}

int ha_mroonga::wrapper_external_lock(THD *thd, int lock_type)
{
  int error = 0;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  error = wrap_handler->ha_external_lock(thd, lock_type);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::storage_external_lock(THD *thd, int lock_type)
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(0);
}

int ha_mroonga::external_lock(THD *thd, int lock_type)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  mrn_lock_type = lock_type;
  if (share->wrapper_mode)
  {
    error = wrapper_external_lock(thd, lock_type);
  } else {
    error = storage_external_lock(thd, lock_type);
  }
  DBUG_RETURN(error);
}

int ha_mroonga::wrapper_rnd_init(bool scan)
{
  int error = 0;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  error = wrap_handler->ha_rnd_init(scan);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::storage_rnd_init(bool scan)
{
  MRN_DBUG_ENTER_METHOD();
  mrn_change_encoding(ctx, NULL);
  cursor = grn_table_cursor_open(ctx, grn_table, NULL, 0, NULL, 0, 0, -1, 0);
  if (ctx->rc) {
    my_message(ER_ERROR_ON_READ, ctx->errbuf, MYF(0));
    DBUG_RETURN(ER_ERROR_ON_READ);
  }
  DBUG_RETURN(0);
}

int ha_mroonga::rnd_init(bool scan)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  if (share->wrapper_mode)
  {
    error = wrapper_rnd_init(scan);
  } else {
    error = storage_rnd_init(scan);
  }
  DBUG_RETURN(error);
}

int ha_mroonga::wrapper_rnd_end()
{
  int error = 0;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  error = wrap_handler->ha_rnd_end();
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::storage_rnd_end()
{
  MRN_DBUG_ENTER_METHOD();
  clear_cursor();
  DBUG_RETURN(0);
}

int ha_mroonga::rnd_end()
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  if (share->wrapper_mode)
  {
    error = wrapper_rnd_end();
  } else {
    error = storage_rnd_end();
  }
  DBUG_RETURN(error);
}

int ha_mroonga::wrapper_rnd_next(uchar *buf)
{
  int error = 0;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  if (fulltext_searching)
    set_pk_bitmap();
#ifdef MRN_HANDLER_HAVE_HA_RND_NEXT
  error = wrap_handler->ha_rnd_next(buf);
#else
  error = wrap_handler->rnd_next(buf);
#endif
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::storage_rnd_next(uchar *buf)
{
  MRN_DBUG_ENTER_METHOD();
  int error = storage_get_next_record(buf);
  DBUG_RETURN(error);
}

int ha_mroonga::rnd_next(uchar *buf)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  if (share->wrapper_mode)
  {
    error = wrapper_rnd_next(buf);
  } else {
    error = storage_rnd_next(buf);
  }
  DBUG_RETURN(error);
}

int ha_mroonga::wrapper_rnd_pos(uchar *buf, uchar *pos)
{
  int error = 0;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
#ifdef MRN_HANDLER_HAVE_HA_RND_POS
  error = wrap_handler->ha_rnd_pos(buf, pos);
#else
  error = wrap_handler->rnd_pos(buf, pos);
#endif
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::storage_rnd_pos(uchar *buf, uchar *pos)
{
  MRN_DBUG_ENTER_METHOD();
  record_id = *((grn_id*) pos);
  storage_store_fields(buf, record_id);
  DBUG_RETURN(0);
}

int ha_mroonga::rnd_pos(uchar *buf, uchar *pos)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  if (share->wrapper_mode)
  {
    error = wrapper_rnd_pos(buf, pos);
  } else {
    error = storage_rnd_pos(buf, pos);
  }
  DBUG_RETURN(error);
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

void ha_mroonga::storage_position(const uchar *record)
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
    storage_position(record);
  DBUG_VOID_RETURN;
}

int ha_mroonga::generic_extra(enum ha_extra_function operation)
{
  MRN_DBUG_ENTER_METHOD();
  switch (operation) {
  case HA_EXTRA_IGNORE_DUP_KEY:
    ignoring_duplicated_key = true;
    break;
  case HA_EXTRA_NO_IGNORE_DUP_KEY:
    ignoring_duplicated_key = false;
    break;
  case HA_EXTRA_WRITE_CAN_REPLACE:
    replacing_ = true;
    break;
  case HA_EXTRA_WRITE_CANNOT_REPLACE:
    replacing_ = false;
    break;
  case HA_EXTRA_INSERT_WITH_UPDATE:
    inserting_with_update = true;
    break;
  case HA_EXTRA_KEYREAD:
    ignoring_no_key_columns = true;
    break;
  case HA_EXTRA_NO_KEYREAD:
    ignoring_no_key_columns = false;
    break;
  default:
    break;
  }
  DBUG_RETURN(0);
}

int ha_mroonga::wrapper_extra(enum ha_extra_function operation)
{
  int error = 0;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  error = wrap_handler->extra(operation);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::storage_extra(enum ha_extra_function operation)
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(0);
}

int ha_mroonga::extra(enum ha_extra_function operation)
{
  int error = 0;
  MRN_DBUG_ENTER_METHOD();
  DBUG_PRINT("info",
             ("mroonga: this=%p; extra-operation=%s",
              this, mrn_inspect_extra_function(operation)));
  if (share->wrapper_mode)
  {
    if ((error = wrapper_extra(operation)))
      DBUG_RETURN(error);
  } else {
    if ((error = storage_extra(operation)))
      DBUG_RETURN(error);
  }
  error = generic_extra(operation);
  DBUG_RETURN(error);
}

int ha_mroonga::wrapper_extra_opt(enum ha_extra_function operation,
                                  ulong cache_size)
{
  int error = 0;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  error = wrap_handler->extra_opt(operation, cache_size);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::storage_extra_opt(enum ha_extra_function operation,
                                  ulong cache_size)
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(0);
}

int ha_mroonga::extra_opt(enum ha_extra_function operation, ulong cache_size)
{
  int error = 0;
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
  {
    if ((error = wrapper_extra_opt(operation, cache_size)))
      DBUG_RETURN(error);
  } else {
    if ((error = storage_extra_opt(operation, cache_size)))
      DBUG_RETURN(error);
  }
  error = generic_extra(operation);
  DBUG_RETURN(error);
}

bool ha_mroonga::wrapper_is_target_index(KEY *key_info)
{
  MRN_DBUG_ENTER_METHOD();
  bool target_index =
    (key_info->algorithm == HA_KEY_ALG_FULLTEXT) || mrn_is_geo_key(key_info);
  DBUG_PRINT("info", ("mroonga: %s", target_index ? "true" : "false"));
  DBUG_RETURN(target_index);
}

bool ha_mroonga::wrapper_have_target_index()
{
  MRN_DBUG_ENTER_METHOD();

  bool have_target_index = false;

  uint i;
  uint n_keys = table->s->keys;
  for (i = 0; i < n_keys; i++) {
    KEY key_info = table->key_info[i];

    if (wrapper_is_target_index(&key_info)) {
      have_target_index = true;
      break;
    }
  }

  DBUG_PRINT("info", ("mroonga: %s", have_target_index ? "true" : "false"));
  DBUG_RETURN(have_target_index);
}

int ha_mroonga::wrapper_write_row(uchar *buf)
{
  int error = 0;
  THD *thd = ha_thd();
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  tmp_disable_binlog(thd);
  error = wrap_handler->ha_write_row(buf);
  reenable_binlog(thd);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);

  if (!error && wrapper_have_target_index()) {
    error = wrapper_write_row_index(buf);
  }

  DBUG_RETURN(error);
}

int ha_mroonga::wrapper_write_row_index(uchar *buf)
{
  MRN_DBUG_ENTER_METHOD();

  int error = 0;

  if (is_dry_write()) {
    DBUG_PRINT("info", ("mroonga: dry write: ha_mroonga::%s", __FUNCTION__));
    DBUG_RETURN(error);
  }

  mrn_change_encoding(ctx, NULL);
  GRN_BULK_REWIND(&key_buffer);
  grn_bulk_space(ctx, &key_buffer, table->key_info->key_length);
  key_copy((uchar *)(GRN_TEXT_VALUE(&key_buffer)),
           buf,
           &(table->key_info[table_share->primary_key]),
           table->key_info[table_share->primary_key].key_length);

  int added;
  grn_id record_id;
  record_id = grn_table_add(ctx, grn_table,
                            GRN_TEXT_VALUE(&key_buffer),
                            GRN_TEXT_LEN(&key_buffer),
                            &added);
  if (record_id == GRN_ID_NIL) {
    DBUG_PRINT("info", ("mroonga: failed to add a new record into groonga"));
    char error_message[MRN_MESSAGE_BUFFER_SIZE];
    snprintf(error_message, MRN_MESSAGE_BUFFER_SIZE,
             "failed to add a new record into groonga: key=<%.*s>",
             (int)GRN_TEXT_LEN(&key_buffer),
             GRN_TEXT_VALUE(&key_buffer));
    error = ER_ERROR_ON_WRITE;
    push_warning(ha_thd(), Sql_condition::WARN_LEVEL_WARN, error,
                 error_message);
    DBUG_RETURN(0);
  }

  mrn::DebugColumnAccess debug_column_access(table, table->read_set);
  uint i;
  uint n_keys = table->s->keys;
  for (i = 0; i < n_keys; i++) {
    KEY key_info = table->key_info[i];

    if (!(wrapper_is_target_index(&key_info))) {
      continue;
    }

    grn_obj *index_column = grn_index_columns[i];
    if (!index_column) {
      continue;
    }

    uint j;
    for (j = 0; j < key_info.key_parts; j++) {
      Field *field = key_info.key_part[j].field;

      if (field->is_null())
        continue;

      error = mrn_change_encoding(ctx, field->charset());
      if (error)
        goto err;
      error = generic_store_bulk(field, &new_value_buffer);
      if (error) {
        my_message(error,
                   "mroonga: wrapper: "
                   "failed to get new value for updating index.",
                   MYF(0));
        goto err;
      }

      grn_rc rc;
      rc = grn_column_index_update(ctx, index_column, record_id, j + 1,
                                   NULL, &new_value_buffer);
      if (rc) {
        error = ER_ERROR_ON_WRITE;
        my_message(error, ctx->errbuf, MYF(0));
        goto err;
      }
    }
  }
err:

  DBUG_RETURN(error);
}

int ha_mroonga::storage_write_row(uchar *buf)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;

  if (is_dry_write()) {
    DBUG_PRINT("info", ("mroonga: dry write: ha_mroonga::%s", __FUNCTION__));
    DBUG_RETURN(error);
  }

  THD *thd = ha_thd();
  int i;
  uint j;
  int n_columns = table->s->fields;

  if (table->next_number_field && buf == table->record[0])
  {
    if ((error = update_auto_increment()))
      DBUG_RETURN(error);
  }

  mrn::DebugColumnAccess debug_column_access(table, table->read_set);
  if (thd->abort_on_warning) {
    for (i = 0; i < n_columns; i++) {
      Field *field = table->field[i];
      const char *column_name = field->field_name;
      int column_name_size = strlen(column_name);

      if (field->is_null()) continue;

      if (strncmp(MRN_COLUMN_NAME_ID, column_name, column_name_size) == 0) {
        my_message(ER_DATA_TOO_LONG, "cannot insert value to _id column", MYF(0));
        DBUG_RETURN(ER_DATA_TOO_LONG);
      }
      if (strncmp(MRN_COLUMN_NAME_SCORE, column_name, column_name_size) == 0) {
        my_message(ER_DATA_TOO_LONG, "cannot insert value to _score column", MYF(0));
        DBUG_RETURN(ER_DATA_TOO_LONG);
      }
    }
  }

  uchar *pkey = NULL;
  int pkey_size = 0;
  uint pkey_nr = table->s->primary_key;
  GRN_BULK_REWIND(&key_buffer);
  if (pkey_nr != MAX_INDEXES) {
    KEY key_info = table->key_info[pkey_nr];
    if (key_info.key_parts == 1) {
      Field *pkey_field = key_info.key_part[0].field;
      error = mrn_change_encoding(ctx, pkey_field->charset());
      if (error) {
        DBUG_RETURN(error);
      }
      generic_store_bulk(pkey_field, &key_buffer);
      pkey = (uchar *)(GRN_TEXT_VALUE(&key_buffer));
      pkey_size = GRN_TEXT_LEN(&key_buffer);
    } else {
      mrn_change_encoding(ctx, NULL);
      uchar key[MRN_MAX_KEY_SIZE];
      key_copy(key, buf, &key_info, key_info.key_length);
      grn_bulk_space(ctx, &key_buffer, key_info.key_length);
      pkey = (uchar *)(GRN_TEXT_VALUE(&key_buffer));
      storage_encode_multiple_column_key(&key_info,
                                         key, key_info.key_length,
                                         pkey, (uint *)&pkey_size,
                                         false);
    }
  }

  int added;
  record_id = grn_table_add(ctx, grn_table, pkey, pkey_size, &added);
  if (ctx->rc) {
    my_message(ER_ERROR_ON_WRITE, ctx->errbuf, MYF(0));
    DBUG_RETURN(ER_ERROR_ON_WRITE);
  }
  if (!added) {
    // duplicated error
    error = HA_ERR_FOUND_DUPP_KEY;
    memcpy(dup_ref, &record_id, sizeof(grn_id));
    dup_key = pkey_nr;
    if (!ignoring_duplicated_key) {
      GRN_LOG(ctx, GRN_LOG_ERROR, "duplicated _id on insert");
    }
    DBUG_RETURN(error);
  }

  st_mrn_slot_data *slot_data;
  if ((error = storage_write_row_unique_indexes(buf, record_id)))
  {
    goto err1;
  }

  grn_obj colbuf;
  GRN_VOID_INIT(&colbuf);
  for (i = 0; i < n_columns; i++) {
    Field *field = table->field[i];
    const char *column_name = field->field_name;
    int column_name_size = strlen(column_name);

    if (field->is_null()) continue;

    if (strncmp(MRN_COLUMN_NAME_ID, column_name, column_name_size) == 0) {
      push_warning(thd, Sql_condition::WARN_LEVEL_WARN, WARN_DATA_TRUNCATED,
                   "data truncated for _id column");
      continue;
    }

    if (strncmp(MRN_COLUMN_NAME_SCORE, column_name, column_name_size) == 0) {
      push_warning(thd, Sql_condition::WARN_LEVEL_WARN, WARN_DATA_TRUNCATED,
                   "data truncated for _score column");
      continue;
    }

    error = mrn_change_encoding(ctx, field->charset());
    if (error) {
      grn_obj_unlink(ctx, &colbuf);
      goto err2;
    }
    generic_store_bulk(field, &colbuf);
    if (added && is_grn_zero_column_value(grn_columns[i], &colbuf)) {
      // WORKAROUND: groonga can't index newly added '0' value for
      // fix size column. So we add non-'0' value first then add
      // real '0' value again. It will be removed when groonga
      // supports 'null' value.
      char *bytes = GRN_BULK_HEAD(&colbuf);
      bytes[0] = '\1';
      grn_obj_set_value(ctx, grn_columns[i], record_id, &colbuf, GRN_OBJ_SET);
      bytes[0] = '\0';
    }
    grn_obj_set_value(ctx, grn_columns[i], record_id, &colbuf, GRN_OBJ_SET);
    if (ctx->rc) {
      grn_obj_unlink(ctx, &colbuf);
      my_message(ER_ERROR_ON_WRITE, ctx->errbuf, MYF(0));
      error = ER_ERROR_ON_WRITE;
      goto err2;
    }
  }
  grn_obj_unlink(ctx, &colbuf);

  error = storage_write_row_multiple_column_indexes(buf, record_id);
  if (error) {
    goto err2;
  }

  // for UDF last_insert_grn_id()
  slot_data = mrn_get_slot_data(thd, true);
  if (slot_data == NULL) {
    error = HA_ERR_OUT_OF_MEM;
    goto err2;
  }
  slot_data->last_insert_record_id = record_id;

  DBUG_RETURN(0);

err2:
  for (j = 0; j < table->s->keys; j++) {
    if (j == pkey_nr) {
      continue;
    }
    KEY *key_info = &table->key_info[j];
    if (key_info->flags & HA_NOSAME) {
      grn_table_delete_by_id(ctx, grn_index_tables[j], key_id[j]);
    }
  }
err1:
  grn_table_delete_by_id(ctx, grn_table, record_id);
  DBUG_RETURN(error);
}

int ha_mroonga::storage_write_row_multiple_column_index(uchar *buf,
                                                        grn_id record_id,
                                                        KEY *key_info,
                                                        grn_obj *index_column)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;

  mrn_change_encoding(ctx, NULL);
  GRN_BULK_REWIND(&key_buffer);
  grn_bulk_space(ctx, &key_buffer, key_info->key_length);
  key_copy((uchar *)(GRN_TEXT_VALUE(&key_buffer)),
           buf,
           key_info,
           key_info->key_length);
  GRN_BULK_REWIND(&encoded_key_buffer);
  grn_bulk_space(ctx, &encoded_key_buffer, key_info->key_length);
  uint encoded_key_length;
  storage_encode_multiple_column_key(key_info,
                                     (uchar *)(GRN_TEXT_VALUE(&key_buffer)),
                                     key_info->key_length,
                                     (uchar *)(GRN_TEXT_VALUE(&encoded_key_buffer)),
                                     &encoded_key_length, false);

  grn_rc rc;
  rc = grn_column_index_update(ctx, index_column, record_id, 1, NULL,
                               &encoded_key_buffer);
  if (rc) {
    error = ER_ERROR_ON_WRITE;
    my_message(error, ctx->errbuf, MYF(0));
  }
  DBUG_RETURN(error);
}

int ha_mroonga::storage_write_row_multiple_column_indexes(uchar *buf,
                                                          grn_id record_id)
{
  MRN_DBUG_ENTER_METHOD();

  int error = 0;

  mrn::DebugColumnAccess debug_column_access(table, table->read_set);
  uint i;
  uint n_keys = table->s->keys;
  for (i = 0; i < n_keys; i++) {
    if (i == table->s->primary_key) {
      continue;
    }

    KEY key_info = table->key_info[i];

    if (key_info.key_parts == 1 || (key_info.flags & HA_FULLTEXT)) {
      continue;
    }

    grn_obj *index_column = grn_index_columns[i];
    if (!index_column) {
      continue;
    }

    if ((error = storage_write_row_multiple_column_index(buf,
                                                         record_id,
                                                         &key_info,
                                                         index_column)))
    {
      goto err;
    }
  }

err:

  DBUG_RETURN(error);
}

int ha_mroonga::storage_write_row_unique_index(uchar *buf, grn_id record_id,
                                               KEY *key_info,
                                               grn_obj *index_table,
                                               grn_id *key_id)
{
  void *ukey = NULL;
  int error, ukey_size = 0;
  MRN_DBUG_ENTER_METHOD();
  GRN_BULK_REWIND(&key_buffer);
  if (key_info->key_parts == 1) {
    Field *ukey_field = key_info->key_part[0].field;
    error = mrn_change_encoding(ctx, ukey_field->charset());
    if (error) {
      DBUG_RETURN(error);
    }
    generic_store_bulk(ukey_field, &key_buffer);
    ukey = GRN_TEXT_VALUE(&key_buffer);
    ukey_size = GRN_TEXT_LEN(&key_buffer);
  } else {
    mrn_change_encoding(ctx, NULL);
    uchar key[MRN_MAX_KEY_SIZE];
    key_copy(key, buf, key_info, key_info->key_length);
    grn_bulk_space(ctx, &key_buffer, key_info->key_length);
    ukey = GRN_TEXT_VALUE(&key_buffer);
    storage_encode_multiple_column_key(key_info,
                                       key, key_info->key_length,
                                       (uchar *)(ukey), (uint *)&ukey_size,
                                       false);
  }

  int added;
  *key_id = grn_table_add(ctx, index_table, ukey, ukey_size, &added);
  if (ctx->rc) {
    my_message(ER_ERROR_ON_WRITE, ctx->errbuf, MYF(0));
    DBUG_RETURN(ER_ERROR_ON_WRITE);
  }
  if (!added) {
    // duplicated error
    error = HA_ERR_FOUND_DUPP_KEY;
    memcpy(dup_ref, key_id, sizeof(grn_id));
    if (!ignoring_duplicated_key) {
      GRN_LOG(ctx, GRN_LOG_ERROR, "duplicated _id on insert");
    }
    DBUG_RETURN(error);
  }
  DBUG_RETURN(0);
}

int ha_mroonga::storage_write_row_unique_indexes(uchar *buf, grn_id record_id)
{
  int error = 0;
  uint i;
  uint n_keys = table->s->keys;
  MRN_DBUG_ENTER_METHOD();

  for (i = 0; i < n_keys; i++) {
    if (i == table->s->primary_key) {
      continue;
    }

    KEY *key_info = &table->key_info[i];

    if (!(key_info->flags & HA_NOSAME)) {
      continue;
    }

    grn_obj *index_table = grn_index_tables[i];
    if (!index_table) {
      continue;
    }

    if ((error = storage_write_row_unique_index(buf, record_id, key_info,
                                                index_table, &key_id[i])))
    {
      if (error == HA_ERR_FOUND_DUPP_KEY)
      {
        dup_key = i;
      }
      goto err;
    }
  }
  DBUG_RETURN(0);

err:
  if (i) {
    mrn_change_encoding(ctx, NULL);
    do {
      i--;
      KEY *key_info = &table->key_info[i];
      if (key_info->flags & HA_NOSAME) {
        grn_table_delete_by_id(ctx, grn_index_tables[i], key_id[i]);
      }
    } while (i);
  }
  DBUG_RETURN(error);
}

int ha_mroonga::write_row(uchar *buf)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  if (share->wrapper_mode)
  {
    error = wrapper_write_row(buf);
  } else {
    error = storage_write_row(buf);
  }
  DBUG_RETURN(error);
}

int ha_mroonga::wrapper_get_record_id(uchar *data, grn_id *record_id,
                                      const char *context)
{
  MRN_DBUG_ENTER_METHOD();

  int error = 0;

  grn_obj key;
  GRN_TEXT_INIT(&key, 0);

  mrn_change_encoding(ctx, NULL);
  grn_bulk_space(ctx, &key, table->key_info->key_length);
  key_copy((uchar *)(GRN_TEXT_VALUE(&key)),
           data,
           &(table->key_info[table_share->primary_key]),
           table->key_info[table_share->primary_key].key_length);

  *record_id = grn_table_get(ctx, grn_table,
                             GRN_TEXT_VALUE(&key), GRN_TEXT_LEN(&key));
  if (*record_id == GRN_ID_NIL) {
    DBUG_PRINT("info", ("mroonga: %s", context));
    char error_message[MRN_MESSAGE_BUFFER_SIZE];
    snprintf(error_message, MRN_MESSAGE_BUFFER_SIZE,
             "%s: key=<%.*s>",
             context, (int)GRN_TEXT_LEN(&key), GRN_TEXT_VALUE(&key));
    error = ER_ERROR_ON_WRITE;
    push_warning(ha_thd(), Sql_condition::WARN_LEVEL_WARN, error,
                 error_message);
  }
  grn_obj_unlink(ctx, &key);

  DBUG_RETURN(error);
}

int ha_mroonga::wrapper_update_row(const uchar *old_data, uchar *new_data)
{
  MRN_DBUG_ENTER_METHOD();

  int error = 0;
  THD *thd = ha_thd();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  tmp_disable_binlog(thd);
  error = wrap_handler->ha_update_row(old_data, new_data);
  reenable_binlog(thd);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);

  if (!error && wrapper_have_target_index()) {
    error = wrapper_update_row_index(old_data, new_data);
  }

  DBUG_RETURN(error);
}

int ha_mroonga::wrapper_update_row_index(const uchar *old_data, uchar *new_data)
{
  MRN_DBUG_ENTER_METHOD();

  int error = 0;

  if (is_dry_write()) {
    DBUG_PRINT("info", ("mroonga: dry write: ha_mroonga::%s", __FUNCTION__));
    DBUG_RETURN(error);
  }

  mrn_change_encoding(ctx, NULL);
  KEY key_info = table->key_info[table_share->primary_key];
  GRN_BULK_REWIND(&key_buffer);
  key_copy((uchar *)(GRN_TEXT_VALUE(&key_buffer)),
           new_data,
           &key_info, key_info.key_length);
  int added;
  grn_id new_record_id;
  new_record_id = grn_table_add(ctx, grn_table,
                                GRN_TEXT_VALUE(&key_buffer),
                                table->key_info->key_length,
                                &added);
  if (new_record_id == GRN_ID_NIL) {
    char error_message[MRN_MESSAGE_BUFFER_SIZE];
    snprintf(error_message, MRN_MESSAGE_BUFFER_SIZE,
             "failed to get new record ID for updating from groonga: key=<%.*s>",
             (int)GRN_TEXT_LEN(&key_buffer), GRN_TEXT_VALUE(&key_buffer));
    error = ER_ERROR_ON_WRITE;
    my_message(error, error_message, MYF(0));
    DBUG_RETURN(error);
  }

  grn_id old_record_id;
  my_ptrdiff_t ptr_diff = PTR_BYTE_DIFF(old_data, table->record[0]);
  for (uint j = 0; j < key_info.key_parts; j++) {
    Field *field = key_info.key_part[j].field;
    field->move_field_offset(ptr_diff);
  }
  error = wrapper_get_record_id((uchar *)old_data, &old_record_id,
                                "failed to get old record ID "
                                "for updating from groonga");
  for (uint j = 0; j < key_info.key_parts; j++) {
    Field *field = key_info.key_part[j].field;
    field->move_field_offset(-ptr_diff);
  }
  if (error) {
    DBUG_RETURN(0);
  }

  mrn::DebugColumnAccess debug_column_access(table, table->read_set);
  uint i;
  uint n_keys = table->s->keys;
  for (i = 0; i < n_keys; i++) {
    KEY key_info = table->key_info[i];

    if (!(wrapper_is_target_index(&key_info))) {
      continue;
    }

    grn_obj *index_column = grn_index_columns[i];

    uint j;
    for (j = 0; j < key_info.key_parts; j++) {
      Field *field = key_info.key_part[j].field;

      generic_store_bulk(field, &new_value_buffer);

      field->move_field_offset(ptr_diff);
      generic_store_bulk(field, &old_value_buffer);
      field->move_field_offset(-ptr_diff);

      grn_rc rc;
      if (old_record_id == new_record_id) {
        if (added) {
          rc = grn_column_index_update(ctx, index_column, old_record_id, j + 1,
                                       &old_value_buffer, NULL);
          if (!rc) {
            rc = grn_column_index_update(ctx, index_column, new_record_id, j + 1,
                                         NULL, &new_value_buffer);
          }
        } else {
          rc = grn_column_index_update(ctx, index_column, old_record_id, j + 1,
                                       &old_value_buffer, &new_value_buffer);
        }
      } else {
        rc = grn_column_index_update(ctx, index_column, old_record_id, j + 1,
                                     &old_value_buffer, NULL);
        if (!rc) {
          rc = grn_column_index_update(ctx, index_column, new_record_id, j + 1,
                                       NULL, &new_value_buffer);
        }
        if (!rc) {
          rc = grn_table_delete_by_id(ctx, grn_table, old_record_id);
        }
      }
      if (rc) {
        error = ER_ERROR_ON_WRITE;
        my_message(error, ctx->errbuf, MYF(0));
        goto err;
      }
    }
  }
err:

  DBUG_RETURN(error);
}

int ha_mroonga::storage_update_row(const uchar *old_data, uchar *new_data)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;

  if (is_dry_write()) {
    DBUG_PRINT("info", ("mroonga: dry write: ha_mroonga::%s", __FUNCTION__));
    DBUG_RETURN(error);
  }

  grn_obj colbuf;
  int i;
  uint j;
  int n_columns = table->s->fields;
  THD *thd = ha_thd();

  if (thd->abort_on_warning) {
    for (i = 0; i < n_columns; i++) {
      Field *field = table->field[i];
      const char *column_name = field->field_name;
      int column_name_size = strlen(column_name);

      if (bitmap_is_set(table->write_set, field->field_index)) {
        if (field->is_null()) continue;
        if (strncmp(MRN_COLUMN_NAME_ID, column_name, column_name_size) == 0) {
          my_message(ER_DATA_TOO_LONG, "cannot update value to _id column", MYF(0));
          DBUG_RETURN(ER_DATA_TOO_LONG);
        }
        if (strncmp(MRN_COLUMN_NAME_SCORE, column_name, column_name_size) == 0) {
          my_message(ER_DATA_TOO_LONG, "cannot update value to _score column", MYF(0));
          DBUG_RETURN(ER_DATA_TOO_LONG);
        }
      }
    }
  }

  KEY *pkey_info = NULL;
  storage_store_fields_for_prep_update(old_data, new_data, record_id);
  {
    mrn::DebugColumnAccess debug_column_access(table, table->read_set);
    if ((error = storage_prepare_delete_row_unique_indexes(old_data,
                                                           record_id))) {
      DBUG_RETURN(error);
    }
    if ((error = storage_update_row_unique_indexes(new_data, record_id)))
    {
      DBUG_RETURN(error);
    }
  }

  if (table->s->primary_key != MAX_INDEXES) {
    pkey_info = &(table->key_info[table->s->primary_key]);
  }
  GRN_VOID_INIT(&colbuf);
  for (i = 0; i < n_columns; i++) {
    Field *field = table->field[i];
    const char *column_name = field->field_name;
    int column_name_size = strlen(column_name);
    if (bitmap_is_set(table->write_set, field->field_index)) {
      mrn::DebugColumnAccess debug_column_access(table, table->read_set);
      DBUG_PRINT("info", ("mroonga: update column %d(%d)",i,field->field_index));

      if (field->is_null()) continue;

      if (strncmp(MRN_COLUMN_NAME_ID, column_name, column_name_size) == 0) {
        push_warning(thd, Sql_condition::WARN_LEVEL_WARN, WARN_DATA_TRUNCATED,
                     "data truncated for _id column");
        continue;
      }

      if (strncmp(MRN_COLUMN_NAME_SCORE, column_name, column_name_size) == 0) {
        push_warning(thd, Sql_condition::WARN_LEVEL_WARN, WARN_DATA_TRUNCATED,
                     "data truncated for _score column");
        continue;
      }
      error = mrn_change_encoding(ctx, field->charset());
      if (error)
        goto err;

      bool on_duplicate_key_update =
        (inserting_with_update && ignoring_duplicated_key);
      if (!on_duplicate_key_update && pkey_info) {
        bool have_pkey = false;
        for (j = 0; j < pkey_info->key_parts; j++) {
          Field *pkey_field = pkey_info->key_part[j].field;
          if (strcmp(pkey_field->field_name, column_name) == 0) {
            if (!replacing_) {
              char message[MRN_BUFFER_SIZE];
              snprintf(message, MRN_BUFFER_SIZE,
                       "data truncated for primary key column: <%s>",
                       column_name);
              push_warning(thd, Sql_condition::WARN_LEVEL_WARN,
                           WARN_DATA_TRUNCATED, message);
            }
            have_pkey = true;
          }
        }
        if (have_pkey) {
          continue;
        }
      }

      generic_store_bulk(field, &colbuf);
      grn_obj_set_value(ctx, grn_columns[i], record_id, &colbuf, GRN_OBJ_SET);
      if (ctx->rc) {
        grn_obj_unlink(ctx, &colbuf);
        my_message(ER_ERROR_ON_WRITE, ctx->errbuf, MYF(0));
        error = ER_ERROR_ON_WRITE;
        goto err;
      }
    }
  }
  grn_obj_unlink(ctx, &colbuf);

  if ((error = storage_update_row_index(old_data, new_data)))
  {
    goto err;
  }

  if ((error = storage_delete_row_unique_indexes()))
  {
    DBUG_RETURN(error);
  }

  DBUG_RETURN(0);

err:
  for (j = 0; j < table->s->keys; j++) {
    if (j == table->s->primary_key) {
      continue;
    }
    KEY *key_info = &table->key_info[j];
    if ((key_info->flags & HA_NOSAME) && key_id[j] != GRN_ID_NIL) {
      grn_table_delete_by_id(ctx, grn_index_tables[j], key_id[j]);
    }
  }
  DBUG_RETURN(error);
}

int ha_mroonga::storage_update_row_index(const uchar *old_data, uchar *new_data)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;

  grn_obj old_key, old_encoded_key, new_key, new_encoded_key;
  GRN_TEXT_INIT(&old_key, 0);
  GRN_TEXT_INIT(&old_encoded_key, 0);
  GRN_TEXT_INIT(&new_key, 0);
  GRN_TEXT_INIT(&new_encoded_key, 0);

  my_ptrdiff_t ptr_diff = PTR_BYTE_DIFF(old_data, table->record[0]);

  mrn::DebugColumnAccess debug_column_access(table, table->read_set);
  uint i;
  uint n_keys = table->s->keys;
  mrn_change_encoding(ctx, NULL);
  for (i = 0; i < n_keys; i++) {
    if (i == table->s->primary_key) {
      continue;
    }

    KEY key_info = table->key_info[i];

    if (key_info.key_parts == 1 || (key_info.flags & HA_FULLTEXT)) {
      continue;
    }

    GRN_BULK_REWIND(&old_key);
    grn_bulk_space(ctx, &old_key, key_info.key_length);
    for (uint j = 0; j < key_info.key_parts; j++) {
      Field *field = key_info.key_part[j].field;
      field->move_field_offset(ptr_diff);
    }
    key_copy((uchar *)(GRN_TEXT_VALUE(&old_key)),
             (uchar *)old_data,
             &key_info,
             key_info.key_length);
    for (uint j = 0; j < key_info.key_parts; j++) {
      Field *field = key_info.key_part[j].field;
      field->move_field_offset(-ptr_diff);
    }
    GRN_BULK_REWIND(&old_encoded_key);
    grn_bulk_space(ctx, &old_encoded_key, key_info.key_length);
    uint old_encoded_key_length;
    storage_encode_multiple_column_key(&key_info,
                                       (uchar *)(GRN_TEXT_VALUE(&old_key)),
                                       key_info.key_length,
                                       (uchar *)(GRN_TEXT_VALUE(&old_encoded_key)),
                                       &old_encoded_key_length,
                                       false);

    GRN_BULK_REWIND(&new_key);
    grn_bulk_space(ctx, &new_key, key_info.key_length);
    key_copy((uchar *)(GRN_TEXT_VALUE(&new_key)),
             (uchar *)new_data,
             &key_info,
             key_info.key_length);
    GRN_BULK_REWIND(&new_encoded_key);
    grn_bulk_space(ctx, &new_encoded_key, key_info.key_length);
    uint new_encoded_key_length;
    storage_encode_multiple_column_key(&key_info,
                                       (uchar *)(GRN_TEXT_VALUE(&new_key)),
                                       key_info.key_length,
                                       (uchar *)(GRN_TEXT_VALUE(&new_encoded_key)),
                                       &new_encoded_key_length,
                                       false);

    grn_obj *index_column = grn_index_columns[i];
    grn_rc rc;
    rc = grn_column_index_update(ctx, index_column, record_id, 1,
                                 &old_encoded_key, &new_encoded_key);
    if (rc) {
      error = ER_ERROR_ON_WRITE;
      my_message(error, ctx->errbuf, MYF(0));
      goto err;
    }
  }
err:
  grn_obj_unlink(ctx, &old_key);
  grn_obj_unlink(ctx, &old_encoded_key);
  grn_obj_unlink(ctx, &new_key);
  grn_obj_unlink(ctx, &new_encoded_key);

  DBUG_RETURN(error);
}

int ha_mroonga::storage_update_row_unique_indexes(uchar *new_data,
                                                  grn_id record_id)
{
  int error;
  uint i;
  uint n_keys = table->s->keys;
  MRN_DBUG_ENTER_METHOD();

  for (i = 0; i < n_keys; i++) {
    if (i == table->s->primary_key) {
      continue;
    }

    KEY *key_info = &table->key_info[i];
    if (!(key_info->flags & HA_NOSAME)) {
      continue;
    }

    grn_obj *index_table = grn_index_tables[i];
    if (!index_table) {
      key_id[i] = GRN_ID_NIL;
      del_key_id[i] = GRN_ID_NIL;
      continue;
    }

    if (
      key_info->key_parts == 1 &&
      !bitmap_is_set(table->write_set,
                     key_info->key_part[0].field->field_index)
    ) {
      /* no change */
      key_id[i] = GRN_ID_NIL;
      del_key_id[i] = GRN_ID_NIL;
      continue;
    }

    if ((error = storage_write_row_unique_index(new_data, record_id, key_info,
                                                index_table, &key_id[i])))
    {
      if (error == HA_ERR_FOUND_DUPP_KEY)
      {
        if (key_id[i] == del_key_id[i]) {
          /* no change */
          key_id[i] = GRN_ID_NIL;
          del_key_id[i] = GRN_ID_NIL;
          continue;
        }
        dup_key = i;
        DBUG_PRINT("info", ("mroonga: different key ID: %d record ID: %d,%d",
          i, key_id[i], del_key_id[i]));
      }
      goto err;
    }
  }
  DBUG_RETURN(0);

err:
  if (i) {
    mrn_change_encoding(ctx, NULL);
    do {
      i--;
      KEY *key_info = &table->key_info[i];
      if ((key_info->flags & HA_NOSAME) && key_id[i] != GRN_ID_NIL) {
        grn_table_delete_by_id(ctx, grn_index_tables[i], key_id[i]);
      }
    } while (i);
  }
  DBUG_RETURN(error);
}

int ha_mroonga::update_row(const uchar *old_data, uchar *new_data)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  if (share->wrapper_mode)
  {
    error = wrapper_update_row(old_data, new_data);
  } else {
    error = storage_update_row(old_data, new_data);
  }
  DBUG_RETURN(error);
}

int ha_mroonga::wrapper_delete_row(const uchar *buf)
{
  MRN_DBUG_ENTER_METHOD();

  int error = 0;
  THD *thd= ha_thd();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  tmp_disable_binlog(thd);
  error = wrap_handler->ha_delete_row(buf);
  reenable_binlog(thd);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);

  if (!error && wrapper_have_target_index()) {
    error = wrapper_delete_row_index(buf);
  }

  DBUG_RETURN(error);
}

int ha_mroonga::wrapper_delete_row_index(const uchar *buf)
{
  MRN_DBUG_ENTER_METHOD();

  int error = 0;

  if (is_dry_write()) {
    DBUG_PRINT("info", ("mroonga: dry write: ha_mroonga::%s", __FUNCTION__));
    DBUG_RETURN(error);
  }

  mrn_change_encoding(ctx, NULL);
  grn_id record_id;
  error = wrapper_get_record_id((uchar *)buf, &record_id,
                                "failed to get record ID "
                                "for deleting from groonga");
  if (error) {
    DBUG_RETURN(0);
  }

  mrn::DebugColumnAccess debug_column_access(table, table->read_set);
  uint i;
  uint n_keys = table->s->keys;
  for (i = 0; i < n_keys; i++) {
    KEY key_info = table->key_info[i];

    if (!(wrapper_is_target_index(&key_info))) {
      continue;
    }

    grn_obj *index_column = grn_index_columns[i];

    uint j;
    for (j = 0; j < key_info.key_parts; j++) {
      Field *field = key_info.key_part[j].field;

      if (field->is_null())
        continue;

      generic_store_bulk(field, &old_value_buffer);
      grn_rc rc;
      rc = grn_column_index_update(ctx, index_column, record_id, j + 1,
                                   &old_value_buffer, NULL);
      if (rc) {
        error = ER_ERROR_ON_WRITE;
        my_message(error, ctx->errbuf, MYF(0));
        goto err;
      }
    }
  }
err:
  grn_table_delete_by_id(ctx, grn_table, record_id);
  if (ctx->rc) {
    error = ER_ERROR_ON_WRITE;
    my_message(error, ctx->errbuf, MYF(0));
  }

  DBUG_RETURN(error);
}

int ha_mroonga::storage_delete_row(const uchar *buf)
{
  MRN_DBUG_ENTER_METHOD();
  int error;

  if (is_dry_write()) {
    DBUG_PRINT("info", ("mroonga: dry write: ha_mroonga::%s", __FUNCTION__));
    DBUG_RETURN(0);
  }

  storage_store_fields_for_prep_update(buf, NULL, record_id);
  if ((error = storage_prepare_delete_row_unique_indexes(buf, record_id))) {
    DBUG_RETURN(error);
  }
  mrn_change_encoding(ctx, NULL);
  grn_table_delete_by_id(ctx, grn_table, record_id);
  if (ctx->rc) {
    my_message(ER_ERROR_ON_WRITE, ctx->errbuf, MYF(0));
    DBUG_RETURN(ER_ERROR_ON_WRITE);
  }
  if (
    (error = storage_delete_row_index(buf)) ||
    (error = storage_delete_row_unique_indexes())
  ) {
    DBUG_RETURN(error);
  }
  DBUG_RETURN(0);
}

int ha_mroonga::storage_delete_row_index(const uchar *buf)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;

  grn_obj key, encoded_key;
  GRN_TEXT_INIT(&key, 0);
  GRN_TEXT_INIT(&encoded_key, 0);

  mrn::DebugColumnAccess debug_column_access(table, table->read_set);
  uint i;
  uint n_keys = table->s->keys;
  mrn_change_encoding(ctx, NULL);
  for (i = 0; i < n_keys; i++) {
    if (i == table->s->primary_key) {
      continue;
    }

    KEY key_info = table->key_info[i];

    if (key_info.key_parts == 1 || (key_info.flags & HA_FULLTEXT)) {
      continue;
    }

    GRN_BULK_REWIND(&key);
    grn_bulk_space(ctx, &key, key_info.key_length);
    key_copy((uchar *)(GRN_TEXT_VALUE(&key)),
             (uchar *)buf,
             &key_info,
             key_info.key_length);
    GRN_BULK_REWIND(&encoded_key);
    grn_bulk_space(ctx, &encoded_key, key_info.key_length);
    uint encoded_key_length;
    storage_encode_multiple_column_key(&key_info,
                                       (uchar *)(GRN_TEXT_VALUE(&key)),
                                       key_info.key_length,
                                       (uchar *)(GRN_TEXT_VALUE(&encoded_key)),
                                       &encoded_key_length,
                                       false);

    grn_obj *index_column = grn_index_columns[i];
    grn_rc rc;
    rc = grn_column_index_update(ctx, index_column, record_id, 1,
                                 &encoded_key, NULL);
    if (rc) {
      error = ER_ERROR_ON_WRITE;
      my_message(error, ctx->errbuf, MYF(0));
      goto err;
    }
  }
err:
  grn_obj_unlink(ctx, &encoded_key);
  grn_obj_unlink(ctx, &key);

  DBUG_RETURN(error);
}

int ha_mroonga::storage_delete_row_unique_index(grn_obj *index_table,
                                                grn_id del_key_id)
{
  MRN_DBUG_ENTER_METHOD();
  grn_rc rc = grn_table_delete_by_id(ctx, index_table, del_key_id);
  if (rc) {
    my_message(ER_ERROR_ON_WRITE, ctx->errbuf, MYF(0));
    DBUG_RETURN(ER_ERROR_ON_WRITE);
  }
  DBUG_RETURN(0);
}

int ha_mroonga::storage_delete_row_unique_indexes()
{
  int error = 0, tmp_error;
  uint i;
  uint n_keys = table->s->keys;
  MRN_DBUG_ENTER_METHOD();

  for (i = 0; i < n_keys; i++) {
    if (i == table->s->primary_key) {
      continue;
    }

    KEY *key_info = &table->key_info[i];
    if ((!(key_info->flags & HA_NOSAME)) || del_key_id[i] == GRN_ID_NIL) {
      continue;
    }

    grn_obj *index_table = grn_index_tables[i];
    if ((tmp_error = storage_delete_row_unique_index(index_table,
                                                     del_key_id[i])))
    {
      error = tmp_error;
    }
  }
  DBUG_RETURN(error);
}

int ha_mroonga::storage_prepare_delete_row_unique_index(const uchar *buf,
                                                        grn_id record_id,
                                                        KEY *key_info,
                                                        grn_obj *index_table,
                                                        grn_obj *index_column,
                                                        grn_id *del_key_id)
{
  const void *ukey = NULL;
  uint32 ukey_size = 0;
  MRN_DBUG_ENTER_METHOD();
  if (key_info->key_parts == 1) {
    ukey = grn_obj_get_value_(ctx, index_column, record_id, &ukey_size);
  } else {
    mrn_change_encoding(ctx, NULL);
    uchar key[MRN_MAX_KEY_SIZE];
    key_copy(key, (uchar *) buf, key_info, key_info->key_length);
    grn_bulk_space(ctx, &key_buffer, key_info->key_length);
    ukey = GRN_TEXT_VALUE(&key_buffer);
    storage_encode_multiple_column_key(key_info,
                                       key, key_info->key_length,
                                       (uchar *)ukey, (uint *)&ukey_size,
                                       false);
  }
  *del_key_id = grn_table_get(ctx, index_table, ukey, ukey_size);
  DBUG_RETURN(0);
}

int ha_mroonga::storage_prepare_delete_row_unique_indexes(const uchar *buf,
                                                          grn_id record_id)
{
  int error = 0, tmp_error;
  uint i;
  uint n_keys = table->s->keys;
  MRN_DBUG_ENTER_METHOD();

  for (i = 0; i < n_keys; i++) {
    if (i == table->s->primary_key) {
      continue;
    }

    KEY *key_info = &table->key_info[i];
    if (!(key_info->flags & HA_NOSAME)) {
      continue;
    }

    grn_obj *index_table = grn_index_tables[i];
    if (!index_table) {
      del_key_id[i] = GRN_ID_NIL;
      continue;
    }

    grn_obj *index_column;
    if (key_info->key_parts == 1) {
      index_column = grn_columns[key_info->key_part[0].field->field_index];
    } else {
      index_column = grn_index_columns[i];
    }
    if ((tmp_error = storage_prepare_delete_row_unique_index(buf, record_id,
                                                             key_info,
                                                             index_table,
                                                             index_column,
                                                             &del_key_id[i])))
    {
      error = tmp_error;
    }
  }
  DBUG_RETURN(error);
}

int ha_mroonga::delete_row(const uchar *buf)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  if (share->wrapper_mode)
  {
    error = wrapper_delete_row(buf);
  } else {
    error = storage_delete_row(buf);
  }
  DBUG_RETURN(error);
}

uint ha_mroonga::wrapper_max_supported_key_parts()
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(MAX_REF_PARTS);
}

uint ha_mroonga::storage_max_supported_key_parts()
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(1);
}

uint ha_mroonga::max_supported_key_parts()
{
  MRN_DBUG_ENTER_METHOD();
  uint parts;
  if (share->wrapper_mode)
  {
    parts = wrapper_max_supported_key_parts();
  } else {
    parts = storage_max_supported_key_parts();
  }
  DBUG_RETURN(parts);
}

ha_rows ha_mroonga::wrapper_records_in_range(uint key_nr, key_range *range_min,
                                             key_range *range_max)
{
  ha_rows row_count;
  MRN_DBUG_ENTER_METHOD();
  KEY key_info = table->s->key_info[key_nr];
  if (mrn_is_geo_key(&key_info)) {
    row_count = generic_records_in_range_geo(key_nr, range_min, range_max);
  } else {
    MRN_SET_WRAP_SHARE_KEY(share, table->s);
    MRN_SET_WRAP_TABLE_KEY(this, table);
    row_count = wrap_handler->records_in_range(key_nr, range_min, range_max);
    MRN_SET_BASE_SHARE_KEY(share, table->s);
    MRN_SET_BASE_TABLE_KEY(this, table);
  }
  DBUG_RETURN(row_count);
}

ha_rows ha_mroonga::storage_records_in_range(uint key_nr, key_range *range_min,
                                             key_range *range_max)
{
  MRN_DBUG_ENTER_METHOD();
  int flags = 0;
  uint size_min = 0, size_max = 0;
  ha_rows row_count = 0;
  uchar *key_min = NULL, *key_max = NULL;
  uchar key_min_entity[MRN_MAX_KEY_SIZE];
  uchar key_max_entity[MRN_MAX_KEY_SIZE];
  KEY key_info = table->s->key_info[key_nr];
  bool is_multiple_column_index = key_info.key_parts > 1;

  if (is_multiple_column_index) {
    mrn_change_encoding(ctx, NULL);
    if (range_min && range_max &&
        range_min->length == range_max->length &&
        memcmp(range_min->key, range_max->key, range_min->length) == 0) {
      flags |= GRN_CURSOR_PREFIX;
      key_min = key_min_entity;
      storage_encode_multiple_column_key(&key_info,
                                         range_min->key, range_min->length,
                                         key_min, &size_min,
                                         false);
    } else {
      if (range_min) {
        key_min = key_min_entity;
        storage_encode_multiple_column_key(&key_info,
                                           range_min->key, range_min->length,
                                           key_min, &size_min,
                                           false);
      }
      if (range_max) {
        key_max = key_max_entity;
        storage_encode_multiple_column_key(&key_info,
                                           range_max->key, range_max->length,
                                           key_max, &size_max,
                                           false);
      }
    }
  } else if (mrn_is_geo_key(&key_info)) {
    mrn_change_encoding(ctx, key_info.key_part->field->charset());
    row_count = generic_records_in_range_geo(key_nr, range_min, range_max);
    DBUG_RETURN(row_count);
  } else {
    KEY_PART_INFO key_part = key_info.key_part[0];
    Field *field = key_part.field;
    const char *column_name = field->field_name;
    int column_name_size = strlen(column_name);
    mrn_change_encoding(ctx, field->charset());

    if (strncmp(MRN_COLUMN_NAME_ID, column_name, column_name_size) == 0) {
      DBUG_RETURN((ha_rows)1);
    }

    if (range_min) {
      key_min = key_min_entity;
      storage_encode_key(field, range_min->key, key_min, &size_min);
    }
    if (range_max) {
      key_max = key_max_entity;
      storage_encode_key(field, range_max->key, key_max, &size_max);
    }
  }

  if (range_min && range_min->flag == HA_READ_AFTER_KEY) {
    flags |= GRN_CURSOR_GT;
  }
  if (range_max && range_max->flag == HA_READ_BEFORE_KEY) {
    flags |= GRN_CURSOR_LT;
  }

  uint pkey_nr = table->s->primary_key;
  if (key_nr == pkey_nr) {
    DBUG_PRINT("info", ("mroonga: use primary key"));
    grn_table_cursor *cursor;
    cursor = grn_table_cursor_open(ctx, grn_table,
                                   key_min, size_min,
                                   key_max, size_max,
                                   0, -1, flags);
    while (grn_table_cursor_next(ctx, cursor) != GRN_ID_NIL) {
      row_count++;
    }
    grn_table_cursor_close(ctx, cursor);
  } else {
    if (is_multiple_column_index) {
      DBUG_PRINT("info", ("mroonga: use multiple column key%u", key_nr));
    } else {
      DBUG_PRINT("info", ("mroonga: use key%u", key_nr));
    }
    uint table_size = grn_table_size(ctx, grn_table);
    uint cardinality = grn_table_size(ctx, grn_index_tables[key_nr]);
    grn_table_cursor *cursor;
    grn_table_cursor *index_cursor;

    cursor = grn_table_cursor_open(ctx, grn_index_tables[key_nr],
                                   key_min, size_min,
                                   key_max, size_max,
                                   0, -1, flags);
    index_cursor = grn_index_cursor_open(ctx, cursor,
                                         grn_index_columns[key_nr],
                                         0, GRN_ID_MAX, 0);
    while (grn_table_cursor_next(ctx, index_cursor) != GRN_ID_NIL) {
      row_count++;
    }
    grn_obj_unlink(ctx, index_cursor);
    grn_table_cursor_close(ctx, cursor);
    row_count = (int)(round((double)table_size * ((double)row_count / (double)cardinality)));
  }
  DBUG_RETURN(row_count);
}

ha_rows ha_mroonga::generic_records_in_range_geo(uint key_nr,
                                                 key_range *range_min,
                                                 key_range *range_max)
{
  MRN_DBUG_ENTER_METHOD();
  ha_rows row_count;
  int error;

  if (!range_min) {
    DBUG_PRINT("info",
               ("mroonga: range min is missing for geometry range search"));
    DBUG_RETURN(HA_POS_ERROR);
  }
  if (range_max) {
    DBUG_PRINT("info",
               ("mroonga: range max is specified for geometry range search"));
    DBUG_RETURN(HA_POS_ERROR);
  }
  error = mrn_change_encoding(ctx,
    table->key_info[key_nr].key_part->field->charset());
  if (error)
    DBUG_RETURN(error);
  if (!(range_min->flag & HA_READ_MBR_CONTAIN)) {
    push_warning_unsupported_spatial_index_search(range_min->flag);
    row_count = grn_table_size(ctx, grn_table);
    DBUG_RETURN(row_count);
  }

  geo_store_rectangle(range_min->key);
  row_count = grn_geo_estimate_in_rectangle(ctx,
                                            grn_index_columns[key_nr],
                                            &top_left_point,
                                            &bottom_right_point);
  DBUG_RETURN(row_count);
}

ha_rows ha_mroonga::records_in_range(uint key_nr, key_range *range_min, key_range *range_max)
{
  MRN_DBUG_ENTER_METHOD();
  ha_rows row_count = 0;
  if (share->wrapper_mode)
  {
    row_count = wrapper_records_in_range(key_nr, range_min, range_max);
  } else {
    row_count = storage_records_in_range(key_nr, range_min, range_max);
  }
  DBUG_PRINT("info", ("mroonga: row_count=%" MRN_HA_ROWS_FORMAT, row_count));
  DBUG_RETURN(row_count);
}

int ha_mroonga::wrapper_index_init(uint idx, bool sorted)
{
  int error = 0;
  KEY key_info = table->s->key_info[idx];
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  if (!mrn_is_geo_key(&key_info) && key_info.algorithm != HA_KEY_ALG_FULLTEXT)
  {
    error = wrap_handler->ha_index_init(share->wrap_key_nr[idx], sorted);
  } else {
    error = wrap_handler->ha_index_init(share->wrap_primary_key, sorted);
  }
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::storage_index_init(uint idx, bool sorted)
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(0);
}

int ha_mroonga::index_init(uint idx, bool sorted)
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_PRINT("info", ("mroonga: idx=%u", idx));
  active_index = idx;
  int error = 0;
  if (share->wrapper_mode)
  {
    error = wrapper_index_init(idx, sorted);
  } else {
    error = storage_index_init(idx, sorted);
  }
  DBUG_RETURN(error);
}

int ha_mroonga::wrapper_index_end()
{
  int error = 0;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  error = wrap_handler->ha_index_end();
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::storage_index_end()
{
  MRN_DBUG_ENTER_METHOD();
  clear_cursor();
  clear_cursor_geo();
  DBUG_RETURN(0);
}

int ha_mroonga::index_end()
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  if (share->wrapper_mode)
  {
    error = wrapper_index_end();
  } else {
    error = storage_index_end();
  }
  DBUG_RETURN(error);
}

int ha_mroonga::wrapper_index_read_map(uchar *buf, const uchar *key,
                                       key_part_map keypart_map,
                                       enum ha_rkey_function find_flag)
{
  int error = 0;
  MRN_DBUG_ENTER_METHOD();
  KEY key_info = table->key_info[active_index];
  if (mrn_is_geo_key(&key_info)) {
    clear_cursor();
    clear_cursor_geo();
    error = generic_geo_open_cursor(key, find_flag);
    if (!error) {
      error = wrapper_get_next_record(buf);
    }
    DBUG_RETURN(error);
  } else {
    MRN_SET_WRAP_SHARE_KEY(share, table->s);
    MRN_SET_WRAP_TABLE_KEY(this, table);
    if (fulltext_searching)
      set_pk_bitmap();
#ifdef MRN_HANDLER_HAVE_HA_INDEX_READ_MAP
    error = wrap_handler->ha_index_read_map(buf, key, keypart_map, find_flag);
#else
    error = wrap_handler->index_read_map(buf, key, keypart_map, find_flag);
#endif
    MRN_SET_BASE_SHARE_KEY(share, table->s);
    MRN_SET_BASE_TABLE_KEY(this, table);
  }
  DBUG_RETURN(error);
}

int ha_mroonga::storage_index_read_map(uchar *buf, const uchar *key,
                                       key_part_map keypart_map,
                                       enum ha_rkey_function find_flag)
{
  MRN_DBUG_ENTER_METHOD();
  check_count_skip(keypart_map, 0, false);

  int error = 0;

  uint key_nr = active_index;
  KEY key_info = table->key_info[key_nr];
  int flags = 0;
  uint size_min = 0, size_max = 0;
  uchar *key_min = NULL, *key_max = NULL;
  uchar key_min_entity[MRN_MAX_KEY_SIZE];
  uchar key_max_entity[MRN_MAX_KEY_SIZE];

  clear_cursor();
  clear_cursor_geo();

  bool is_multiple_column_index = key_info.key_parts > 1;
  if (is_multiple_column_index) {
    mrn_change_encoding(ctx, NULL);
    flags |= GRN_CURSOR_PREFIX;
    uint key_length = calculate_key_len(table, active_index, key, keypart_map);
    key_min = key_min_entity;
    storage_encode_multiple_column_key(&key_info,
                                       key, key_length,
                                       key_min, &size_min,
                                       false);
  } else if (mrn_is_geo_key(&key_info)) {
    error = mrn_change_encoding(ctx, key_info.key_part->field->charset());
    if (error)
      DBUG_RETURN(error);
    error = generic_geo_open_cursor(key, find_flag);
    if (!error) {
      error = storage_get_next_record(buf);
    }
    DBUG_RETURN(error);
  } else {
    KEY_PART_INFO key_part = key_info.key_part[0];
    Field *field = key_part.field;
    error = mrn_change_encoding(ctx, field->charset());
    if (error)
      DBUG_RETURN(error);

    if (find_flag == HA_READ_KEY_EXACT) {
      const char *column_name = field->field_name;
      int column_name_size = strlen(column_name);

      key_min = key_min_entity;
      key_max = key_min_entity;
      storage_encode_key(field, key, key_min, &size_min);
      size_max = size_min;
      // for _id
      if (strncmp(MRN_COLUMN_NAME_ID, column_name, column_name_size) == 0) {
        grn_id found_record_id = *((grn_id *)key_min);
        if (grn_table_at(ctx, grn_table, found_record_id) != GRN_ID_NIL) { // found
          storage_store_fields(buf, found_record_id);
          table->status = 0;
          record_id = found_record_id;
          DBUG_RETURN(0);
        } else {
          table->status = STATUS_NOT_FOUND;
          DBUG_RETURN(HA_ERR_END_OF_FILE);
        }
      }
    } else if (
      find_flag == HA_READ_BEFORE_KEY ||
      find_flag == HA_READ_PREFIX_LAST_OR_PREV
      ) {
      key_max = key_max_entity;
      storage_encode_key(field, key, key_max_entity, &size_max);
    } else {
      key_min = key_min_entity;
      storage_encode_key(field, key, key_min_entity, &size_min);
    }
  }

  switch (find_flag) {
  case HA_READ_BEFORE_KEY:
    flags |= GRN_CURSOR_LT;
    break;
  case HA_READ_AFTER_KEY:
    flags |= GRN_CURSOR_GT;
    break;
  default:
    break;
  }

  uint pkey_nr = table->s->primary_key;
  if (key_nr == pkey_nr) {
    DBUG_PRINT("info", ("mroonga: use primary key"));
    cursor = grn_table_cursor_open(ctx, grn_table,
                                   key_min, size_min,
                                   key_max, size_max,
                                   0, -1, flags);
  } else {
    if (is_multiple_column_index) {
      DBUG_PRINT("info", ("mroonga: use multiple column key%u", key_nr));
    } else {
      DBUG_PRINT("info", ("mroonga: use key%u", key_nr));
    }
    index_table_cursor = grn_table_cursor_open(ctx, grn_index_tables[key_nr],
                                               key_min, size_min,
                                               key_max, size_max,
                                               0, -1, flags);
    cursor = grn_index_cursor_open(ctx, index_table_cursor,
                                   grn_index_columns[key_nr],
                                   0, GRN_ID_MAX, 0);
  }
  if (ctx->rc) {
    my_message(ER_ERROR_ON_READ, ctx->errbuf, MYF(0));
    DBUG_RETURN(ER_ERROR_ON_READ);
  }
  error = storage_get_next_record(buf);
  DBUG_RETURN(error);
}

int ha_mroonga::index_read_map(uchar *buf, const uchar *key,
                               key_part_map keypart_map,
                               enum ha_rkey_function find_flag)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  if (share->wrapper_mode)
  {
    error = wrapper_index_read_map(buf, key, keypart_map, find_flag);
  } else {
    error = storage_index_read_map(buf, key, keypart_map, find_flag);
  }
  DBUG_PRINT("info", ("mroonga: error=%d", error));
  DBUG_RETURN(error);
}

#ifdef MRN_HANDLER_HAVE_INDEX_READ_LAST_MAP
int ha_mroonga::wrapper_index_read_last_map(uchar *buf, const uchar *key,
                                            key_part_map keypart_map)
{
  int error = 0;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  if (fulltext_searching)
    set_pk_bitmap();
  error = wrap_handler->index_read_last_map(buf, key, keypart_map);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::storage_index_read_last_map(uchar *buf, const uchar *key,
                                            key_part_map keypart_map)
{
  MRN_DBUG_ENTER_METHOD();
  uint key_nr = active_index;
  KEY key_info = table->key_info[key_nr];

  int flags = GRN_CURSOR_DESCENDING, error;
  uint size_min = 0, size_max = 0;
  uchar *key_min = NULL, *key_max = NULL;
  uchar key_min_entity[MRN_MAX_KEY_SIZE];

  clear_cursor();

  bool is_multiple_column_index = key_info.key_parts > 1;
  if (is_multiple_column_index) {
    mrn_change_encoding(ctx, NULL);
    flags |= GRN_CURSOR_PREFIX;
    uint key_length = calculate_key_len(table, active_index, key, keypart_map);
    key_min = key_min_entity;
    storage_encode_multiple_column_key(&key_info,
                                       key, key_length,
                                       key_min, &size_min,
                                       false);
  } else {
    KEY_PART_INFO key_part = key_info.key_part[0];
    Field *field = key_part.field;
    error = mrn_change_encoding(ctx, field->charset());
    if (error)
      DBUG_RETURN(error);

    key_min = key_min_entity;
    key_max = key_min_entity;
    storage_encode_key(field, key, key_min, &size_min);
    size_max = size_min;
  }

  uint pkey_nr = table->s->primary_key;
  if (key_nr == pkey_nr) {
    DBUG_PRINT("info", ("mroonga: use primary key"));
    cursor = grn_table_cursor_open(ctx, grn_table,
                                   key_min, size_min, key_max, size_max,
                                   0, -1, flags);
  } else {
    if (is_multiple_column_index) {
      DBUG_PRINT("info", ("mroonga: use multiple column key%u", key_nr));
    } else {
      DBUG_PRINT("info", ("mroonga: use key%u", key_nr));
    }
    index_table_cursor = grn_table_cursor_open(ctx, grn_index_tables[key_nr],
                                               key_min, size_min,
                                               key_max, size_max,
                                               0, -1, flags);
    cursor = grn_index_cursor_open(ctx, index_table_cursor,
                                   grn_index_columns[key_nr],
                                   0, GRN_ID_MAX, 0);
  }
  if (ctx->rc) {
    my_message(ER_ERROR_ON_READ, ctx->errbuf, MYF(0));
    DBUG_RETURN(ER_ERROR_ON_READ);
  }
  error = storage_get_next_record(buf);
  DBUG_RETURN(error);
}

int ha_mroonga::index_read_last_map(uchar *buf, const uchar *key,
                                    key_part_map keypart_map)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  if (share->wrapper_mode)
  {
    error = wrapper_index_read_last_map(buf, key, keypart_map);
  } else {
    error = storage_index_read_last_map(buf, key, keypart_map);
  }
  DBUG_RETURN(error);
}
#endif

int ha_mroonga::wrapper_index_next(uchar *buf)
{
  int error = 0;
  MRN_DBUG_ENTER_METHOD();
  KEY key_info = table->key_info[active_index];
  if (mrn_is_geo_key(&key_info)) {
    error = wrapper_get_next_record(buf);
  } else {
    MRN_SET_WRAP_SHARE_KEY(share, table->s);
    MRN_SET_WRAP_TABLE_KEY(this, table);
    if (fulltext_searching)
      set_pk_bitmap();
#ifdef MRN_HANDLER_HAVE_HA_INDEX_NEXT
    error = wrap_handler->ha_index_next(buf);
#else
    error = wrap_handler->index_next(buf);
#endif
    MRN_SET_BASE_SHARE_KEY(share, table->s);
    MRN_SET_BASE_TABLE_KEY(this, table);
  }
  DBUG_RETURN(error);
}

int ha_mroonga::storage_index_next(uchar *buf)
{
  MRN_DBUG_ENTER_METHOD();
  int error = storage_get_next_record(buf);
  DBUG_RETURN(error);
}

int ha_mroonga::index_next(uchar *buf)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  if (share->wrapper_mode)
  {
    error = wrapper_index_next(buf);
  } else {
    error = storage_index_next(buf);
  }
  DBUG_RETURN(error);
}

int ha_mroonga::wrapper_index_prev(uchar *buf)
{
  int error = 0;
  MRN_DBUG_ENTER_METHOD();
  KEY key_info = table->key_info[active_index];
  if (mrn_is_geo_key(&key_info)) {
    error = wrapper_get_next_record(buf);
  } else {
    MRN_SET_WRAP_SHARE_KEY(share, table->s);
    MRN_SET_WRAP_TABLE_KEY(this, table);
    if (fulltext_searching)
      set_pk_bitmap();
#ifdef MRN_HANDLER_HAVE_HA_INDEX_NEXT
    error = wrap_handler->ha_index_prev(buf);
#else
    error = wrap_handler->index_prev(buf);
#endif
    MRN_SET_BASE_SHARE_KEY(share, table->s);
    MRN_SET_BASE_TABLE_KEY(this, table);
  }
  DBUG_RETURN(error);
}

int ha_mroonga::storage_index_prev(uchar *buf)
{
  MRN_DBUG_ENTER_METHOD();
  int error = storage_get_next_record(buf);
  DBUG_RETURN(error);
}

int ha_mroonga::index_prev(uchar *buf)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  if (share->wrapper_mode)
  {
    error = wrapper_index_prev(buf);
  } else {
    error = storage_index_prev(buf);
  }
  DBUG_RETURN(error);
}

int ha_mroonga::wrapper_index_first(uchar *buf)
{
  int error = 0;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  if (fulltext_searching)
    set_pk_bitmap();
#ifdef MRN_HANDLER_HAVE_HA_INDEX_FIRST
  error = wrap_handler->ha_index_first(buf);
#else
  error = wrap_handler->index_first(buf);
#endif
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::storage_index_first(uchar *buf)
{
  MRN_DBUG_ENTER_METHOD();
  clear_cursor();
  int flags = GRN_CURSOR_ASCENDING;
  uint pkey_nr = table->s->primary_key;
  mrn_change_encoding(ctx, NULL);
  if (active_index == pkey_nr) {
    DBUG_PRINT("info", ("mroonga: use primary key"));
    cursor = grn_table_cursor_open(ctx, grn_table, NULL, 0, NULL, 0,
                                   0, -1, flags);
  } else {
    if (table->key_info[active_index].key_parts > 1) {
      DBUG_PRINT("info", ("mroonga: use multiple column key%u", active_index));
    } else {
      DBUG_PRINT("info", ("mroonga: use key%u", active_index));
    }
    index_table_cursor = grn_table_cursor_open(ctx,
                                               grn_index_tables[active_index],
                                               NULL, 0,
                                               NULL, 0,
                                               0, -1, flags);
    cursor = grn_index_cursor_open(ctx, index_table_cursor,
                                   grn_index_columns[active_index],
                                   0, GRN_ID_MAX, 0);
  }
  if (ctx->rc) {
    my_message(ER_ERROR_ON_READ, ctx->errbuf, MYF(0));
    DBUG_RETURN(ER_ERROR_ON_READ);
  }
  int error = storage_get_next_record(buf);
  DBUG_RETURN(error);
}

int ha_mroonga::index_first(uchar *buf)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  if (share->wrapper_mode)
  {
    error = wrapper_index_first(buf);
  } else {
    error = storage_index_first(buf);
  }
  DBUG_RETURN(error);
}

int ha_mroonga::wrapper_index_last(uchar *buf)
{
  int error = 0;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  if (fulltext_searching)
    set_pk_bitmap();
#ifdef MRN_HANDLER_HAVE_HA_INDEX_LAST
  error = wrap_handler->ha_index_last(buf);
#else
  error = wrap_handler->index_last(buf);
#endif
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::storage_index_last(uchar *buf)
{
  MRN_DBUG_ENTER_METHOD();
  clear_cursor();
  int flags = GRN_CURSOR_DESCENDING;
  uint pkey_nr = table->s->primary_key;
  mrn_change_encoding(ctx, NULL);
  if (active_index == pkey_nr) {
    DBUG_PRINT("info", ("mroonga: use primary key"));
    cursor = grn_table_cursor_open(ctx, grn_table, NULL, 0, NULL, 0,
                                   0, -1, flags);
  } else {
    if (table->key_info[active_index].key_parts > 1) {
      DBUG_PRINT("info", ("mroonga: use multiple column key%u", active_index));
    } else {
      DBUG_PRINT("info", ("mroonga: use key%u", active_index));
    }
    index_table_cursor = grn_table_cursor_open(ctx,
                                               grn_index_tables[active_index],
                                               NULL, 0,
                                               NULL, 0,
                                               0, -1, flags);
    cursor = grn_index_cursor_open(ctx, index_table_cursor,
                                   grn_index_columns[active_index],
                                   0, GRN_ID_MAX, 0);
  }
  if (ctx->rc) {
    my_message(ER_ERROR_ON_READ, ctx->errbuf, MYF(0));
    DBUG_RETURN(ER_ERROR_ON_READ);
  }
  int error = storage_get_next_record(buf);
  DBUG_RETURN(error);
}

int ha_mroonga::index_last(uchar *buf)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  if (share->wrapper_mode)
  {
    error = wrapper_index_last(buf);
  } else {
    error = storage_index_last(buf);
  }
  DBUG_RETURN(error);
}

int ha_mroonga::wrapper_index_next_same(uchar *buf, const uchar *key,
                                        uint keylen)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  KEY key_info = table->s->key_info[active_index];
  if (mrn_is_geo_key(&key_info)) {
    error = wrapper_get_next_record(buf);
  } else {
    MRN_SET_WRAP_SHARE_KEY(share, table->s);
    MRN_SET_WRAP_TABLE_KEY(this, table);
    if (fulltext_searching)
      set_pk_bitmap();
#ifdef MRN_HANDLER_HAVE_HA_INDEX_NEXT_SAME
    error = wrap_handler->ha_index_next_same(buf, key, keylen);
#else
    error = wrap_handler->index_next_same(buf, key, keylen);
#endif
    MRN_SET_BASE_SHARE_KEY(share, table->s);
    MRN_SET_BASE_TABLE_KEY(this, table);
  }
  DBUG_RETURN(error);
}

int ha_mroonga::storage_index_next_same(uchar *buf, const uchar *key,
                                        uint keylen)
{
  MRN_DBUG_ENTER_METHOD();
  int error = storage_get_next_record(count_skip ? NULL : buf);
  DBUG_RETURN(error);
}

int ha_mroonga::index_next_same(uchar *buf, const uchar *key, uint keylen)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  if (share->wrapper_mode)
  {
    error = wrapper_index_next_same(buf, key, keylen);
  } else {
    error = storage_index_next_same(buf, key, keylen);
  }
  DBUG_RETURN(error);
}

int ha_mroonga::wrapper_read_range_first(const key_range *start_key,
                                         const key_range *end_key,
                                         bool eq_range, bool sorted)
{
  int error = 0;
  MRN_DBUG_ENTER_METHOD();
  KEY key_info = table->key_info[active_index];
  if (mrn_is_geo_key(&key_info)) {
    clear_cursor();
    clear_cursor_geo();
    error = generic_geo_open_cursor(start_key->key, start_key->flag);
    if (!error) {
      error = wrapper_get_next_record(table->record[0]);
    }
    DBUG_RETURN(error);
  }
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  if (fulltext_searching)
    set_pk_bitmap();
  error = wrap_handler->read_range_first(start_key, end_key, eq_range,
                                            sorted);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::storage_read_range_first(const key_range *start_key,
                                         const key_range *end_key,
                                         bool eq_range, bool sorted)
{
  MRN_DBUG_ENTER_METHOD();
  check_count_skip(start_key ? start_key->keypart_map : 0,
                   end_key ? end_key->keypart_map : 0, false);
  int flags = 0, error;
  uint size_min = 0, size_max = 0;
  uchar *key_min = NULL, *key_max = NULL;
  uchar key_min_entity[MRN_MAX_KEY_SIZE];
  uchar key_max_entity[MRN_MAX_KEY_SIZE];
  KEY key_info = table->s->key_info[active_index];

  clear_cursor();

  bool is_multiple_column_index = key_info.key_parts > 1;
  if (is_multiple_column_index) {
    mrn_change_encoding(ctx, NULL);
    if (start_key && end_key &&
        start_key->length == end_key->length &&
        memcmp(start_key->key, end_key->key, start_key->length) == 0) {
      flags |= GRN_CURSOR_PREFIX;
      key_min = key_min_entity;
      storage_encode_multiple_column_key(&key_info,
                                         start_key->key, start_key->length,
                                         key_min, &size_min,
                                         false);
    } else {
      if (start_key) {
        key_min = key_min_entity;
        storage_encode_multiple_column_key(&key_info,
                                           start_key->key, start_key->length,
                                           key_min, &size_min,
                                           false);
      }
      if (end_key) {
        key_max = key_max_entity;
        storage_encode_multiple_column_key(&key_info,
                                           end_key->key, end_key->length,
                                           key_max, &size_max,
                                           false);
      }
    }
  } else {
    KEY_PART_INFO key_part = key_info.key_part[0];
    Field *field = key_part.field;
    const char *column_name = field->field_name;
    int column_name_size = strlen(column_name);
    error = mrn_change_encoding(ctx, field->charset());
    if (error)
      DBUG_RETURN(error);
    if (start_key) {
      key_min = key_min_entity;
      storage_encode_key(field, start_key->key, key_min_entity,
                         &size_min);
      if (start_key->flag == HA_READ_KEY_EXACT) {
        // for _id
        if (strncmp(MRN_COLUMN_NAME_ID, column_name, column_name_size) == 0) {
          grn_id found_record_id = *((grn_id *)key_min);
          if (grn_table_at(ctx, grn_table, found_record_id) != GRN_ID_NIL) { // found
            storage_store_fields(table->record[0], found_record_id);
            table->status = 0;
            cursor = NULL;
            record_id = found_record_id;
            DBUG_RETURN(0);
          } else {
            table->status = STATUS_NOT_FOUND;
            cursor = NULL;
            record_id = GRN_ID_NIL;
            DBUG_RETURN(HA_ERR_END_OF_FILE);
          }
        }
      }
    }
    if (end_key) {
      key_max = key_max_entity;
      storage_encode_key(field, end_key->key, key_max, &size_max);
    }
  }

  if (start_key && start_key->flag == HA_READ_AFTER_KEY) {
    flags |= GRN_CURSOR_GT;
  }
  if (end_key && end_key->flag == HA_READ_BEFORE_KEY) {
    flags |= GRN_CURSOR_LT;
  }

  uint pkey_nr = table->s->primary_key;
  if (active_index == pkey_nr) {
    DBUG_PRINT("info", ("mroonga: use primary key"));
    cursor = grn_table_cursor_open(ctx, grn_table,
                                   key_min, size_min, key_max, size_max,
                                   0, -1, flags);
  } else {
    if (is_multiple_column_index) {
      DBUG_PRINT("info", ("mroonga: use multiple column key%u", active_index));
    } else {
      DBUG_PRINT("info", ("mroonga: use key%u", active_index));
    }
    index_table_cursor = grn_table_cursor_open(ctx,
                                               grn_index_tables[active_index],
                                               key_min, size_min,
                                               key_max, size_max,
                                               0, -1, flags);
    cursor = grn_index_cursor_open(ctx, index_table_cursor,
                                   grn_index_columns[active_index],
                                   0, GRN_ID_MAX, 0);
  }
  if (ctx->rc) {
    my_message(ER_ERROR_ON_READ, ctx->errbuf, MYF(0));
    DBUG_RETURN(ER_ERROR_ON_READ);
  }
  error = storage_get_next_record(table->record[0]);
  DBUG_RETURN(error);
}

int ha_mroonga::read_range_first(const key_range *start_key,
                                 const key_range *end_key,
                                 bool eq_range, bool sorted)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  if (share->wrapper_mode)
  {
    error = wrapper_read_range_first(start_key, end_key, eq_range,
                                     sorted);
  } else {
    error = storage_read_range_first(start_key, end_key, eq_range, sorted);
  }
  DBUG_RETURN(error);
}

int ha_mroonga::wrapper_read_range_next()
{
  int error = 0;
  MRN_DBUG_ENTER_METHOD();
  KEY key_info = table->key_info[active_index];
  if (mrn_is_geo_key(&key_info)) {
    error = wrapper_get_next_record(table->record[0]);
    DBUG_RETURN(error);
  }
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  if (fulltext_searching)
    set_pk_bitmap();
  error = wrap_handler->read_range_next();
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::storage_read_range_next()
{
  MRN_DBUG_ENTER_METHOD();

  if (cursor == NULL) {
    DBUG_RETURN(HA_ERR_END_OF_FILE);
  }
  int error = storage_get_next_record(count_skip ? NULL : table->record[0]);

  DBUG_RETURN(error);
}

int ha_mroonga::read_range_next()
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  if (share->wrapper_mode)
  {
    error = wrapper_read_range_next();
  } else {
    error = storage_read_range_next();
  }
  DBUG_RETURN(error);
}

int ha_mroonga::generic_ft_init()
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  mrn_change_encoding(ctx, NULL);
  if (sorted_result) {
    cursor = grn_table_cursor_open(ctx, sorted_result,
                                   NULL, 0, NULL, 0,
                                   0, -1, 0);
  } else {
    cursor = grn_table_cursor_open(ctx, matched_record_keys, NULL, 0, NULL, 0, 0,
                                   -1, 0);
  }
  if (ctx->rc) {
    error = ER_ERROR_ON_READ;
    my_message(error, ctx->errbuf, MYF(0));
  } else {
    if (sorted_result) {
      if (grn_table->header.type == GRN_TABLE_NO_KEY) {
        id_accessor = grn_obj_column(ctx, sorted_result,
                                     MRN_COLUMN_NAME_ID,
                                     strlen(MRN_COLUMN_NAME_ID));
      } else {
        key_accessor = grn_obj_column(ctx, sorted_result,
                                      MRN_COLUMN_NAME_KEY,
                                      strlen(MRN_COLUMN_NAME_KEY));
      }
    } else {
      key_accessor = grn_obj_column(ctx, matched_record_keys,
                                    MRN_COLUMN_NAME_KEY,
                                    strlen(MRN_COLUMN_NAME_KEY));
    }
  }
  DBUG_RETURN(error);
}

int ha_mroonga::wrapper_ft_init()
{
  MRN_DBUG_ENTER_METHOD();
  int error = generic_ft_init();
  DBUG_RETURN(error);
}

int ha_mroonga::storage_ft_init()
{
  MRN_DBUG_ENTER_METHOD();
  int error = generic_ft_init();
  record_id = GRN_ID_NIL;
  DBUG_RETURN(error);
}

int ha_mroonga::ft_init()
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  if (share->wrapper_mode)
  {
    error = wrapper_ft_init();
  } else {
    error = storage_ft_init();
  }
  DBUG_RETURN(error);
}

void ha_mroonga::wrapper_ft_end()
{
  MRN_DBUG_ENTER_METHOD();
  handler::ft_end();
  DBUG_VOID_RETURN;
}

void ha_mroonga::storage_ft_end()
{
  MRN_DBUG_ENTER_METHOD();
  handler::ft_end();
  DBUG_VOID_RETURN;
}

void ha_mroonga::ft_end()
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
    wrapper_ft_end();
  else
    storage_ft_end();
  DBUG_VOID_RETURN;
}

void ha_mroonga::generic_ft_init_ext_add_conditions_fast_order_limit(
  struct st_mrn_ft_info *info, grn_obj *expression)
{
  MRN_DBUG_ENTER_METHOD();

  Item *where = table->pos_in_table_list->select_lex->where;

  if (where->type() != Item::COND_ITEM) {
    DBUG_VOID_RETURN;
  }

  grn_obj column_name, value;
  GRN_TEXT_INIT(&column_name, 0);
  GRN_VOID_INIT(&value);
  Item_cond *cond_item = (Item_cond *)where;
  List_iterator<Item> iterator(*((cond_item)->argument_list()));
  const Item *sub_item;
  while ((sub_item = iterator++)) {
    switch (sub_item->type()) {
    case Item::FUNC_ITEM:
      {
        const Item_func *func_item = (const Item_func *)sub_item;
        switch (func_item->functype()) {
        case Item_func::EQ_FUNC:
          {
            Item **arguments = func_item->arguments();
            Item *left_item = arguments[0];
            Item *right_item = arguments[1];
            if (left_item->type() == Item::FIELD_ITEM) {
              GRN_BULK_REWIND(&column_name);
              GRN_TEXT_PUTS(info->ctx, &column_name, left_item->name);
              grn_expr_append_const(info->ctx, expression, &column_name,
                                    GRN_OP_PUSH, 1);
              grn_expr_append_op(info->ctx, expression, GRN_OP_GET_VALUE, 1);
              grn_obj_reinit(info->ctx, &value, GRN_DB_INT64, 0);
              GRN_INT64_SET(info->ctx, &value, right_item->val_int());
              grn_expr_append_const(info->ctx, expression, &value,
                                    GRN_OP_PUSH, 1);
              grn_expr_append_op(info->ctx, expression, GRN_OP_EQUAL, 2);
              grn_expr_append_op(info->ctx, expression, GRN_OP_AND, 2);
            }
          }
          break;
        default:
          break;
        }
      }
      break;
    default:
      break;
    }
  }
  grn_obj_unlink(info->ctx, &column_name);
  grn_obj_unlink(info->ctx, &value);

  DBUG_VOID_RETURN;
}

struct st_mrn_ft_info *ha_mroonga::generic_ft_init_ext_select(uint flags,
                                                              uint key_nr,
                                                              String *key)
{
  MRN_DBUG_ENTER_METHOD();

  struct st_mrn_ft_info *info = new st_mrn_ft_info();
  info->mroonga = this;
  info->ctx = grn_ctx_open(0);
  grn_ctx_use(info->ctx, grn_ctx_db(ctx));
  info->table = grn_table;
  info->result = grn_table_create(info->ctx, NULL, 0, NULL,
                                  GRN_OBJ_TABLE_HASH_KEY | GRN_OBJ_WITH_SUBREC,
                                  grn_table, 0);
  info->score_column = grn_obj_column(info->ctx, info->result,
                                      MRN_COLUMN_NAME_SCORE,
                                      strlen(MRN_COLUMN_NAME_SCORE));
  GRN_TEXT_INIT(&(info->key), 0);
  grn_bulk_space(info->ctx, &(info->key), table->key_info->key_length);
  GRN_INT32_INIT(&(info->score), 0);
  info->active_index = key_nr;
  info->key_info = &(table->key_info[key_nr]);
  info->primary_key_info = &(table->key_info[table_share->primary_key]);

  grn_obj *index_column = grn_index_columns[key_nr];
  grn_obj *match_columns, *match_columns_variable;
  GRN_EXPR_CREATE_FOR_QUERY(info->ctx, info->table, match_columns,
                            match_columns_variable);
  grn_expr_append_obj(info->ctx, match_columns, index_column, GRN_OP_PUSH, 1);

  grn_obj *expression, *expression_variable;
  GRN_EXPR_CREATE_FOR_QUERY(info->ctx, info->table,
                            expression, expression_variable);

  grn_rc rc = GRN_SUCCESS;
  mrn_change_encoding(info->ctx,
                      table->key_info[key_nr].key_part->field->charset());
  if (flags & FT_BOOL) {
    const char *keyword, *keyword_original;
    uint keyword_length, keyword_length_original;
    grn_operator default_operator = GRN_OP_OR;
    keyword = keyword_original = key->ptr();
    keyword_length = keyword_length_original = key->length();
    // WORKAROUND: support only '*D+', '*D-' and '*DOR' pragma.
    if (keyword_length > 0) {
      if (keyword[0] == '*' && keyword_length > 1) {
        switch (keyword[1]) {
        case 'D':
          if (keyword_length > 2 && keyword[2] == '+') {
            default_operator = GRN_OP_AND;
            keyword += 3;
            keyword_length -= 3;
          } else if (keyword_length > 2 && keyword[2] == '-') {
            default_operator = GRN_OP_OR;
            keyword += 3;
            keyword_length -= 3;
          } else if (keyword_length > 3 && memcmp(keyword + 2, "OR", 2) == 0) {
            default_operator = GRN_OP_OR;
            keyword += 4;
            keyword_length -= 4;
          }
          break;
        default:
          break;
        }
      }
    }
    // WORKAROUND: ignore the first '+' to support "+apple macintosh" pattern.
    while (keyword_length > 0 && keyword[0] == ' ') {
      keyword++;
      keyword_length--;
    }
    if (keyword_length > 0 && keyword[0] == '+') {
      keyword++;
      keyword_length--;
    }
    grn_expr_flags expression_flags =
      GRN_EXPR_SYNTAX_QUERY | GRN_EXPR_ALLOW_LEADING_NOT;
    rc = grn_expr_parse(info->ctx, expression,
                        keyword, keyword_length,
                        match_columns, GRN_OP_MATCH, default_operator,
                        expression_flags);
    if (rc) {
      char error_message[MRN_MESSAGE_BUFFER_SIZE];
      snprintf(error_message, MRN_MESSAGE_BUFFER_SIZE,
               "failed to parse fulltext search keyword: <%.*s>: <%s>",
               keyword_length_original, keyword_original,
               info->ctx->errbuf);
      my_message(ER_PARSE_ERROR, error_message, MYF(0));
      GRN_LOG(info->ctx, GRN_LOG_ERROR, "%s", error_message);
    }
  } else {
    grn_obj query;
    GRN_TEXT_INIT(&query, GRN_OBJ_DO_SHALLOW_COPY);
    GRN_TEXT_SET(info->ctx, &query, key->ptr(), key->length());
    grn_expr_append_obj(info->ctx, expression, match_columns, GRN_OP_PUSH, 1);
    grn_expr_append_const(info->ctx, expression, &query, GRN_OP_PUSH, 1);
    grn_expr_append_op(info->ctx, expression, GRN_OP_SIMILAR, 2);
    grn_obj_unlink(info->ctx, &query);
  }

  if (rc == GRN_SUCCESS) {
    if (fast_order_limit) {
      generic_ft_init_ext_add_conditions_fast_order_limit(info, expression);
    }
    longlong escalation_threshold = THDVAR(ha_thd(), match_escalation_threshold);
    mrn::MatchEscalationThresholdScope scope(info->ctx, escalation_threshold);
    grn_table_select(info->ctx, info->table, expression,
                     info->result, GRN_OP_OR);
  }

  grn_obj_unlink(info->ctx, expression);
  grn_obj_unlink(info->ctx, match_columns);

  DBUG_RETURN(info);
}

FT_INFO *ha_mroonga::generic_ft_init_ext(uint flags, uint key_nr, String *key)
{
  MRN_DBUG_ENTER_METHOD();
  COND *select_cond = get_select_cond();
  current_ft_item = get_tgt_ft_item(current_ft_item);
  if (select_cond && !check_ft_in_where_item_type(select_cond, current_ft_item))
  {
    DBUG_PRINT("info", ("mroonga: key[%u] '%s' is not used for where clause",
      key_nr, key->c_ptr_safe()));
    struct st_mrn_ft_info *info =
      generic_ft_init_ext_select(flags, key_nr, key);
    DBUG_RETURN((FT_INFO *)info);
  }

  check_count_skip(0, 0, true);

  clear_cursor();

  mrn_change_encoding(ctx, system_charset_info);
  grn_operator operation = GRN_OP_AND;
  if (!matched_record_keys) {
    matched_record_keys = grn_table_create(ctx, NULL, 0, NULL,
                                           GRN_OBJ_TABLE_HASH_KEY | GRN_OBJ_WITH_SUBREC,
                                           grn_table, 0);
    operation = GRN_OP_OR;
  }

  grn_table_sort_key *sort_keys = NULL;
  int n_sort_keys = 0;
  longlong limit = -1;
  check_fast_order_limit(&sort_keys, &n_sort_keys, &limit);

  struct st_mrn_ft_info *info =
    generic_ft_init_ext_select(flags, key_nr, key);

  grn_rc rc;
  rc = grn_table_setoperation(ctx, matched_record_keys, info->result,
                              matched_record_keys, operation);
  if (rc) {
    char error_message[MRN_MESSAGE_BUFFER_SIZE];
    snprintf(error_message, MRN_MESSAGE_BUFFER_SIZE,
             "failed to merge matched record keys: <%s>",
             ctx->errbuf);
    my_message(ER_ERROR_ON_READ, error_message, MYF(0));
    GRN_LOG(ctx, GRN_LOG_ERROR, "%s", error_message);
  }
  if (fast_order_limit) {
    sorted_result = grn_table_create(ctx, NULL,
                                     0, NULL,
                                     GRN_OBJ_TABLE_NO_KEY, NULL,
                                     matched_record_keys);
    grn_table_sort(ctx, matched_record_keys, 0, limit, sorted_result,
                   sort_keys, n_sort_keys);
  }
  if (sort_keys) {
    for (int i = 0; i < n_sort_keys; i++) {
      grn_obj_unlink(info->ctx, sort_keys[i].key);
    }
    free(sort_keys);
  }

  DBUG_RETURN((FT_INFO *)info);
}

FT_INFO *ha_mroonga::wrapper_ft_init_ext(uint flags, uint key_nr, String *key)
{
  MRN_DBUG_ENTER_METHOD();
  FT_INFO *info = generic_ft_init_ext(flags, key_nr, key);
  struct st_mrn_ft_info *mrn_ft_info = (struct st_mrn_ft_info *)info;
  mrn_ft_info->please = &mrn_wrapper_ft_vft;
  DBUG_RETURN(info);
}

FT_INFO *ha_mroonga::storage_ft_init_ext(uint flags, uint key_nr, String *key)
{
  MRN_DBUG_ENTER_METHOD();
  FT_INFO *info = generic_ft_init_ext(flags, key_nr, key);
  struct st_mrn_ft_info *mrn_ft_info = (struct st_mrn_ft_info *)info;
  mrn_ft_info->please = &mrn_storage_ft_vft;
  DBUG_RETURN(info);
}

FT_INFO *ha_mroonga::ft_init_ext(uint flags, uint key_nr, String *key)
{
  MRN_DBUG_ENTER_METHOD();
  fulltext_searching = true;
  FT_INFO *info;
  if (key_nr == NO_SUCH_KEY) {
    struct st_mrn_ft_info *mrn_ft_info = new st_mrn_ft_info();
    mrn_ft_info->please = &mrn_no_such_key_ft_vft;
    info = (FT_INFO *)mrn_ft_info;
  } else {
    if (share->wrapper_mode)
    {
      info = wrapper_ft_init_ext(flags, key_nr, key);
    } else {
      info = storage_ft_init_ext(flags, key_nr, key);
    }
  }
  DBUG_RETURN(info);
}

int ha_mroonga::wrapper_ft_read(uchar *buf)
{
  MRN_DBUG_ENTER_METHOD();
  int error = wrapper_get_next_record(buf);
  DBUG_RETURN(error);
}

int ha_mroonga::storage_ft_read(uchar *buf)
{
  MRN_DBUG_ENTER_METHOD();
  grn_id found_record_id;
  mrn_change_encoding(ctx, NULL);

  found_record_id = grn_table_cursor_next(ctx, cursor);
  if (ctx->rc) {
    my_message(ER_ERROR_ON_READ, ctx->errbuf, MYF(0));
    DBUG_RETURN(ER_ERROR_ON_READ);
  }

  if (found_record_id == GRN_ID_NIL) {
    table->status = STATUS_NOT_FOUND;
    DBUG_RETURN(HA_ERR_END_OF_FILE);
  }
  table->status = 0;

  if (count_skip && record_id != GRN_ID_NIL) {
    DBUG_RETURN(0);
  }

  GRN_BULK_REWIND(&key_buffer);
  if (id_accessor) {
    grn_obj id_buffer;
    GRN_RECORD_INIT(&id_buffer, 0, grn_obj_id(ctx, grn_table));
    grn_obj_get_value(ctx, id_accessor, found_record_id, &id_buffer);
    record_id = GRN_RECORD_VALUE(&id_buffer);
  } else if (key_accessor) {
    grn_obj_get_value(ctx, key_accessor, found_record_id, &key_buffer);
    record_id = grn_table_get(ctx, grn_table,
                              GRN_TEXT_VALUE(&key_buffer),
                              GRN_TEXT_LEN(&key_buffer));
  } else {
    void *key;
    grn_table_cursor_get_key(ctx, cursor, &key);
    if (ctx->rc) {
      record_id = GRN_ID_NIL;
      my_message(ER_ERROR_ON_READ, ctx->errbuf, MYF(0));
      DBUG_RETURN(ER_ERROR_ON_READ);
    } else {
      record_id = *((grn_id *)key);
    }
  }
  storage_store_fields(buf, record_id);
  DBUG_RETURN(0);
}

int ha_mroonga::ft_read(uchar *buf)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  if (share->wrapper_mode)
  {
    error = wrapper_ft_read(buf);
  } else {
    error = storage_ft_read(buf);
  }
  DBUG_RETURN(error);
}

const Item *ha_mroonga::wrapper_cond_push(const Item *cond)
{
  const Item *reminder_cond;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  reminder_cond = wrap_handler->cond_push(cond);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(reminder_cond);
}

const Item *ha_mroonga::storage_cond_push(const Item *cond)
{
  MRN_DBUG_ENTER_METHOD();
  const Item *reminder_cond = cond;
  if (!pushed_cond && is_groonga_layer_condition(cond)) {
    reminder_cond = NULL;
  }
  DBUG_RETURN(reminder_cond);
}

const Item *ha_mroonga::cond_push(const Item *cond)
{
  MRN_DBUG_ENTER_METHOD();
  const Item *reminder_cond;
  if (share->wrapper_mode)
  {
    reminder_cond = wrapper_cond_push(cond);
  } else {
    reminder_cond = storage_cond_push(cond);
  }
  DBUG_RETURN(reminder_cond);
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

void ha_mroonga::storage_cond_pop()
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
    storage_cond_pop();
  DBUG_VOID_RETURN;
}

bool ha_mroonga::wrapper_get_error_message(int error, String *buf)
{
  bool temporary_error;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  temporary_error = wrap_handler->get_error_message(error, buf);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(temporary_error);
}

bool ha_mroonga::storage_get_error_message(int error, String *buf)
{
  MRN_DBUG_ENTER_METHOD();
  bool temporary_error = false;
  // latest error message
  buf->copy(ctx->errbuf, (uint) strlen(ctx->errbuf), system_charset_info);
  DBUG_RETURN(temporary_error);
}

bool ha_mroonga::get_error_message(int error, String *buf)
{
  MRN_DBUG_ENTER_METHOD();
  bool temporary_error;
  if (share && share->wrapper_mode)
  {
    temporary_error = wrapper_get_error_message(error, buf);
  } else {
    temporary_error = storage_get_error_message(error, buf);
  }
  DBUG_RETURN(temporary_error);
}

void ha_mroonga::mkdir_p(const char *directory)
{
  MRN_DBUG_ENTER_METHOD();

  int i = 0;
  char sub_directory[MRN_MAX_PATH_SIZE];
  sub_directory[0] = '\0';
  while (true) {
    if (directory[i] == '/' || directory[i] == '\0') {
      sub_directory[i] = '\0';
      struct stat directory_status;
      if (stat(sub_directory, &directory_status) != 0) {
        DBUG_PRINT("info", ("mroonga: creating directory: <%s>", sub_directory));
        GRN_LOG(ctx, GRN_LOG_INFO, "creating directory: <%s>", sub_directory);
        if (MRN_MKDIR(sub_directory, S_IRWXU) == 0) {
          DBUG_PRINT("info",
                     ("mroonga: created directory: <%s>", sub_directory));
          GRN_LOG(ctx, GRN_LOG_INFO, "created directory: <%s>", sub_directory);
        } else{
          DBUG_PRINT("error",
                     ("mroonga: failed to create directory: <%s>: <%s>",
                      sub_directory, strerror(errno)));
          GRN_LOG(ctx, GRN_LOG_ERROR,
                  "failed to create directory: <%s>: <%s>",
                  sub_directory, strerror(errno));
          DBUG_VOID_RETURN;
        }
      }
    }

    if (directory[i] == '\0') {
      break;
    }

    sub_directory[i] = directory[i];
    ++i;
  }

  DBUG_VOID_RETURN;
}

void ha_mroonga::push_warning_unsupported_spatial_index_search(enum ha_rkey_function flag)
{
  char search_name[MRN_BUFFER_SIZE];
  if (flag == HA_READ_MBR_INTERSECT) {
    strcpy(search_name, "intersect");
  } else if (flag == HA_READ_MBR_WITHIN) {
    strcpy(search_name, "within");
  } else if (flag & HA_READ_MBR_DISJOINT) {
    strcpy(search_name, "disjoint");
  } else if (flag & HA_READ_MBR_EQUAL) {
    strcpy(search_name, "equal");
  } else {
    sprintf(search_name, "unknown: %d", flag);
  }
  push_warning_printf(ha_thd(),
                      Sql_condition::WARN_LEVEL_WARN,
                      ER_UNSUPPORTED_EXTENSION,
                      "spatial index search "
                      "except MBRContains aren't supported: <%s>",
                      search_name);
}

void ha_mroonga::clear_cursor()
{
  MRN_DBUG_ENTER_METHOD();
  if (id_accessor) {
    grn_obj_unlink(ctx, id_accessor);
    id_accessor = NULL;
  }
  if (key_accessor) {
    grn_obj_unlink(ctx, key_accessor);
    key_accessor = NULL;
  }
  if (cursor) {
    grn_obj_unlink(ctx, cursor);
    cursor = NULL;
  }
  if (index_table_cursor) {
    grn_table_cursor_close(ctx, index_table_cursor);
    index_table_cursor = NULL;
  }
  DBUG_VOID_RETURN;
}

void ha_mroonga::clear_cursor_geo()
{
  MRN_DBUG_ENTER_METHOD();
  if (cursor_geo) {
    grn_obj_unlink(ctx, cursor_geo);
    cursor_geo = NULL;
  }
  DBUG_VOID_RETURN;
}

void ha_mroonga::clear_search_result()
{
  MRN_DBUG_ENTER_METHOD();
  clear_cursor();
  if (sorted_result) {
    grn_obj_unlink(ctx, sorted_result);
    sorted_result = NULL;
  }
  if (matched_record_keys) {
    grn_obj_unlink(ctx, matched_record_keys);
    matched_record_keys = NULL;
  }
  DBUG_VOID_RETURN;
}

void ha_mroonga::clear_search_result_geo()
{
  MRN_DBUG_ENTER_METHOD();
  clear_cursor_geo();
  if (grn_source_column_geo) {
    grn_obj_unlink(ctx, grn_source_column_geo);
    grn_source_column_geo = NULL;
  }
  DBUG_VOID_RETURN;
}

void ha_mroonga::clear_indexes()
{
  MRN_DBUG_ENTER_METHOD();
  uint n_keys = table->s->keys;
  uint pkey_nr = table->s->primary_key;

  for (uint i = 0; i < n_keys; i++) {
    if (i != pkey_nr) {
      if (grn_index_tables) {
        grn_obj_unlink(ctx, grn_index_tables[i]);
      }
      if (grn_index_columns) {
        grn_obj_unlink(ctx, grn_index_columns[i]);
      }
    }
  }

  if (grn_index_tables) {
    free(grn_index_tables);
    grn_index_tables = NULL;
  }

  if (grn_index_columns) {
    free(grn_index_columns);
    grn_index_columns = NULL;
  }

  if (key_id) {
    free(key_id);
    key_id = NULL;
  }

  if (del_key_id) {
    free(del_key_id);
    del_key_id = NULL;
  }

  DBUG_VOID_RETURN;
}

int ha_mroonga::alter_share_add(const char *path, TABLE_SHARE *table_share)
{
  MRN_DBUG_ENTER_METHOD();
  st_mrn_slot_data *slot_data = mrn_get_slot_data(ha_thd(), true);
  if (!slot_data)
    DBUG_RETURN(HA_ERR_OUT_OF_MEM);
  st_mrn_alter_share *alter_share =
    (st_mrn_alter_share *)malloc(sizeof(st_mrn_alter_share));
  if (!alter_share)
    DBUG_RETURN(HA_ERR_OUT_OF_MEM);
  alter_share->next = NULL;
  strcpy(alter_share->path, path);
  alter_share->alter_share = table_share;
  if (slot_data->first_alter_share)
  {
    st_mrn_alter_share *tmp_alter_share = slot_data->first_alter_share;
    while (tmp_alter_share->next)
      tmp_alter_share = tmp_alter_share->next;
    tmp_alter_share->next = alter_share;
  } else {
    slot_data->first_alter_share = alter_share;
  }
  DBUG_RETURN(0);
}

void ha_mroonga::remove_related_files(const char *base_path)
{
  MRN_DBUG_ENTER_METHOD();

  const char *base_directory_name = ".";
  size_t base_path_length = strlen(base_path);
#ifdef WIN32
  WIN32_FIND_DATA data;
  HANDLE finder = FindFirstFile(base_directory_name, &data);
  if (finder != INVALID_HANDLE_VALUE) {
    do {
      if (!(data.dwFileAttributes & FILE_ATTRIBUTE_NORMAL)) {
        continue;
      }
      if (strncmp(data.cFileName, base_path, base_path_length) == 0) {
        unlink(data.cFileName);
      }
    } while (FindNextFile(finder, &data) != 0);
    FindClose(finder);
  }
#else
  DIR *dir = opendir(base_directory_name);
  if (dir) {
    while (struct dirent *entry = readdir(dir)) {
      struct stat file_status;
      if (stat(entry->d_name, &file_status) != 0) {
        continue;
      }
      if (!((file_status.st_mode & S_IFMT) && S_IFREG)) {
        continue;
      }
      if (strncmp(entry->d_name, base_path, base_path_length) == 0) {
        unlink(entry->d_name);
      }
    }
    closedir(dir);
  }
#endif

  DBUG_VOID_RETURN;
}

void ha_mroonga::remove_grn_obj_force(const char *name)
{
  MRN_DBUG_ENTER_METHOD();

  grn_obj *obj = grn_ctx_get(ctx, name, strlen(name));
  if (obj) {
    grn_obj_remove(ctx, obj);
  } else {
    grn_obj *db = grn_ctx_db(ctx);
    grn_id id = grn_table_get(ctx, db, name, strlen(name));
    if (id) {
      char path[MRN_MAX_PATH_SIZE];
      grn_obj_delete_by_id(ctx, db, id, GRN_TRUE);
      if (grn_obj_path_by_id(ctx, db, id, path) == GRN_SUCCESS) {
        remove_related_files(path);
      }
    }
  }

  DBUG_VOID_RETURN;
}

grn_obj *ha_mroonga::find_tokenizer(const char *name, int name_length)
{
  MRN_DBUG_ENTER_METHOD();
  grn_obj *tokenizer;
  mrn_change_encoding(ctx, system_charset_info);
  tokenizer = grn_ctx_get(ctx, name, name_length);
  if (!tokenizer) {
    char message[MRN_BUFFER_SIZE];
    sprintf(message,
            "specified fulltext parser <%.*s> doesn't exist. "
            "default fulltext parser <%s> is used instead.",
            name_length, name,
            MRN_PARSER_DEFAULT);
    push_warning(ha_thd(),
                 Sql_condition::WARN_LEVEL_WARN, ER_UNSUPPORTED_EXTENSION,
                 message);
    tokenizer = grn_ctx_get(ctx,
                            MRN_PARSER_DEFAULT,
                            strlen(MRN_PARSER_DEFAULT));
  }
  if (!tokenizer) {
    push_warning(ha_thd(),
                 Sql_condition::WARN_LEVEL_WARN, ER_UNSUPPORTED_EXTENSION,
                 "couldn't find fulltext parser. "
                 "Bigram fulltext parser is used instead.");
    tokenizer = grn_ctx_at(ctx, GRN_DB_BIGRAM);
  }
  DBUG_RETURN(tokenizer);
}

int ha_mroonga::wrapper_get_next_record(uchar *buf)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  mrn_change_encoding(ctx, NULL);
  do {
    GRN_BULK_REWIND(&key_buffer);
    if (cursor_geo) {
      grn_id found_record_id;
      grn_posting *posting;
      posting = grn_geo_cursor_next(ctx, cursor_geo);
      if (!posting) {
        error = HA_ERR_END_OF_FILE;
        clear_cursor_geo();
        break;
      }
      found_record_id = posting->rid;
      grn_table_get_key(ctx, grn_table, found_record_id,
                        GRN_TEXT_VALUE(&key_buffer),
                        table->key_info->key_length);
    } else if (cursor) {
      grn_id found_record_id;
      found_record_id = grn_table_cursor_next(ctx, cursor);
      if (found_record_id == GRN_ID_NIL) {
        error = HA_ERR_END_OF_FILE;
        clear_cursor();
        break;
      }
      if (key_accessor) {
        grn_obj_get_value(ctx, key_accessor, found_record_id, &key_buffer);
      } else {
        void *key;
        int key_length;
        key_length = grn_table_cursor_get_key(ctx, cursor, &key);
        GRN_TEXT_SET(ctx, &key_buffer, key, key_length);
      }
    } else {
      error = HA_ERR_END_OF_FILE;
      break;
    }

    MRN_SET_WRAP_SHARE_KEY(share, table->s);
    MRN_SET_WRAP_TABLE_KEY(this, table);
    if (wrap_handler->inited == NONE)
    {
#ifdef MRN_HANDLER_HAVE_HA_INDEX_READ_IDX_MAP
      error = wrap_handler->ha_index_read_idx_map(buf,
                                                  share->wrap_primary_key,
                                                  (uchar *)GRN_TEXT_VALUE(
                                                    &key_buffer),
                                                  pk_keypart_map,
                                                  HA_READ_KEY_EXACT);
#else
      error = wrap_handler->index_read_idx_map(buf,
                                               share->wrap_primary_key,
                                               (uchar *)GRN_TEXT_VALUE(
                                                 &key_buffer),
                                               pk_keypart_map,
                                               HA_READ_KEY_EXACT);
#endif
    } else {
#ifdef MRN_HANDLER_HAVE_HA_INDEX_READ_MAP
      error = wrap_handler->ha_index_read_map(buf,
                                              (uchar *)GRN_TEXT_VALUE(
                                                &key_buffer),
                                              pk_keypart_map,
                                              HA_READ_KEY_EXACT);
#else
      error = wrap_handler->index_read_map(buf,
                                           (uchar *)GRN_TEXT_VALUE(
                                             &key_buffer),
                                           pk_keypart_map,
                                           HA_READ_KEY_EXACT);
#endif
    }
    MRN_SET_BASE_SHARE_KEY(share, table->s);
    MRN_SET_BASE_TABLE_KEY(this, table);
  } while (error == HA_ERR_END_OF_FILE || error == HA_ERR_KEY_NOT_FOUND);
  DBUG_RETURN(error);
}

int ha_mroonga::storage_get_next_record(uchar *buf)
{
  MRN_DBUG_ENTER_METHOD();
  if (cursor_geo) {
    grn_posting *posting;
    posting = grn_geo_cursor_next(ctx, cursor_geo);
    if (posting) {
      record_id = posting->rid;
    } else {
      record_id = GRN_ID_NIL;
    }
  } else if (cursor) {
    record_id = grn_table_cursor_next(ctx, cursor);
  } else {
    record_id = GRN_ID_NIL;
  }
  if (ctx->rc) {
    int error = ER_ERROR_ON_READ;
    my_message(error, ctx->errbuf, MYF(0));
    DBUG_RETURN(error);
  }
  if (record_id == GRN_ID_NIL) {
    DBUG_PRINT("info", ("mroonga: storage_get_next_record: end-of-file"));
    table->status = STATUS_NOT_FOUND;
    DBUG_RETURN(HA_ERR_END_OF_FILE);
  }
  if (buf) {
    if (ignoring_no_key_columns)
      storage_store_fields_by_index(buf);
    else
      storage_store_fields(buf, record_id);
    if (cursor_geo && grn_source_column_geo) {
      int latitude, longitude;
      GRN_GEO_POINT_VALUE(&source_point, latitude, longitude);
      double latitude_in_degree = GRN_GEO_MSEC2DEGREE(latitude);
      double longitude_in_degree = GRN_GEO_MSEC2DEGREE(longitude);
      if (!((bottom_right_latitude_in_degree <= latitude_in_degree &&
             latitude_in_degree <= top_left_latitude_in_degree) &&
            (top_left_longitude_in_degree <= longitude_in_degree &&
             longitude_in_degree <= bottom_right_longitude_in_degree))) {
        DBUG_PRINT("info",
                   ("mroonga: remove not contained geo point: "
                    "<%g,%g>(<%d,%d>); key: <%g,%g>(<%d,%d>), <%g,%g>(<%d,%d>)",
                    latitude_in_degree, longitude_in_degree,
                    latitude, longitude,
                    top_left_latitude_in_degree, top_left_longitude_in_degree,
                    GRN_GEO_DEGREE2MSEC(top_left_latitude_in_degree),
                    GRN_GEO_DEGREE2MSEC(top_left_longitude_in_degree),
                    bottom_right_latitude_in_degree,
                    bottom_right_longitude_in_degree,
                    GRN_GEO_DEGREE2MSEC(bottom_right_latitude_in_degree),
                    GRN_GEO_DEGREE2MSEC(bottom_right_longitude_in_degree)));
        int error = storage_get_next_record(buf);
        DBUG_RETURN(error);
      }
    }
  }
  table->status = 0;
  DBUG_RETURN(0);
}

void ha_mroonga::geo_store_rectangle(const uchar *rectangle)
{
  MRN_DBUG_ENTER_METHOD();

  double locations[4];
  for (int i = 0; i < 4; i++) {
    uchar reversed_value[8];
    for (int j = 0; j < 8; j++) {
      reversed_value[j] = (rectangle + (8 * i))[7 - j];
    }
    mi_float8get(locations[i], reversed_value);
  }
  top_left_longitude_in_degree = locations[0];
  bottom_right_longitude_in_degree = locations[1];
  bottom_right_latitude_in_degree = locations[2];
  top_left_latitude_in_degree = locations[3];
  int top_left_latitude = GRN_GEO_DEGREE2MSEC(top_left_latitude_in_degree);
  int top_left_longitude = GRN_GEO_DEGREE2MSEC(top_left_longitude_in_degree);
  int bottom_right_latitude = GRN_GEO_DEGREE2MSEC(bottom_right_latitude_in_degree);
  int bottom_right_longitude = GRN_GEO_DEGREE2MSEC(bottom_right_longitude_in_degree);
  GRN_GEO_POINT_SET(ctx, &top_left_point,
                    top_left_latitude, top_left_longitude);
  GRN_GEO_POINT_SET(ctx, &bottom_right_point,
                    bottom_right_latitude, bottom_right_longitude);

  DBUG_VOID_RETURN;
}

int ha_mroonga::generic_geo_open_cursor(const uchar *key,
                                        enum ha_rkey_function find_flag)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  int flags = 0;
  if (find_flag & HA_READ_MBR_CONTAIN) {
    grn_obj *index = grn_index_columns[active_index];
    geo_store_rectangle(key);
    cursor_geo = grn_geo_cursor_open_in_rectangle(ctx,
                                                  index,
                                                  &top_left_point,
                                                  &bottom_right_point,
                                                  0, -1);
    if (cursor_geo) {
      if (grn_source_column_geo) {
        grn_obj_unlink(ctx, grn_source_column_geo);
      }
      grn_obj sources;
      GRN_OBJ_INIT(&sources, GRN_BULK, 0, GRN_ID_NIL);
      grn_obj_get_info(ctx, index, GRN_INFO_SOURCE, &sources);
      grn_source_column_geo = grn_ctx_at(ctx, GRN_RECORD_VALUE(&sources));
      grn_obj_unlink(ctx, &sources);
    }
  } else {
    push_warning_unsupported_spatial_index_search(find_flag);
    cursor = grn_table_cursor_open(ctx, grn_table, NULL, 0, NULL, 0,
                                   0, -1, flags);
  }
  if (ctx->rc) {
    error = ER_ERROR_ON_READ;
    my_message(error, ctx->errbuf, MYF(0));
  }
  DBUG_RETURN(error);
}

bool ha_mroonga::is_dry_write()
{
  MRN_DBUG_ENTER_METHOD();
  bool dry_write_p = THDVAR(ha_thd(), dry_write);
  DBUG_RETURN(dry_write_p);
}

bool ha_mroonga::is_enable_optimization()
{
  MRN_DBUG_ENTER_METHOD();
  bool enable_optimization_p = THDVAR(ha_thd(), enable_optimization);
  DBUG_RETURN(enable_optimization_p);
}

bool ha_mroonga::is_need_normalize(Field *field) const
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_PRINT("info",
             ("mroonga: result_type = %u", field->result_type()));
  DBUG_PRINT("info",
             ("mroonga: charset->name = %s", field->charset()->name));
  DBUG_PRINT("info",
             ("mroonga: charset->csname = %s", field->charset()->csname));
  DBUG_PRINT("info",
             ("mroonga: charset->state = %u", field->charset()->state));
  bool need_normalize_p = false;
  if (field->charset()->state & (MY_CS_BINSORT | MY_CS_CSSORT)) {
    need_normalize_p = false;
    DBUG_PRINT("info", ("mroonga: is_need_normalize: false: sort is required"));
  } else {
    grn_builtin_type grn_type;
    grn_type = mrn_grn_type_from_field(ctx, field, true);
    if (GRN_DB_SHORT_TEXT <= grn_type && grn_type <= GRN_DB_LONG_TEXT) {
      need_normalize_p = true;
      DBUG_PRINT("info", ("mroonga: is_need_normalize: true: text type"));
    } else {
      need_normalize_p = false;
      DBUG_PRINT("info", ("mroonga: is_need_normalize: false: no text type"));
    }
  }
  DBUG_RETURN(need_normalize_p);
}

bool ha_mroonga::is_temporary_table_name(const char *name) const
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_PRINT("info", ("mroonga: table name = %s", name));
  bool temporary_table_name_p = (name[0] != '.');
  DBUG_RETURN(temporary_table_name_p);
}

void ha_mroonga::check_count_skip(key_part_map start_key_part_map,
                                  key_part_map end_key_part_map, bool fulltext)
{
  MRN_DBUG_ENTER_METHOD();

  if (!is_enable_optimization()) {
    DBUG_PRINT("info", ("mroonga: count skip: optimization is disabled"));
    count_skip = false;
    DBUG_VOID_RETURN;
  }

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
      count_skip = false;
      DBUG_VOID_RETURN;
    }

    uint i = 0;
    Item *where;
    if (fulltext) {
      where = select_lex->where;
      if (!where ||
          where->type() != Item::FUNC_ITEM ||
          ((Item_func *)where)->functype() != Item_func::FT_FUNC) {
        count_skip = false;
        DBUG_VOID_RETURN;
      }
      where = where->next;
      if (!where ||
          where->type() != Item::STRING_ITEM) {
        count_skip = false;
        DBUG_VOID_RETURN;
      }
      for (where = where->next; where; where = where->next) {
        if (where->type() != Item::FIELD_ITEM)
          break;
      }
      if (where != info) {
        count_skip = false;
        DBUG_VOID_RETURN;
      }
      count_skip = true;
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
          if (!field)
            break;
          if (field->table != table)
            break;
          uint j;
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
          count_skip = true;
          mrn_count_skip++;
          DBUG_VOID_RETURN;
        }
      }
    }
  }
  count_skip = false;
  DBUG_VOID_RETURN;
}

bool ha_mroonga::is_groonga_layer_condition(const Item *item,
                                            const Item_func **match_against)
{
  MRN_DBUG_ENTER_METHOD();

  if (!item) {
    DBUG_RETURN(false);
  }

  bool groonga_layer_condition = false;
  switch (item->type()) {
  case Item::COND_ITEM:
    if (grn_columns) {
      Item_cond *cond_item = (Item_cond *)item;
      if (cond_item->functype() == Item_func::COND_AND_FUNC) {
        List_iterator<Item> iterator(*((cond_item)->argument_list()));
        const Item *sub_item;
        groonga_layer_condition = true;
        while ((sub_item = iterator++)) {
          if (!is_groonga_layer_condition(sub_item, match_against)) {
            groonga_layer_condition = false;
            break;
          }
        }
      }
    }
    break;
  case Item::FUNC_ITEM:
    {
      const Item_func *func_item = (const Item_func *)item;
      switch (func_item->functype()) {
      case Item_func::EQ_FUNC:
        {
          Item **arguments = func_item->arguments();
          Item *left_item = arguments[0];
          Item *right_item = arguments[1];
          if (grn_columns) {
            if (left_item->type() == Item::FIELD_ITEM &&
                right_item->basic_const_item() &&
                right_item->type() == Item::INT_ITEM) {
              groonga_layer_condition = true;
            }
          }
        }
        break;
      case Item_func::FT_FUNC:
        groonga_layer_condition = true;
        if (match_against) {
          *match_against = func_item;
        }
        break;
      default:
        break;
      }
    }
    break;
  default:
    break;
  }

  DBUG_RETURN(groonga_layer_condition);
}

bool ha_mroonga::is_fulltext_search_item(const Item *item)
{
  MRN_DBUG_ENTER_METHOD();

  if (item->type() != Item::FUNC_ITEM) {
    DBUG_RETURN(false);
  }

  const Item_func *func_item = (const Item_func *)item;
  if (func_item->functype() != Item_func::FT_FUNC) {
    DBUG_RETURN(false);
  }

  DBUG_RETURN(true);
}

bool ha_mroonga::is_grn_zero_column_value(grn_obj *column, grn_obj *value)
{
  MRN_DBUG_ENTER_METHOD();

  if (column->header.type != GRN_COLUMN_FIX_SIZE) {
    DBUG_RETURN(false);
  }

  char *bytes = GRN_BULK_HEAD(value);
  unsigned int size = GRN_BULK_VSIZE(value);
  for (unsigned int i = 0; i < size; ++i) {
    if (bytes[i] != '\0') {
      DBUG_RETURN(false);
    }
  }

  DBUG_RETURN(true);
}

void ha_mroonga::check_fast_order_limit(grn_table_sort_key **sort_keys,
                                        int *n_sort_keys,
                                        longlong *limit)
{
  MRN_DBUG_ENTER_METHOD();

  if (!is_enable_optimization()) {
    DBUG_PRINT("info", ("mroonga: fast order limit: optimization is disabled"));
    fast_order_limit = false;
    DBUG_VOID_RETURN;
  }

  TABLE_LIST *table_list = table->pos_in_table_list;
  st_select_lex *select_lex = table_list->select_lex;
  SELECT_LEX_UNIT *unit = table_list->derived;
  st_select_lex *first_select_lex;
  if (unit)
  {
    first_select_lex = unit->first_select();
  } else {
    first_select_lex = select_lex;
  }
  DBUG_PRINT("info",
    ("mroonga: first_select_lex->options=%llu",
      first_select_lex ? first_select_lex->options : 0));

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
    if (select_lex->offset_limit) {
      *limit = select_lex->offset_limit->val_int();
    } else {
      *limit = 0;
    }
    *limit += select_lex->select_limit->val_int();
    if (*limit > (longlong)INT_MAX) {
      DBUG_PRINT("info",
                 ("mroonga: fast_order_limit = false: "
                  "too long limit: %lld <= %d is required",
                  *limit, INT_MAX));
      fast_order_limit = false;
      DBUG_VOID_RETURN;
    }
    if (first_select_lex && (first_select_lex->options & OPTION_FOUND_ROWS)) {
      DBUG_PRINT("info",
                 ("mroonga: fast_order_limit = false: "
                  "SQL_CALC_FOUND_ROWS is specified"));
      fast_order_limit = false;
      DBUG_VOID_RETURN;
    }
    Item *where = select_lex->where;
    const Item_func *match_against = NULL;
    if (pushed_cond) {
      if (!is_groonga_layer_condition(where, &match_against)) {
        DBUG_PRINT("info",
                   ("mroonga: fast_order_limit = false: "
                    "not groonga layer condition search"));
        fast_order_limit = false;
        DBUG_VOID_RETURN;
      }
      if (!match_against) {
        DBUG_PRINT("info",
                   ("mroonga: fast_order_limit = false: "
                    "groonga layer condition but not fulltext search"));
        fast_order_limit = false;
        DBUG_VOID_RETURN;
      }
    } else {
      if (!is_fulltext_search_item(where)) {
        DBUG_PRINT("info",
                   ("mroonga: fast_order_limit = false: not fulltext search"));
        fast_order_limit = false;
        DBUG_VOID_RETURN;
      }
      match_against = (const Item_func *)where;
    }
    *n_sort_keys = select_lex->order_list.elements;
    *sort_keys = (grn_table_sort_key *)malloc(sizeof(grn_table_sort_key) *
                                              *n_sort_keys);
    ORDER *order;
    int i;
    mrn_change_encoding(ctx, system_charset_info);
    for (order = (ORDER *) select_lex->order_list.first, i = 0; order;
         order = order->next, i++) {
      Item *item = *order->item;
      if (grn_columns && item->type() == Item::FIELD_ITEM)
      {
        Field *field = ((Item_field *) (*order->item))->field;
        const char *column_name = field->field_name;
        int column_name_size = strlen(column_name);

        if (is_need_normalize(field))
        {
          DBUG_PRINT("info", ("mroonga: fast_order_limit = false: "
                              "sort by collated value isn't supported yet."));
          fast_order_limit = false;
          free(*sort_keys);
          *sort_keys = NULL;
          *n_sort_keys = 0;
          DBUG_VOID_RETURN;
        }

        (*sort_keys)[i].key = grn_obj_column(ctx, matched_record_keys,
                                             column_name, column_name_size);
      } else if (match_against->eq(item, true)) {
        (*sort_keys)[i].key = grn_obj_column(ctx, matched_record_keys,
                                             MRN_COLUMN_NAME_SCORE,
                                             strlen(MRN_COLUMN_NAME_SCORE));
      } else {
        DBUG_PRINT("info", ("mroonga: fast_order_limit = false: "
                            "sort by computed value isn't supported."));
        fast_order_limit = false;
        free(*sort_keys);
        *sort_keys = NULL;
        *n_sort_keys = 0;
        DBUG_VOID_RETURN;
      }
      (*sort_keys)[i].offset = 0;
      if (MRN_ORDER_IS_ASC(order))
      {
        (*sort_keys)[i].flags = GRN_TABLE_SORT_ASC;
      } else {
        (*sort_keys)[i].flags = GRN_TABLE_SORT_DESC;
      }
    }
    DBUG_PRINT("info", ("mroonga: fast_order_limit = true"));
    fast_order_limit = true;
    mrn_fast_order_limit++;
    DBUG_VOID_RETURN;
  }
  DBUG_PRINT("info", ("mroonga: fast_order_limit = false"));
  fast_order_limit = false;
  DBUG_VOID_RETURN;
}

int ha_mroonga::generic_store_bulk_string(Field *field, grn_obj *buf)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  String value;
  field->val_str(NULL, &value);
  grn_obj_reinit(ctx, buf, GRN_DB_SHORT_TEXT, 0);
  GRN_TEXT_SET(ctx, buf, value.ptr(), value.length());
  DBUG_RETURN(error);
}

int ha_mroonga::generic_store_bulk_integer(Field *field, grn_obj *buf)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  long long value = field->val_int();
  uint32 size = field->pack_length();
  Field_num *field_num = static_cast<Field_num *>(field);
  bool is_unsigned = field_num->unsigned_flag;
  switch (size) {
  case 1:
    if (is_unsigned) {
      grn_obj_reinit(ctx, buf, GRN_DB_UINT8, 0);
      GRN_UINT8_SET(ctx, buf, value);
    } else {
      grn_obj_reinit(ctx, buf, GRN_DB_INT8, 0);
      GRN_INT8_SET(ctx, buf, value);
    }
    break;
  case 2:
    if (is_unsigned) {
      grn_obj_reinit(ctx, buf, GRN_DB_UINT16, 0);
      GRN_UINT16_SET(ctx, buf, value);
    } else {
      grn_obj_reinit(ctx, buf, GRN_DB_INT16, 0);
      GRN_INT16_SET(ctx, buf, value);
    }
    break;
  case 3:
  case 4:
    if (is_unsigned) {
      grn_obj_reinit(ctx, buf, GRN_DB_UINT32, 0);
      GRN_UINT32_SET(ctx, buf, value);
    } else {
      grn_obj_reinit(ctx, buf, GRN_DB_INT32, 0);
      GRN_INT32_SET(ctx, buf, value);
    }
    break;
  case 8:
    if (is_unsigned) {
      grn_obj_reinit(ctx, buf, GRN_DB_UINT64, 0);
      GRN_UINT64_SET(ctx, buf, value);
    } else {
      grn_obj_reinit(ctx, buf, GRN_DB_INT64, 0);
      GRN_INT64_SET(ctx, buf, value);
    }
    break;
  default:
    // Why!?
    error = HA_ERR_UNSUPPORTED;
    char error_message[MRN_MESSAGE_BUFFER_SIZE];
    snprintf(error_message, MRN_MESSAGE_BUFFER_SIZE,
             "unknown integer value size: <%u>: "
             "available sizes: [1, 2, 3, 4, 8]",
             size);
    push_warning(ha_thd(), Sql_condition::WARN_LEVEL_WARN,
                 error, error_message);
    break;
  }
  DBUG_RETURN(error);
}

int ha_mroonga::generic_store_bulk_unsigned_integer(Field *field, grn_obj *buf)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  long long signed_value = field->val_int();
  unsigned long long unsigned_value = *((unsigned long long *)(&signed_value));
  uint32 size = field->pack_length();
  switch (size) {
  case 1:
    grn_obj_reinit(ctx, buf, GRN_DB_UINT8, 0);
    GRN_UINT8_SET(ctx, buf, unsigned_value);
    break;
  case 2:
    grn_obj_reinit(ctx, buf, GRN_DB_UINT16, 0);
    GRN_UINT16_SET(ctx, buf, unsigned_value);
    break;
  case 3:
  case 4:
    grn_obj_reinit(ctx, buf, GRN_DB_UINT32, 0);
    GRN_UINT32_SET(ctx, buf, unsigned_value);
    break;
  case 8:
    grn_obj_reinit(ctx, buf, GRN_DB_UINT64, 0);
    GRN_UINT64_SET(ctx, buf, unsigned_value);
    break;
  default:
    // Why!?
    error = HA_ERR_UNSUPPORTED;
    char error_message[MRN_MESSAGE_BUFFER_SIZE];
    snprintf(error_message, MRN_MESSAGE_BUFFER_SIZE,
             "unknown unsigned integer value size: <%u>: "
             "available sizes: [1, 2, 3, 4, 8]",
             size);
    push_warning(ha_thd(), Sql_condition::WARN_LEVEL_WARN,
                 error, error_message);
    break;
  }
  DBUG_RETURN(error);
}

int ha_mroonga::generic_store_bulk_float(Field *field, grn_obj *buf)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  double value = field->val_real();
  uint32 size = field->pack_length();
  switch (size) {
  case 4:
  case 8:
    grn_obj_reinit(ctx, buf, GRN_DB_FLOAT, 0);
    GRN_FLOAT_SET(ctx, buf, value);
    break;
  default:
    // Why!?
    error = HA_ERR_UNSUPPORTED;
    char error_message[MRN_MESSAGE_BUFFER_SIZE];
    snprintf(error_message, MRN_MESSAGE_BUFFER_SIZE,
             "unknown float value size: <%u>: "
             "available sizes: [4, 8]",
             size);
    push_warning(ha_thd(), Sql_condition::WARN_LEVEL_WARN,
                 error, error_message);
    break;
  }
  DBUG_RETURN(error);
}

long long int ha_mroonga::get_grn_time_from_timestamp_field(Field_timestamp *field)
{
  MRN_DBUG_ENTER_METHOD();
  long long int grn_time = 0;
#ifdef MRN_TIMESTAMP_USE_TIMEVAL
  int warnings = 0;
  struct timeval time_value;
  if (field->get_timestamp(&time_value, &warnings)) {
    // XXX: Should we report warnings or MySQL does?
  } else {
    grn_time = GRN_TIME_PACK(time_value.tv_sec, time_value.tv_usec);
  }
#elif defined(MRN_TIMESTAMP_USE_MY_TIME_T)
  unsigned long int micro_seconds;
  my_time_t seconds = field->get_timestamp(&micro_seconds);
  grn_time = GRN_TIME_PACK(seconds, micro_seconds);
#else
  my_bool is_null_value;
  long seconds = field->get_timestamp(&is_null_value);
  grn_time = GRN_TIME_PACK(seconds, 0);
#endif
  DBUG_RETURN(grn_time);
}

int ha_mroonga::generic_store_bulk_timestamp(Field *field, grn_obj *buf)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  Field_timestamp *timestamp_field = (Field_timestamp *)field;
  long long int time = get_grn_time_from_timestamp_field(timestamp_field);
  grn_obj_reinit(ctx, buf, GRN_DB_TIME, 0);
  GRN_TIME_SET(ctx, buf, time);
  DBUG_RETURN(error);
}

int ha_mroonga::generic_store_bulk_date(Field *field, grn_obj *buf)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  long long int date_value = field->val_int();
  struct tm date;
  memset(&date, 0, sizeof(struct tm));
  date.tm_year = date_value / 10000 % 10000 - 1900;
  date.tm_mon = date_value / 100 % 100 - 1;
  date.tm_mday = date_value % 100;
  int usec = 0;
  long long int time = mrn_tm_to_grn_time(&date, usec);
  grn_obj_reinit(ctx, buf, GRN_DB_TIME, 0);
  GRN_TIME_SET(ctx, buf, time);
  DBUG_RETURN(error);
}

int ha_mroonga::generic_store_bulk_time(Field *field, grn_obj *buf)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  Field_time *time_field = (Field_time *)field;
  MYSQL_TIME mysql_time;
  time_field->get_time(&mysql_time);
  long long int time = mrn_mysql_time_to_grn_time(&mysql_time);
  grn_obj_reinit(ctx, buf, GRN_DB_TIME, 0);
  GRN_TIME_SET(ctx, buf, time);
  DBUG_RETURN(error);
}

int ha_mroonga::generic_store_bulk_datetime(Field *field, grn_obj *buf)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  Field_datetime *datetime_field = (Field_datetime *)field;
  MYSQL_TIME mysql_time;
  datetime_field->get_time(&mysql_time);
  long long int time = mrn_mysql_time_to_grn_time(&mysql_time);
  grn_obj_reinit(ctx, buf, GRN_DB_TIME, 0);
  GRN_TIME_SET(ctx, buf, time);
  DBUG_RETURN(error);
}

int ha_mroonga::generic_store_bulk_year(Field *field, grn_obj *buf)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;

  int year;
  if (field->field_length == 2) {
    year = field->val_int() + 2000;
  } else {
    year = field->val_int();
  }

  struct tm date;
  memset(&date, 0, sizeof(struct tm));
  date.tm_year = year - 1900;
  date.tm_mon = 0;
  date.tm_mday = 1;

  int usec = 0;
  long long int time = mrn_tm_to_grn_time(&date, usec);
  grn_obj_reinit(ctx, buf, GRN_DB_TIME, 0);
  GRN_TIME_SET(ctx, buf, time);
  DBUG_RETURN(error);
}

#ifdef MRN_HAVE_MYSQL_TYPE_DATETIME2
int ha_mroonga::generic_store_bulk_datetime2(Field *field, grn_obj *buf)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  Field_datetimef *datetimef_field = (Field_datetimef *)field;
  MYSQL_TIME mysql_time;
  datetimef_field->get_time(&mysql_time);
  long long int time = mrn_mysql_time_to_grn_time(&mysql_time);
  grn_obj_reinit(ctx, buf, GRN_DB_TIME, 0);
  GRN_TIME_SET(ctx, buf, time);
  DBUG_RETURN(error);
}
#endif

#ifdef MRN_HAVE_MYSQL_TYPE_TIME2
int ha_mroonga::generic_store_bulk_time2(Field *field, grn_obj *buf)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  MYSQL_TIME mysql_time;
  field->get_time(&mysql_time);
  long long int time = mrn_mysql_time_to_grn_time(&mysql_time);
  grn_obj_reinit(ctx, buf, GRN_DB_TIME, 0);
  GRN_TIME_SET(ctx, buf, time);
  DBUG_RETURN(error);
}
#endif

int ha_mroonga::generic_store_bulk_new_date(Field *field, grn_obj *buf)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  Field_newdate *newdate_field = (Field_newdate *)field;
  MYSQL_TIME mysql_date;
  newdate_field->get_time(&mysql_date);
  long long int time = mrn_mysql_time_to_grn_time(&mysql_date);
  grn_obj_reinit(ctx, buf, GRN_DB_TIME, 0);
  GRN_TIME_SET(ctx, buf, time);
  DBUG_RETURN(error);
}

int ha_mroonga::generic_store_bulk_new_decimal(Field *field, grn_obj *buf)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  String value;
  Field_new_decimal *new_decimal_field = (Field_new_decimal *)field;
  new_decimal_field->val_str(&value, NULL);
  grn_obj_reinit(ctx, buf, GRN_DB_SHORT_TEXT, 0);
  GRN_TEXT_SET(ctx, buf, value.ptr(), value.length());
  DBUG_RETURN(error);
}

int ha_mroonga::generic_store_bulk_blob(Field *field, grn_obj *buf)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  String buffer;
  Field_blob *blob = (Field_blob *)field;
  const char *value = blob->val_str(0, &buffer)->ptr();
  grn_obj_reinit(ctx, buf, GRN_DB_TEXT, 0);
  GRN_TEXT_SET(ctx, buf, value, blob->get_length());
  DBUG_RETURN(error);
}

int ha_mroonga::generic_store_bulk_geometry(Field *field, grn_obj *buf)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
#ifdef HAVE_SPATIAL
  String buffer;
  Field_geom *geometry = (Field_geom *)field;
  const char *wkb = geometry->val_str(0, &buffer)->ptr();
  int len = geometry->get_length();
  error = mrn_set_geometry(ctx, buf, wkb, len);
#endif
  DBUG_RETURN(error);
}

int ha_mroonga::generic_store_bulk(Field *field, grn_obj *buf)
{
  MRN_DBUG_ENTER_METHOD();
  int error;
  error = mrn_change_encoding(ctx, field->charset());
  if (error)
    return error;
  switch (field->real_type()) {
  case MYSQL_TYPE_DECIMAL:
    error = generic_store_bulk_string(field, buf);
    break;
  case MYSQL_TYPE_TINY:
  case MYSQL_TYPE_SHORT:
  case MYSQL_TYPE_LONG:
    error = generic_store_bulk_integer(field, buf);
    break;
  case MYSQL_TYPE_FLOAT:
  case MYSQL_TYPE_DOUBLE:
    error = generic_store_bulk_float(field, buf);
    break;
  case MYSQL_TYPE_NULL:
    error = generic_store_bulk_integer(field, buf);
    break;
  case MYSQL_TYPE_TIMESTAMP:
    error = generic_store_bulk_timestamp(field, buf);
    break;
  case MYSQL_TYPE_LONGLONG:
  case MYSQL_TYPE_INT24:
    error = generic_store_bulk_integer(field, buf);
    break;
  case MYSQL_TYPE_DATE:
    error = generic_store_bulk_date(field, buf);
    break;
  case MYSQL_TYPE_TIME:
    error = generic_store_bulk_time(field, buf);
    break;
  case MYSQL_TYPE_DATETIME:
    error = generic_store_bulk_datetime(field, buf);
    break;
  case MYSQL_TYPE_YEAR:
    error = generic_store_bulk_year(field, buf);
    break;
  case MYSQL_TYPE_NEWDATE:
    error = generic_store_bulk_new_date(field, buf);
    break;
  case MYSQL_TYPE_VARCHAR:
    error = generic_store_bulk_string(field, buf);
    break;
  case MYSQL_TYPE_BIT:
    error = generic_store_bulk_integer(field, buf);
    break;
#ifdef MRN_HAVE_MYSQL_TYPE_TIMESTAMP2
  case MYSQL_TYPE_TIMESTAMP2:
    error = generic_store_bulk_timestamp(field, buf);
    break;
#endif
#ifdef MRN_HAVE_MYSQL_TYPE_DATETIME2
  case MYSQL_TYPE_DATETIME2:
    error = generic_store_bulk_datetime2(field, buf);
    break;
#endif
#ifdef MRN_HAVE_MYSQL_TYPE_TIME2
  case MYSQL_TYPE_TIME2:
    error = generic_store_bulk_time2(field, buf);
    break;
#endif
  case MYSQL_TYPE_NEWDECIMAL:
    error = generic_store_bulk_new_decimal(field, buf);
    break;
  case MYSQL_TYPE_ENUM:
    error = generic_store_bulk_integer(field, buf);
    break;
  case MYSQL_TYPE_SET:
    error = generic_store_bulk_unsigned_integer(field, buf);
    break;
  case MYSQL_TYPE_TINY_BLOB:
  case MYSQL_TYPE_MEDIUM_BLOB:
  case MYSQL_TYPE_LONG_BLOB:
  case MYSQL_TYPE_BLOB:
    error = generic_store_bulk_blob(field, buf);
    break;
  case MYSQL_TYPE_VAR_STRING:
  case MYSQL_TYPE_STRING:
    error = generic_store_bulk_string(field, buf);
    break;
  case MYSQL_TYPE_GEOMETRY:
    error = generic_store_bulk_geometry(field, buf);
    break;
  default:
    error = HA_ERR_UNSUPPORTED;
    break;
  }
  DBUG_RETURN(error);
}

void ha_mroonga::storage_store_field_string(Field *field,
                                            const char *value,
                                            uint value_length)
{
  field->store(value, value_length, field->charset());
}

void ha_mroonga::storage_store_field_integer(Field *field,
                                             const char *value,
                                             uint value_length)
{
  Field_num *field_num = static_cast<Field_num *>(field);
  bool is_unsigned = field_num->unsigned_flag;
  switch (value_length) {
  case 1:
    {
      if (is_unsigned) {
        unsigned char field_value;
        field_value = *((unsigned char *)value);
        field->store(field_value);
      } else {
        char field_value;
        field_value = *((char *)value);
        field->store(field_value);
      }
      break;
    }
  case 2:
    {
      if (is_unsigned) {
        unsigned short field_value;
        field_value = *((unsigned short *)value);
        field->store(field_value);
      } else {
        short field_value;
        field_value = *((short *)value);
        field->store(field_value);
      }
      break;
    }
  case 4:
    {
      if (is_unsigned) {
        unsigned int field_value;
        field_value = *((unsigned int *)value);
        field->store(field_value);
      } else {
        int field_value;
        field_value = *((int *)value);
        field->store(field_value);
      }
      break;
    }
  case 8:
    {
      if (is_unsigned) {
        unsigned long long int field_value;
        field_value = *((unsigned long long int *)value);
        field->store(field_value);
      } else {
        long long int field_value;
        field_value = *((long long int *)value);
        field->store(field_value);
      }
      break;
    }
  default:
    {
      // Why!?
      char error_message[MRN_MESSAGE_BUFFER_SIZE];
      snprintf(error_message, MRN_MESSAGE_BUFFER_SIZE,
               "unknown integer value size: <%d>: "
               "available sizes: [1, 2, 4, 8]",
               value_length);
      push_warning(ha_thd(), Sql_condition::WARN_LEVEL_WARN,
                   HA_ERR_UNSUPPORTED, error_message);
      storage_store_field_string(field, value, value_length);
      break;
    }
  }
}

void ha_mroonga::storage_store_field_float(Field *field,
                                           const char *value,
                                           uint value_length)
{
  double field_value;
  field_value = *((double *)value);
  field->store(field_value);
}

void ha_mroonga::storage_store_field_timestamp(Field *field,
                                               const char *value,
                                               uint value_length)
{
  long long int time = *((long long int *)value);
  Field_timestamp *timestamp_field = (Field_timestamp *)field;
#ifdef MRN_TIMESTAMP_USE_TIMEVAL
  struct timeval time_value;
  GRN_TIME_UNPACK(time, time_value.tv_sec, time_value.tv_usec);
  timestamp_field->store_timestamp(&time_value);
#elif defined(MRN_TIMESTAMP_USE_MY_TIME_T)
  int32 sec, usec;
  GRN_TIME_UNPACK(time, sec, usec);
  timestamp_field->store_TIME(sec, usec);
#else
  int32 sec, usec __attribute__((unused));
  GRN_TIME_UNPACK(time, sec, usec);
  timestamp_field->store_timestamp(sec);
#endif
}

void ha_mroonga::storage_store_field_date(Field *field,
                                          const char *value,
                                          uint value_length)
{
  long long int time = *((long long int *)value);
  int32 sec, usec __attribute__((unused));
  GRN_TIME_UNPACK(time, sec, usec);
  struct tm date;
  time_t sec_t = sec;
  gmtime_r(&sec_t, &date);
  long long int date_in_mysql =
    (date.tm_year + 1900) * 10000 +
    (date.tm_mon + 1) * 100 +
    date.tm_mday;
  field->store(date_in_mysql);
}

void ha_mroonga::storage_store_field_time(Field *field,
                                          const char *value,
                                          uint value_length)
{
  long long int time = *((long long int *)value);
  MYSQL_TIME mysql_time;
  memset(&mysql_time, 0, sizeof(MYSQL_TIME));
  mysql_time.time_type = MYSQL_TIMESTAMP_TIME;
  mrn_grn_time_to_mysql_time(time, &mysql_time);
#ifdef MRN_FIELD_STORE_TIME_NEED_TYPE
  Field_time *time_field = (Field_time *)field;
  time_field->store_time(&mysql_time, mysql_time.time_type);
#else
  field->store_time(&mysql_time);
#endif
}

void ha_mroonga::storage_store_field_datetime(Field *field,
                                              const char *value,
                                              uint value_length)
{
  long long int time = *((long long int *)value);
  MYSQL_TIME mysql_datetime;
  memset(&mysql_datetime, 0, sizeof(MYSQL_TIME));
  mysql_datetime.time_type = MYSQL_TIMESTAMP_DATETIME;
  mrn_grn_time_to_mysql_time(time, &mysql_datetime);
#ifdef MRN_FIELD_STORE_TIME_NEED_TYPE
  Field_datetime *datetime_field = (Field_datetime *)field;
  datetime_field->store_time(&mysql_datetime, mysql_datetime.time_type);
#else
  field->store_time(&mysql_datetime);
#endif
}

void ha_mroonga::storage_store_field_year(Field *field,
                                          const char *value,
                                          uint value_length)
{
  long long int time = *((long long int *)value);
  MYSQL_TIME mysql_time;
  memset(&mysql_time, 0, sizeof(MYSQL_TIME));
  mysql_time.time_type = MYSQL_TIMESTAMP_DATE;
  mrn_grn_time_to_mysql_time(time, &mysql_time);
  field->store(mysql_time.year);
}

void ha_mroonga::storage_store_field_new_date(Field *field,
                                              const char *value,
                                              uint value_length)
{
  long long int time = *((long long int *)value);
  MYSQL_TIME mysql_date;
  memset(&mysql_date, 0, sizeof(MYSQL_TIME));
  mysql_date.time_type = MYSQL_TIMESTAMP_DATE;
  mrn_grn_time_to_mysql_time(time, &mysql_date);
#ifdef MRN_FIELD_STORE_TIME_NEED_TYPE
  Field_newdate *newdate_field = (Field_newdate *)field;
  newdate_field->store_time(&mysql_date, MYSQL_TIMESTAMP_DATE);
#else
  field->store_time(&mysql_date);
#endif
}

#ifdef MRN_HAVE_MYSQL_TYPE_DATETIME2
void ha_mroonga::storage_store_field_datetime2(Field *field,
                                               const char *value,
                                               uint value_length)
{
  long long int time = *((long long int *)value);
  MYSQL_TIME mysql_datetime;
  memset(&mysql_datetime, 0, sizeof(MYSQL_TIME));
  mysql_datetime.time_type = MYSQL_TIMESTAMP_DATETIME;
  mrn_grn_time_to_mysql_time(time, &mysql_datetime);
  field->store_time(&mysql_datetime);
}
#endif

#ifdef MRN_HAVE_MYSQL_TYPE_TIME2
void ha_mroonga::storage_store_field_time2(Field *field,
                                           const char *value,
                                           uint value_length)
{
  long long int time = *((long long int *)value);

  MYSQL_TIME mysql_time;
  memset(&mysql_time, 0, sizeof(MYSQL_TIME));
  mysql_time.time_type = MYSQL_TIMESTAMP_TIME;
  mrn_grn_time_to_mysql_time(time, &mysql_time);
  field->store_time(&mysql_time);
}
#endif

void ha_mroonga::storage_store_field_blob(Field *field,
                                          const char *value,
                                          uint value_length)
{
  Field_blob *blob = (Field_blob *)field;
  blob->set_ptr((uchar *)&value_length, (uchar *)value);
}

void ha_mroonga::storage_store_field_geometry(Field *field,
                                              const char *value,
                                              uint value_length)
{
  MRN_DBUG_ENTER_METHOD();
#ifdef HAVE_SPATIAL
  uchar wkb[SRID_SIZE + WKB_HEADER_SIZE + POINT_DATA_SIZE];
  grn_geo_point *field_value = (grn_geo_point *)value;
  int latitude, longitude;
  latitude = field_value->latitude;
  longitude = field_value->longitude;
  if (grn_source_column_geo) {
    GRN_GEO_POINT_SET(ctx, &source_point, latitude, longitude);
  }
  memset(wkb, 0, SRID_SIZE);
  memset(wkb + SRID_SIZE, Geometry::wkb_ndr, 1); // wkb_ndr is meaningless.
  int4store(wkb + SRID_SIZE + 1, Geometry::wkb_point);
  double latitude_in_degree, longitude_in_degree;
  latitude_in_degree = GRN_GEO_MSEC2DEGREE(latitude);
  longitude_in_degree = GRN_GEO_MSEC2DEGREE(longitude);
  float8store(wkb + SRID_SIZE + WKB_HEADER_SIZE,
              longitude_in_degree);
  float8store(wkb + SRID_SIZE + WKB_HEADER_SIZE + SIZEOF_STORED_DOUBLE,
              latitude_in_degree);
  field->store((const char *)wkb,
               (uint)(sizeof(wkb) / sizeof(*wkb)),
               field->charset());
#endif
  DBUG_VOID_RETURN;
}

void ha_mroonga::storage_store_field(Field *field,
                                     const char *value, uint value_length)
{
  field->set_notnull();
  switch (field->real_type()) {
  case MYSQL_TYPE_DECIMAL:
    storage_store_field_string(field, value, value_length);
    break;
  case MYSQL_TYPE_TINY:
  case MYSQL_TYPE_SHORT:
  case MYSQL_TYPE_LONG:
    storage_store_field_integer(field, value, value_length);
    break;
  case MYSQL_TYPE_FLOAT:
  case MYSQL_TYPE_DOUBLE:
    storage_store_field_float(field, value, value_length);
    break;
  case MYSQL_TYPE_NULL:
    storage_store_field_integer(field, value, value_length);
    break;
  case MYSQL_TYPE_TIMESTAMP:
    storage_store_field_timestamp(field, value, value_length);
    break;
  case MYSQL_TYPE_LONGLONG:
  case MYSQL_TYPE_INT24:
    storage_store_field_integer(field, value, value_length);
    break;
  case MYSQL_TYPE_DATE:
    storage_store_field_date(field, value, value_length);
    break;
  case MYSQL_TYPE_TIME:
    storage_store_field_time(field, value, value_length);
    break;
  case MYSQL_TYPE_DATETIME:
    storage_store_field_datetime(field, value, value_length);
    break;
  case MYSQL_TYPE_YEAR:
    storage_store_field_year(field, value, value_length);
    break;
  case MYSQL_TYPE_NEWDATE:
    storage_store_field_new_date(field, value, value_length);
    break;
  case MYSQL_TYPE_VARCHAR:
    storage_store_field_string(field, value, value_length);
    break;
  case MYSQL_TYPE_BIT:
    storage_store_field_integer(field, value, value_length);
    break;
#ifdef MRN_HAVE_MYSQL_TYPE_TIMESTAMP2
  case MYSQL_TYPE_TIMESTAMP2:
    storage_store_field_timestamp(field, value, value_length);
    break;
#endif
#ifdef MRN_HAVE_MYSQL_TYPE_DATETIME2
  case MYSQL_TYPE_DATETIME2:
    storage_store_field_datetime2(field, value, value_length);
    break;
#endif
#ifdef MRN_HAVE_MYSQL_TYPE_TIME2
  case MYSQL_TYPE_TIME2:
    storage_store_field_time2(field, value, value_length);
    break;
#endif
  case MYSQL_TYPE_NEWDECIMAL:
    storage_store_field_string(field, value, value_length);
    break;
  case MYSQL_TYPE_ENUM:
  case MYSQL_TYPE_SET:
    storage_store_field_integer(field, value, value_length);
    break;
  case MYSQL_TYPE_TINY_BLOB:
  case MYSQL_TYPE_MEDIUM_BLOB:
  case MYSQL_TYPE_LONG_BLOB:
  case MYSQL_TYPE_BLOB:
    storage_store_field_blob(field, value, value_length);
    break;
  case MYSQL_TYPE_VAR_STRING:
  case MYSQL_TYPE_STRING:
    storage_store_field_string(field, value, value_length);
    break;
  case MYSQL_TYPE_GEOMETRY:
    storage_store_field_geometry(field, value, value_length);
    break;
  }
}

void ha_mroonga::storage_store_fields(uchar *buf, grn_id record_id)
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_PRINT("info", ("mroonga: stored record ID: %d", record_id));
  my_ptrdiff_t ptr_diff = PTR_BYTE_DIFF(buf, table->record[0]);
  int i;
  int n_columns = table->s->fields;
  for (i = 0; i < n_columns; i++) {
    Field *field = table->field[i];

    if (bitmap_is_set(table->read_set, field->field_index) ||
        bitmap_is_set(table->write_set, field->field_index)) {
      const char *column_name = field->field_name;
      int column_name_size = strlen(column_name);

      if (ignoring_no_key_columns) {
        KEY key_info = table->s->key_info[active_index];
        if (strcmp(key_info.key_part[0].field->field_name, column_name)) {
          continue;
        }
      }

      mrn::DebugColumnAccess debug_column_access(table, table->write_set);
      DBUG_PRINT("info", ("mroonga: store column %d(%d)",i,field->field_index));
      field->move_field_offset(ptr_diff);
      if (strncmp(MRN_COLUMN_NAME_ID, column_name, column_name_size) == 0) {
        // for _id column
        field->set_notnull();
        field->store((int)record_id);
      } else {
        // actual column
        const char *value;
        uint32 value_length;
        value = grn_obj_get_value_(ctx, grn_columns[i], record_id,
                                   &value_length);
        storage_store_field(field, value, value_length);
      }
      field->move_field_offset(-ptr_diff);
    }
  }

  DBUG_VOID_RETURN;
}

void ha_mroonga::storage_store_fields_for_prep_update(const uchar *old_data,
                                                      uchar *new_data,
                                                      grn_id record_id)
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_PRINT("info", ("mroonga: stored record ID: %d", record_id));
  my_ptrdiff_t ptr_diff_old = PTR_BYTE_DIFF(old_data, table->record[0]);
  my_ptrdiff_t ptr_diff_new = 0;
#ifdef MRN_RBR_UPDATE_NEED_ALL_COLUMNS
  if (!written_by_row_based_binlog) {
    if (check_written_by_row_based_binlog()) {
      written_by_row_based_binlog = 2;
    } else {
      written_by_row_based_binlog = 1;
    }
  }
  bool need_all_columns =
    (new_data && written_by_row_based_binlog == 2);
#endif
  if (new_data) {
    ptr_diff_new = PTR_BYTE_DIFF(new_data, table->record[0]);
  }
  int i;
  int n_columns = table->s->fields;
  for (i = 0; i < n_columns; i++) {
    Field *field = table->field[i];

    if (
      !bitmap_is_set(table->read_set, field->field_index) &&
      !bitmap_is_set(table->write_set, field->field_index) &&
#ifdef MRN_RBR_UPDATE_NEED_ALL_COLUMNS
      (
        need_all_columns ||
#endif
        bitmap_is_set(&multiple_column_key_bitmap, field->field_index)
#ifdef MRN_RBR_UPDATE_NEED_ALL_COLUMNS
      )
#endif
    ) {
      mrn::DebugColumnAccess debug_column_access(table, table->write_set);
      DBUG_PRINT("info", ("mroonga: store column %d(%d)",i,field->field_index));
      const char *value;
      uint32 value_length;
      value = grn_obj_get_value_(ctx, grn_columns[i], record_id,
                                 &value_length);
      // old column
      field->move_field_offset(ptr_diff_old);
      storage_store_field(field, value, value_length);
      field->move_field_offset(-ptr_diff_old);
      if (new_data) {
        // new column
        field->move_field_offset(ptr_diff_new);
        storage_store_field(field, value, value_length);
        field->move_field_offset(-ptr_diff_new);
      }
    }
  }

  DBUG_VOID_RETURN;
}

void ha_mroonga::storage_store_fields_by_index(uchar *buf)
{
  MRN_DBUG_ENTER_METHOD();
  uint key_length;
  void *key;
  KEY *key_info = &table->key_info[active_index];
  if (table->s->primary_key == active_index)
    key_length = grn_table_cursor_get_key(ctx, cursor, &key);
  else
    key_length = grn_table_cursor_get_key(ctx, index_table_cursor, &key);

  if (key_info->key_parts == 1) {
    my_ptrdiff_t ptr_diff = PTR_BYTE_DIFF(buf, table->record[0]);
    Field *field = key_info->key_part->field;
    mrn::DebugColumnAccess debug_column_access(table, table->write_set);
    field->move_field_offset(ptr_diff);
    storage_store_field(field, (const char *)key, key_length);
    field->move_field_offset(-ptr_diff);
  } else {
    uchar enc_buf[MAX_KEY_LENGTH];
    uint enc_len;
    storage_encode_multiple_column_key(key_info,
                                       (uchar *)key, key_length,
                                       enc_buf, &enc_len,
                                       true);
    key_restore(buf, enc_buf, key_info, enc_len);
  }
  DBUG_VOID_RETURN;
}

int ha_mroonga::storage_encode_key_time(Field *field, const uchar *key,
                                        uchar *buf, uint *size)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  long long int time;
#ifdef MRN_MARIADB_P
  MYSQL_TIME mysql_time;
  if (field->decimals() == 0) {
    long long int packed_time = sint3korr(key);
    mysql_time.neg = false;
    if (packed_time < 0) {
      mysql_time.neg = true;
      packed_time = -packed_time;
    }
    mysql_time.year = 0;
    mysql_time.month = 0;
    mysql_time.day = 0;
    mysql_time.hour = (int)(packed_time / 10000);
    long long int minute_part = packed_time - mysql_time.hour * 10000;
    mysql_time.minute = (int)(minute_part / 100);
    mysql_time.second = (int)(minute_part % 100);
    mysql_time.second_part = 0;
    mysql_time.time_type = MYSQL_TIMESTAMP_TIME;
  } else {
    // TODO: remove me when MariaDB becomes based on MySQL 5.6.
    // This implementation may be costful.
    Field_time_hires *time_hires_field = (Field_time_hires *)field;
    Field_time_hires unpacker((uchar *)key,
                              (uchar *)(key - 1),
                              time_hires_field->null_bit,
                              time_hires_field->unireg_check,
                              time_hires_field->field_name,
                              time_hires_field->decimals(),
                              time_hires_field->charset());
    uint fuzzy_date = 0;
    unpacker.get_date(&mysql_time, fuzzy_date);
  }
  time = mrn_mysql_time_to_grn_time(&mysql_time);
#else
  int mysql_time = (int)sint3korr(key);
  int sec =
    mysql_time / 10000 * 60 * 60 +
    mysql_time / 100 % 100 * 60 +
    mysql_time % 60;
  int usec = 0;
  time = GRN_TIME_PACK(sec, usec);
#endif
  memcpy(buf, &time, 8);
  *size = 8;
  DBUG_RETURN(error);
}

int ha_mroonga::storage_encode_key_year(Field *field, const uchar *key,
                                        uchar *buf, uint *size)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  int year = (int)key[0];

  struct tm datetime;
  memset(&datetime, 0, sizeof(struct tm));
  datetime.tm_year = year;
  datetime.tm_mon = 0;
  datetime.tm_mday = 1;
  int usec = 0;
  long long int time = mrn_tm_to_grn_time(&datetime, usec);
  memcpy(buf, &time, 8);
  *size = 8;
  DBUG_RETURN(error);
}

int ha_mroonga::storage_encode_key_datetime(Field *field, const uchar *key,
                                            uchar *buf, uint *size)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  long long int time;
#ifdef MRN_MARIADB_P
  if (field->decimals() > 0) {
    // TODO: remove me when MariaDB becomes based on MySQL 5.6.
    // This implementation may be costful.
    Field_datetime_hires *datetime_hires_field = (Field_datetime_hires *)field;
    Field_datetime_hires unpacker((uchar *)key,
                                  (uchar *)(key - 1),
                                  datetime_hires_field->null_bit,
                                  datetime_hires_field->unireg_check,
                                  datetime_hires_field->field_name,
                                  datetime_hires_field->decimals(),
                                  datetime_hires_field->charset());
    MYSQL_TIME mysql_time;
    uint fuzzy_date = 0;
    unpacker.get_date(&mysql_time, fuzzy_date);
    time = mrn_mysql_time_to_grn_time(&mysql_time);
  } else
#endif
  {
    long long int encoded_datetime = sint8korr(key);
    uint32 part1 = (uint32)(encoded_datetime / LL(1000000));
    uint32 part2 = (uint32)(encoded_datetime -
                            (unsigned long long int)part1 * LL(1000000));
    struct tm date;
    memset(&date, 0, sizeof(struct tm));
    date.tm_year = part1 / 10000 - 1900;
    date.tm_mon = part1 / 100 % 100 - 1;
    date.tm_mday = part1 % 100;
    date.tm_hour = part2 / 10000;
    date.tm_min = part2 / 100 % 100;
    date.tm_sec = part2 % 100;
    int usec = 0;
    time = mrn_tm_to_grn_time(&date, usec);
  }
  memcpy(buf, &time, 8);
  *size = 8;
  DBUG_RETURN(error);
}

#ifdef MRN_HAVE_MYSQL_TYPE_TIME2
int ha_mroonga::storage_encode_key_time2(Field *field, const uchar *key,
                                         uchar *buf, uint *size)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;

  Field_timef *time2_field = (Field_timef *)field;
  long long int packed_time =
    my_time_packed_from_binary(key, time2_field->decimals());
  MYSQL_TIME mysql_time;
  TIME_from_longlong_time_packed(&mysql_time, packed_time);
  int sec, usec;
  sec = mysql_time.hour * 60 * 60 + mysql_time.minute * 60 + mysql_time.second;
  if (mysql_time.neg) {
    sec = -sec;
  }
  usec = mysql_time.second_part;
  long long int time = GRN_TIME_PACK(sec, usec);
  memcpy(buf, &time, 8);
  *size = 8;

  DBUG_RETURN(error);
}
#endif

int ha_mroonga::storage_encode_key_enum(Field *field, const uchar *key,
                                        uchar *buf, uint *size)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  if (field->pack_length() == 1) {
    uchar value;
    value = key[0];
    *size = 1;
    memcpy(buf, &value, *size);
  } else {
    uint16 value;
    shortget(value, key);
    *size = 2;
    memcpy(buf, &value, *size);
  }
  DBUG_RETURN(error);
}

int ha_mroonga::storage_encode_key_set(Field *field, const uchar *key,
                                       uchar *buf, uint *size)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  Field_set unpacker((uchar *)key, field->field_length, (uchar *)(key - 1),
                     field->null_bit, field->unireg_check, field->field_name,
                     field->pack_length(),
                     static_cast<Field_set*>(field)->typelib,
                     static_cast<Field_set*>(field)->charset());
  switch (field->pack_length()) {
  case 1:
    {
      int8 signed_value = (int8)(unpacker.val_int());
      uint8 unsigned_value = *((uint8 *)&signed_value);
      *size = 1;
      memcpy(buf, &unsigned_value, *size);
    }
    break;
  case 2:
    {
      int16 signed_value = (int16)(unpacker.val_int());
      uint16 unsigned_value = *((uint16 *)&signed_value);
      *size = 2;
      memcpy(buf, &unsigned_value, *size);
    }
    break;
  case 3:
  case 4:
    {
      int32 signed_value = (int32)(unpacker.val_int());
      uint32 unsigned_value = *((uint32 *)&signed_value);
      *size = 4;
      memcpy(buf, &unsigned_value, *size);
    }
    break;
  case 8:
  default:
    {
      int64 signed_value = (int64)(unpacker.val_int());
      uint64 unsigned_value = *((uint64 *)&signed_value);
      *size = 8;
      memcpy(buf, &unsigned_value, *size);
    }
    break;
  }
  DBUG_RETURN(error);
}

int ha_mroonga::storage_encode_key(Field *field, const uchar *key,
                                   uchar *buf, uint *size)
{
  MRN_DBUG_ENTER_METHOD();
  int error;
  const uchar *ptr = key;

  error = mrn_change_encoding(ctx, field->charset());
  if (error)
    DBUG_RETURN(error);

  if (field->null_bit) {
    ptr += 1;
  }

  switch (field->real_type()) {
  case MYSQL_TYPE_BIT:
  case MYSQL_TYPE_TINY:
    {
      memcpy(buf, ptr, 1);
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
      float float_value;
      double double_value;
      float4get(float_value, ptr);
      double_value = float_value;
      memcpy(buf, &double_value, 8);
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
    storage_encode_key_time(field, ptr, buf, size);
    break;
  case MYSQL_TYPE_YEAR:
    storage_encode_key_year(field, ptr, buf, size);
    break;
  case MYSQL_TYPE_DATETIME:
    storage_encode_key_datetime(field, ptr, buf, size);
    break;
  case MYSQL_TYPE_NEWDATE:
    {
      uint32 encoded_date = uint3korr(ptr);
      struct tm date;
      memset(&date, 0, sizeof(struct tm));
      date.tm_year = encoded_date / (16 * 32) - 1900;
      date.tm_mon = encoded_date / 32 % 16 - 1;
      date.tm_mday = encoded_date % 32;
      int usec = 0;
      long long int time = mrn_tm_to_grn_time(&date, usec);
      memcpy(buf, &time, 8);
      *size = 8;
      break;
    }
#ifdef MRN_HAVE_MYSQL_TYPE_TIME2
  case MYSQL_TYPE_TIME2:
    storage_encode_key_time2(field, ptr, buf, size);
    break;
#endif
  case MYSQL_TYPE_STRING:
  case MYSQL_TYPE_VARCHAR:
  case MYSQL_TYPE_BLOB:
    {
      ptr += HA_KEY_BLOB_LENGTH;
      const char *val = (const char *)ptr;
      int len = strlen(val);
      memcpy(buf, val, len);
      *size = len;
      break;
    }
  case MYSQL_TYPE_ENUM:
    storage_encode_key_enum(field, ptr, buf, size);
    break;
  case MYSQL_TYPE_SET:
    storage_encode_key_set(field, ptr, buf, size);
    break;
  default:
    error = HA_ERR_UNSUPPORTED;
    break;
  }
  DBUG_RETURN(error);
}

void ha_mroonga::storage_encode_multiple_column_key_float(volatile float value,
                                                          uint data_size,
                                                          uchar *buffer,
                                                          bool decode)
{
  MRN_DBUG_ENTER_METHOD();
  int n_bits = (data_size * 8 - 1);
  volatile int *int_value_pointer = (int *)(&value);
  int int_value = *int_value_pointer;
  if (!decode)
    int_value ^= ((int_value >> n_bits) | (1 << n_bits));
  mrn_byte_order_host_to_network(buffer, &int_value, data_size);
  if (decode) {
    int_value = *((int *)buffer);
    *((int *)buffer) = int_value ^ (((int_value ^ (1 << n_bits)) >> n_bits) |
                                    (1 << n_bits));
  }
  DBUG_VOID_RETURN;
}

void ha_mroonga::storage_encode_multiple_column_key_double(volatile double value,
                                                           uint data_size,
                                                           uchar *buffer,
                                                           bool decode)
{
  MRN_DBUG_ENTER_METHOD();
  int n_bits = (data_size * 8 - 1);
  volatile long long int *long_long_value_pointer = (long long int *)(&value);
  volatile long long int long_long_value = *long_long_value_pointer;
  if (!decode)
     long_long_value ^= ((long_long_value >> n_bits) | (1LL << n_bits));
  mrn_byte_order_host_to_network(buffer, &long_long_value, data_size);
  if (decode) {
    long_long_value = *((long long int *)buffer);
    *((long long int *)buffer) =
      long_long_value ^ (((long_long_value ^ (1LL << n_bits)) >> n_bits) |
                          (1LL << n_bits));
  }
  DBUG_VOID_RETURN;
}

int ha_mroonga::storage_encode_multiple_column_key(KEY *key_info,
                                                   const uchar *key,
                                                   uint key_length,
                                                   uchar *buffer,
                                                   uint *encoded_length,
                                                   bool decode)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  const uchar *current_key = key;
  const uchar *key_end = key + key_length;
  uchar *current_buffer = buffer;

  int n_key_parts = key_info->key_parts;
  *encoded_length = 0;
  for (int i = 0; i < n_key_parts && current_key < key_end; i++) {
    KEY_PART_INFO key_part = key_info->key_part[i];
    Field *field = key_part.field;

    if (field->null_bit) {
      *current_buffer = *current_key;
      current_key += 1;
      current_buffer += 1;
      (*encoded_length)++;
    }

    enum {
      TYPE_UNKNOWN,
      TYPE_LONG_LONG_NUMBER,
      TYPE_NUMBER,
      TYPE_FLOAT,
      TYPE_DOUBLE,
      TYPE_BYTE_SEQUENCE
    } data_type = TYPE_UNKNOWN;
    uint data_size = 0;
    long long int long_long_value = 0;
    switch (field->real_type()) {
    case MYSQL_TYPE_DECIMAL:
      data_type = TYPE_BYTE_SEQUENCE;
      data_size = key_part.length;
      break;
    case MYSQL_TYPE_TINY:
      data_type = TYPE_NUMBER;
      data_size = 1;
      break;
    case MYSQL_TYPE_SHORT:
      data_type = TYPE_NUMBER;
      data_size = 2;
      break;
    case MYSQL_TYPE_LONG:
      data_type = TYPE_NUMBER;
      data_size = 4;
      break;
    case MYSQL_TYPE_FLOAT:
      data_type = TYPE_FLOAT;
      data_size = 4;
      {
        float value;
        float4get(value, current_key);
        storage_encode_multiple_column_key_float(value, data_size,
                                                 current_buffer, decode);
      }
      break;
    case MYSQL_TYPE_DOUBLE:
      data_type = TYPE_DOUBLE;
      data_size = 8;
      {
        double value;
        float8get(value, current_key);
        storage_encode_multiple_column_key_double(value, data_size,
                                                  current_buffer, decode);
      }
      break;
    case MYSQL_TYPE_NULL:
      data_type = TYPE_NUMBER;
      data_size = 1;
      break;
    case MYSQL_TYPE_TIMESTAMP:
      // TODO
      data_type = TYPE_BYTE_SEQUENCE;
      data_size = key_part.length;
      break;
    case MYSQL_TYPE_LONGLONG:
      data_type = TYPE_NUMBER;
      data_size = 8;
      break;
    case MYSQL_TYPE_INT24:
      data_type = TYPE_NUMBER;
      data_size = 3;
      break;
    case MYSQL_TYPE_DATE:
    case MYSQL_TYPE_TIME:
    case MYSQL_TYPE_DATETIME:
    case MYSQL_TYPE_YEAR:
    case MYSQL_TYPE_NEWDATE:
      data_type = TYPE_LONG_LONG_NUMBER;
      long_long_value = (long long int)sint8korr(current_key);
      data_size = 8;
      break;
    case MYSQL_TYPE_VARCHAR:
      data_type = TYPE_BYTE_SEQUENCE;
      data_size = HA_KEY_BLOB_LENGTH + key_part.length;
      break;
    case MYSQL_TYPE_BIT:
      // TODO
      data_type = TYPE_NUMBER;
      data_size = 1;
      break;
#ifdef MRN_HAVE_MYSQL_TYPE_TIMESTAMP2
    case MYSQL_TYPE_TIMESTAMP2:
      // TODO
      data_type = TYPE_LONG_LONG_NUMBER;
      long_long_value = (long long int)sint8korr(current_key);
      data_size = 8;
      break;
#endif
#ifdef MRN_HAVE_MYSQL_TYPE_DATETIME2
    case MYSQL_TYPE_DATETIME2:
      // TODO
      data_type = TYPE_LONG_LONG_NUMBER;
      long_long_value = (long long int)sint8korr(current_key);
      data_size = 8;
      break;
#endif
#ifdef MRN_HAVE_MYSQL_TYPE_TIME2
    case MYSQL_TYPE_TIME2:
      // TODO
      data_type = TYPE_LONG_LONG_NUMBER;
      long_long_value = (long long int)sint8korr(current_key);
      data_size = 8;
      break;
#endif
    case MYSQL_TYPE_NEWDECIMAL:
      data_type = TYPE_BYTE_SEQUENCE;
      data_size = key_part.length;
      break;
    case MYSQL_TYPE_ENUM:
      // TODO
      data_type = TYPE_NUMBER;
      data_size = 1;
      break;
    case MYSQL_TYPE_SET:
      // TODO
      data_type = TYPE_NUMBER;
      data_size = 1;
      break;
    case MYSQL_TYPE_TINY_BLOB:
    case MYSQL_TYPE_MEDIUM_BLOB:
    case MYSQL_TYPE_LONG_BLOB:
    case MYSQL_TYPE_BLOB:
      // TODO
      data_type = TYPE_BYTE_SEQUENCE;
      data_size = HA_KEY_BLOB_LENGTH + key_part.length;
      break;
    case MYSQL_TYPE_VAR_STRING:
    case MYSQL_TYPE_STRING:
      // TODO
      data_type = TYPE_BYTE_SEQUENCE;
      data_size = key_part.length;
      break;
    case MYSQL_TYPE_GEOMETRY:
      // TODO
      data_type = TYPE_BYTE_SEQUENCE;
      data_size = key_part.length;
      break;
    }

    switch (data_type) {
    case TYPE_UNKNOWN:
      // TODO: This will not be happen. This is just for
      // suppressing warnings by gcc -O2. :<
      error = HA_ERR_UNSUPPORTED;
      break;
    case TYPE_LONG_LONG_NUMBER:
      if (decode)
        *((uint8 *)(&long_long_value)) ^= 0x80;
      mrn_byte_order_host_to_network(current_buffer, &long_long_value,
                                     data_size);
      if (!decode)
        *((uint8 *)(current_buffer)) ^= 0x80;
      break;
    case TYPE_NUMBER:
      if (decode)
      {
        Field_num *number_field = (Field_num *)field;
        if (!number_field->unsigned_flag) {
          *((uint8 *)(current_key)) ^= 0x80;
        }
      }
      mrn_byte_order_host_to_network(current_buffer, current_key, data_size);
      if (!decode)
      {
        Field_num *number_field = (Field_num *)field;
        if (!number_field->unsigned_flag) {
          *((uint8 *)(current_buffer)) ^= 0x80;
        }
      }
      break;
    case TYPE_FLOAT:
      break;
    case TYPE_DOUBLE:
      break;
    case TYPE_BYTE_SEQUENCE:
      memcpy(current_buffer, current_key, data_size);
      break;
    }

    if (error) {
      break;
    }

    current_key += data_size;
    current_buffer += data_size;
    *encoded_length += data_size;
  }

  DBUG_RETURN(error);
}

int ha_mroonga::wrapper_reset()
{
  int error = 0;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  error = wrap_handler->ha_reset();
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::storage_reset()
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(0);
}

int ha_mroonga::reset()
{
  int error = 0;
  THD *thd = ha_thd();
  MRN_DBUG_ENTER_METHOD();
  DBUG_PRINT("info", ("mroonga: this=%p", this));
  clear_search_result();
  clear_search_result_geo();
  if (share->wrapper_mode)
    error = wrapper_reset();
  else
    error = storage_reset();
  ignoring_no_key_columns = false;
  inserting_with_update = false;
  ignoring_duplicated_key = false;
  fulltext_searching = false;
  replacing_ = false;
  written_by_row_based_binlog = 0;
  mrn_lock_type = F_UNLCK;
  mrn_clear_alter_share(thd);
  current_ft_item = NULL;
  DBUG_RETURN(error);
}

#ifdef MRN_HANDLER_CLONE_NEED_NAME
handler *ha_mroonga::wrapper_clone(const char *name, MEM_ROOT *mem_root)
{
  handler *cloned_handler;
  MRN_DBUG_ENTER_METHOD();
  if (!(cloned_handler = get_new_handler(table->s, mem_root,
                                         table->s->db_type())))
    DBUG_RETURN(NULL);
  ((ha_mroonga *) cloned_handler)->is_clone = true;
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

handler *ha_mroonga::storage_clone(const char *name, MEM_ROOT *mem_root)
{
  MRN_DBUG_ENTER_METHOD();
  handler *cloned_handler;
  cloned_handler = handler::clone(name, mem_root);
  DBUG_RETURN(cloned_handler);
}

handler *ha_mroonga::clone(const char *name, MEM_ROOT *mem_root)
{
  MRN_DBUG_ENTER_METHOD();
  handler *cloned_handler;
  if (share->wrapper_mode)
  {
    cloned_handler = wrapper_clone(name, mem_root);
  } else {
    cloned_handler = storage_clone(name, mem_root);
  }
  DBUG_RETURN(cloned_handler);
}
#else
handler *ha_mroonga::wrapper_clone(MEM_ROOT *mem_root)
{
  handler *cloned_handler;
  MRN_DBUG_ENTER_METHOD();
  if (!(cloned_handler = get_new_handler(table->s, mem_root,
                                         table->s->db_type())))
    DBUG_RETURN(NULL);
  ((ha_mroonga *) cloned_handler)->is_clone = true;
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

handler *ha_mroonga::storage_clone(MEM_ROOT *mem_root)
{
  MRN_DBUG_ENTER_METHOD();
  handler *cloned_handler;
  cloned_handler = handler::clone(mem_root);
  DBUG_RETURN(cloned_handler);
}

handler *ha_mroonga::clone(MEM_ROOT *mem_root)
{
  MRN_DBUG_ENTER_METHOD();
  handler *cloned_handler;
  if (share->wrapper_mode)
  {
    cloned_handler = wrapper_clone(mem_root);
  } else {
    cloned_handler = storage_clone(mem_root);
  }
  DBUG_RETURN(cloned_handler);
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

uint8 ha_mroonga::storage_table_cache_type()
{
  MRN_DBUG_ENTER_METHOD();
  uint8 type = handler::table_cache_type();
  DBUG_RETURN(type);
}

uint8 ha_mroonga::table_cache_type()
{
  MRN_DBUG_ENTER_METHOD();
  uint8 type;
  if (share->wrapper_mode)
  {
    type = wrapper_table_cache_type();
  } else {
    type = storage_table_cache_type();
  }
  DBUG_RETURN(type);
}

#ifdef MRN_HANDLER_HAVE_MULTI_RANGE_READ
ha_rows ha_mroonga::wrapper_multi_range_read_info_const(uint keyno,
                                                        RANGE_SEQ_IF *seq,
                                                        void *seq_init_param,
                                                        uint n_ranges,
                                                        uint *bufsz,
                                                        uint *flags,
                                                        Cost_estimate *cost)
{
  MRN_DBUG_ENTER_METHOD();
  ha_rows rows;
  KEY key_info = table->key_info[keyno];
  if (mrn_is_geo_key(&key_info)) {
    rows = handler::multi_range_read_info_const(keyno, seq, seq_init_param,
                                                n_ranges, bufsz, flags, cost);
    DBUG_RETURN(rows);
  }
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  if (fulltext_searching)
    set_pk_bitmap();
  rows = wrap_handler->multi_range_read_info_const(keyno, seq, seq_init_param,
                                                   n_ranges, bufsz, flags,
                                                   cost);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(rows);
}

ha_rows ha_mroonga::storage_multi_range_read_info_const(uint keyno,
                                                        RANGE_SEQ_IF *seq,
                                                        void *seq_init_param,
                                                        uint n_ranges,
                                                        uint *bufsz,
                                                        uint *flags,
                                                        Cost_estimate *cost)
{
  MRN_DBUG_ENTER_METHOD();
  ha_rows rows = handler::multi_range_read_info_const(keyno, seq,
                                                      seq_init_param,
                                                      n_ranges, bufsz, flags,
                                                      cost);
  DBUG_RETURN(rows);
}

ha_rows ha_mroonga::multi_range_read_info_const(uint keyno, RANGE_SEQ_IF *seq,
                                                void *seq_init_param,
                                                uint n_ranges, uint *bufsz,
                                                uint *flags,
                                                Cost_estimate *cost)
{
  MRN_DBUG_ENTER_METHOD();
  ha_rows rows;
  if (share->wrapper_mode)
  {
    rows = wrapper_multi_range_read_info_const(keyno, seq, seq_init_param,
                                               n_ranges, bufsz,
                                               flags, cost);
  } else {
    rows = storage_multi_range_read_info_const(keyno, seq, seq_init_param,
                                               n_ranges, bufsz,
                                               flags, cost);
  }
  DBUG_RETURN(rows);
}

ha_rows ha_mroonga::wrapper_multi_range_read_info(uint keyno, uint n_ranges,
                                                  uint keys,
#ifdef MRN_HANDLER_HAVE_MULTI_RANGE_READ_INFO_KEY_PARTS
                                                  uint key_parts,
#endif
                                                  uint *bufsz,
                                                  uint *flags,
                                                  Cost_estimate *cost)
{
  MRN_DBUG_ENTER_METHOD();
  ha_rows rows;
  KEY key_info = table->key_info[keyno];
  if (mrn_is_geo_key(&key_info)) {
    rows = handler::multi_range_read_info(keyno, n_ranges, keys,
#ifdef MRN_HANDLER_HAVE_MULTI_RANGE_READ_INFO_KEY_PARTS
                                          key_parts,
#endif
                                          bufsz, flags, cost);
    DBUG_RETURN(rows);
  }
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  if (fulltext_searching)
    set_pk_bitmap();
  rows = wrap_handler->multi_range_read_info(keyno, n_ranges, keys,
#ifdef MRN_HANDLER_HAVE_MULTI_RANGE_READ_INFO_KEY_PARTS
                                             key_parts,
#endif
                                             bufsz, flags, cost);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(rows);
}

ha_rows ha_mroonga::storage_multi_range_read_info(uint keyno, uint n_ranges,
                                                  uint keys,
#ifdef MRN_HANDLER_HAVE_MULTI_RANGE_READ_INFO_KEY_PARTS
                                                  uint key_parts,
#endif
                                                  uint *bufsz,
                                                  uint *flags,
                                                  Cost_estimate *cost)
{
  MRN_DBUG_ENTER_METHOD();
  ha_rows rows = handler::multi_range_read_info(keyno, n_ranges, keys,
#ifdef MRN_HANDLER_HAVE_MULTI_RANGE_READ_INFO_KEY_PARTS
                                                key_parts,
#endif
                                                bufsz, flags, cost);
  DBUG_RETURN(rows);
}

ha_rows ha_mroonga::multi_range_read_info(uint keyno, uint n_ranges, uint keys,
#ifdef MRN_HANDLER_HAVE_MULTI_RANGE_READ_INFO_KEY_PARTS
                                          uint key_parts,
#endif
                                          uint *bufsz, uint *flags,
                                          Cost_estimate *cost)
{
  MRN_DBUG_ENTER_METHOD();
  ha_rows rows;
  if (share->wrapper_mode)
  {
    rows = wrapper_multi_range_read_info(keyno, n_ranges, keys,
#ifdef MRN_HANDLER_HAVE_MULTI_RANGE_READ_INFO_KEY_PARTS
                                         key_parts,
#endif
                                         bufsz, flags, cost);
  } else {
    rows = storage_multi_range_read_info(keyno, n_ranges, keys,
#ifdef MRN_HANDLER_HAVE_MULTI_RANGE_READ_INFO_KEY_PARTS
                                         key_parts,
#endif
                                         bufsz, flags, cost);
  }
  DBUG_RETURN(rows);
}

int ha_mroonga::wrapper_multi_range_read_init(RANGE_SEQ_IF *seq,
                                              void *seq_init_param,
                                              uint n_ranges, uint mode,
                                              HANDLER_BUFFER *buf)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  KEY key_info = table->key_info[active_index];
  if (mrn_is_geo_key(&key_info)) {
    error = handler::multi_range_read_init(seq, seq_init_param,
                                           n_ranges, mode, buf);
    DBUG_RETURN(error);
  }
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  if (fulltext_searching)
    set_pk_bitmap();
  error = wrap_handler->multi_range_read_init(seq, seq_init_param,
                                              n_ranges, mode, buf);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::storage_multi_range_read_init(RANGE_SEQ_IF *seq,
                                              void *seq_init_param,
                                              uint n_ranges, uint mode,
                                              HANDLER_BUFFER *buf)
{
  MRN_DBUG_ENTER_METHOD();
  int error = handler::multi_range_read_init(seq, seq_init_param,
                                             n_ranges, mode, buf);
  DBUG_RETURN(error);
}

int ha_mroonga::multi_range_read_init(RANGE_SEQ_IF *seq, void *seq_init_param,
                                      uint n_ranges, uint mode,
                                      HANDLER_BUFFER *buf)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  if (share->wrapper_mode)
  {
    error = wrapper_multi_range_read_init(seq, seq_init_param,
                                          n_ranges, mode, buf);
  } else {
    error = storage_multi_range_read_init(seq, seq_init_param,
                                          n_ranges, mode, buf);
  }
  DBUG_RETURN(error);
}

int ha_mroonga::wrapper_multi_range_read_next(range_id_t *range_info)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  KEY key_info = table->key_info[active_index];
  if (mrn_is_geo_key(&key_info)) {
    error = handler::multi_range_read_next(range_info);
    DBUG_RETURN(error);
  }
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  if (fulltext_searching)
    set_pk_bitmap();
  error = wrap_handler->multi_range_read_next(range_info);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::storage_multi_range_read_next(range_id_t *range_info)
{
  MRN_DBUG_ENTER_METHOD();
  int error = handler::multi_range_read_next(range_info);
  DBUG_RETURN(error);
}

int ha_mroonga::multi_range_read_next(range_id_t *range_info)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  if (share->wrapper_mode)
  {
    error = wrapper_multi_range_read_next(range_info);
  } else {
    error = storage_multi_range_read_next(range_info);
  }
  DBUG_RETURN(error);
}
#else // MRN_HANDLER_HAVE_MULTI_RANGE_READ
int ha_mroonga::wrapper_read_multi_range_first(KEY_MULTI_RANGE **found_range_p,
                                               KEY_MULTI_RANGE *ranges,
                                               uint range_count,
                                               bool sorted,
                                               HANDLER_BUFFER *buffer)
{
  int error = 0;
  MRN_DBUG_ENTER_METHOD();
  KEY key_info = table->key_info[active_index];
  if (mrn_is_geo_key(&key_info)) {
    error = handler::read_multi_range_first(found_range_p, ranges,
                                            range_count, sorted, buffer);
    DBUG_RETURN(error);
  }
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  if (fulltext_searching)
    set_pk_bitmap();
  error = wrap_handler->read_multi_range_first(found_range_p, ranges,
                                                  range_count, sorted, buffer);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::storage_read_multi_range_first(KEY_MULTI_RANGE **found_range_p,
                                               KEY_MULTI_RANGE *ranges,
                                               uint range_count,
                                               bool sorted,
                                               HANDLER_BUFFER *buffer)
{
  MRN_DBUG_ENTER_METHOD();
  int error = handler::read_multi_range_first(found_range_p, ranges,
                                              range_count, sorted, buffer);
  DBUG_RETURN(error);
}

int ha_mroonga::read_multi_range_first(KEY_MULTI_RANGE **found_range_p,
                                       KEY_MULTI_RANGE *ranges,
                                       uint range_count,
                                       bool sorted,
                                       HANDLER_BUFFER *buffer)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  if (share->wrapper_mode)
  {
    error = wrapper_read_multi_range_first(found_range_p, ranges,
                                           range_count, sorted, buffer);
  } else {
    error = storage_read_multi_range_first(found_range_p, ranges,
                                           range_count, sorted, buffer);
  }
  DBUG_RETURN(error);
}

int ha_mroonga::wrapper_read_multi_range_next(KEY_MULTI_RANGE **found_range_p)
{
  int error = 0;
  MRN_DBUG_ENTER_METHOD();
  KEY key_info = table->key_info[active_index];
  if (mrn_is_geo_key(&key_info)) {
    error = handler::read_multi_range_next(found_range_p);
    DBUG_RETURN(error);
  }
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  if (fulltext_searching)
    set_pk_bitmap();
  error = wrap_handler->read_multi_range_next(found_range_p);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::storage_read_multi_range_next(KEY_MULTI_RANGE **found_range_p)
{
  MRN_DBUG_ENTER_METHOD();
  int error = handler::read_multi_range_next(found_range_p);
  DBUG_RETURN(error);
}

int ha_mroonga::read_multi_range_next(KEY_MULTI_RANGE **found_range_p)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  if (share->wrapper_mode)
  {
    error = wrapper_read_multi_range_next(found_range_p);
  } else {
    error = storage_read_multi_range_next(found_range_p);
  }
  DBUG_RETURN(error);
}
#endif // MRN_HANDLER_HAVE_MULTI_RANGE_READ

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

void ha_mroonga::storage_start_bulk_insert(ha_rows rows)
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
    storage_start_bulk_insert(rows);
  DBUG_VOID_RETURN;
}

int ha_mroonga::wrapper_end_bulk_insert()
{
  int error = 0;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  error = wrap_handler->ha_end_bulk_insert();
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::storage_end_bulk_insert()
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(0);
}

int ha_mroonga::end_bulk_insert()
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  if (share->wrapper_mode)
  {
    error = wrapper_end_bulk_insert();
  } else {
    error = storage_end_bulk_insert();
  }
  DBUG_RETURN(error);
}

int ha_mroonga::wrapper_delete_all_rows()
{
  int error = 0;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  error = wrap_handler->ha_delete_all_rows();
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);

  if (!error && wrapper_have_target_index()) {
    error = wrapper_truncate_index();
  }

  DBUG_RETURN(error);
}

int ha_mroonga::storage_delete_all_rows()
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(storage_truncate());
}

int ha_mroonga::delete_all_rows()
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  if (share->wrapper_mode)
  {
    error = wrapper_delete_all_rows();
  } else {
    error = storage_delete_all_rows();
  }
  DBUG_RETURN(error);
}

#ifdef MRN_HANDLER_HAVE_TRUNCATE
int ha_mroonga::wrapper_truncate()
{
  int error = 0;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  error = wrap_handler->ha_truncate();
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);

  if (!error && wrapper_have_target_index()) {
    error = wrapper_truncate_index();
  }

  DBUG_RETURN(error);
}
#endif

int ha_mroonga::wrapper_truncate_index()
{
  MRN_DBUG_ENTER_METHOD();

  int error = 0;

  error = mrn_change_encoding(ctx, system_charset_info);
  if (error)
    DBUG_RETURN(error);

  if (is_dry_write()) {
    DBUG_PRINT("info", ("mroonga: dry write: ha_mroonga::%s", __FUNCTION__));
    DBUG_RETURN(error);
  }

  grn_rc rc;
  uint i;
  uint n_keys = table->s->keys;
  for (i = 0; i < n_keys; i++) {
    KEY key_info = table->key_info[i];

    if (!(wrapper_is_target_index(&key_info))) {
      continue;
    }

    rc = grn_table_truncate(ctx, grn_index_tables[i]);
    if (rc) {
      error = ER_ERROR_ON_WRITE;
      my_message(error, ctx->errbuf, MYF(0));
      goto err;
    }
  }
err:
  rc = grn_table_truncate(ctx, grn_table);
  if (rc) {
    error = ER_ERROR_ON_WRITE;
    my_message(error, ctx->errbuf, MYF(0));
  }

  DBUG_RETURN(error);
}

int ha_mroonga::storage_truncate()
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;

  if (is_dry_write()) {
    DBUG_PRINT("info", ("mroonga: dry write: ha_mroonga::%s", __FUNCTION__));
    DBUG_RETURN(error);
  }

  grn_rc rc;
  rc = grn_table_truncate(ctx, grn_table);
  if (rc) {
    my_message(ER_ERROR_ON_WRITE, ctx->errbuf, MYF(0));
    DBUG_RETURN(ER_ERROR_ON_WRITE);
  }
  error = storage_truncate_index();
  DBUG_RETURN(error);
}

int ha_mroonga::storage_truncate_index()
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;

  error = mrn_change_encoding(ctx, system_charset_info);
  if (error)
    DBUG_RETURN(error);

  grn_rc rc;
  uint i;
  uint n_keys = table->s->keys;
  for (i = 0; i < n_keys; i++) {
    if (i == table->s->primary_key) {
      continue;
    }

    KEY key_info = table->key_info[i];

    if (
      !(key_info.flags & HA_NOSAME) &&
      (key_info.key_parts == 1 || (key_info.flags & HA_FULLTEXT))
    ) {
      continue;
    }

    rc = grn_table_truncate(ctx, grn_index_tables[i]);
    if (rc) {
      error = ER_ERROR_ON_WRITE;
      my_message(error, ctx->errbuf, MYF(0));
      goto err;
    }
  }
err:
  DBUG_RETURN(error);
}

#ifdef MRN_HANDLER_HAVE_TRUNCATE
int ha_mroonga::truncate()
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  if (share->wrapper_mode)
  {
    error = wrapper_truncate();
  } else {
    error = storage_truncate();
  }
  DBUG_RETURN(error);
}
#endif

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

double ha_mroonga::storage_scan_time()
{
  MRN_DBUG_ENTER_METHOD();
  double time = handler::scan_time();
  DBUG_RETURN(time);
}

double ha_mroonga::scan_time()
{
  MRN_DBUG_ENTER_METHOD();
  double time;
  if (share->wrapper_mode)
  {
    time = wrapper_scan_time();
  } else {
    time = storage_scan_time();
  }
  DBUG_RETURN(time);
}

double ha_mroonga::wrapper_read_time(uint index, uint ranges, ha_rows rows)
{
  double res;
  MRN_DBUG_ENTER_METHOD();
  KEY key_info = table->key_info[index];
  if (mrn_is_geo_key(&key_info)) {
    res = handler::read_time(index, ranges, rows);
    DBUG_RETURN(res);
  }
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  res = wrap_handler->read_time(share->wrap_key_nr[index], ranges, rows);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(res);
}

double ha_mroonga::storage_read_time(uint index, uint ranges, ha_rows rows)
{
  MRN_DBUG_ENTER_METHOD();
  double time = handler::read_time(index, ranges, rows);
  DBUG_RETURN(time);
}

double ha_mroonga::read_time(uint index, uint ranges, ha_rows rows)
{
  MRN_DBUG_ENTER_METHOD();
  double time;
  if (share->wrapper_mode)
  {
    time = wrapper_read_time(index, ranges, rows);
  } else {
    time = storage_read_time(index, ranges, rows);
  }
  DBUG_RETURN(time);
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

const key_map *ha_mroonga::storage_keys_to_use_for_scanning()
{
  MRN_DBUG_ENTER_METHOD();
  const key_map *key_map;
  key_map = handler::keys_to_use_for_scanning();
  DBUG_RETURN(key_map);
}

const key_map *ha_mroonga::keys_to_use_for_scanning()
{
  MRN_DBUG_ENTER_METHOD();
  const key_map *key_map;
  if (share->wrapper_mode)
  {
    key_map = wrapper_keys_to_use_for_scanning();
  } else {
    key_map = storage_keys_to_use_for_scanning();
  }
  DBUG_RETURN(key_map);
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

ha_rows ha_mroonga::storage_estimate_rows_upper_bound()
{
  MRN_DBUG_ENTER_METHOD();
  ha_rows rows = handler::estimate_rows_upper_bound();
  DBUG_RETURN(rows);
}

ha_rows ha_mroonga::estimate_rows_upper_bound()
{
  MRN_DBUG_ENTER_METHOD();
  ha_rows rows;
  if (share->wrapper_mode)
  {
    rows = wrapper_estimate_rows_upper_bound();
  } else {
    rows = storage_estimate_rows_upper_bound();
  }
  DBUG_RETURN(rows);
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

void ha_mroonga::storage_update_create_info(HA_CREATE_INFO* create_info)
{
  MRN_DBUG_ENTER_METHOD();
  handler::update_create_info(create_info);
  DBUG_VOID_RETURN;
}

void ha_mroonga::update_create_info(HA_CREATE_INFO* create_info)
{
  MRN_DBUG_ENTER_METHOD();
  if (!create_info->connect_string.str)
  {
    create_info->connect_string.str = table->s->connect_string.str;
    create_info->connect_string.length = table->s->connect_string.length;
  }
  if (share->wrapper_mode)
    wrapper_update_create_info(create_info);
  else
    storage_update_create_info(create_info);
  st_mrn_slot_data *slot_data = mrn_get_slot_data(ha_thd(), true);
  if (slot_data) {
    slot_data->alter_create_info = create_info;
    if (slot_data->alter_connect_string) {
      my_free(slot_data->alter_connect_string, MYF(0));
      slot_data->alter_connect_string = NULL;
    }
    if (create_info->connect_string.str) {
      slot_data->alter_connect_string = mrn_create_string(
        create_info->connect_string.str, create_info->connect_string.length);
    }
    if (slot_data->alter_comment) {
      my_free(slot_data->alter_comment, MYF(0));
      slot_data->alter_comment = NULL;
    }
    if (create_info->comment.str) {
      slot_data->alter_comment = mrn_create_string(
        create_info->comment.str, create_info->comment.length);
    }
    if (share && share->disable_keys) {
      slot_data->disable_keys_create_info = create_info;
    }
  }
  DBUG_VOID_RETURN;
}

int ha_mroonga::wrapper_rename_table(const char *from, const char *to,
                                     MRN_SHARE *tmp_share,
                                     const char *from_table_name,
                                     const char *to_table_name)
{
  int error = 0;
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
  hnd->init();
  MRN_SET_BASE_SHARE_KEY(tmp_share, tmp_share->table_share);

  if ((error = hnd->ha_rename_table(from, to)))
  {
    delete hnd;
    DBUG_RETURN(error);
  }

  error = wrapper_rename_index(from, to, tmp_share,
                               from_table_name, to_table_name);

  delete hnd;
  DBUG_RETURN(error);
}

int ha_mroonga::wrapper_rename_index(const char *from, const char *to,
                                     MRN_SHARE *tmp_share,
                                     const char *from_table_name,
                                     const char *to_table_name)
{
  int error;
  grn_rc rc;
  MRN_DBUG_ENTER_METHOD();
  error = mrn_change_encoding(ctx, system_charset_info);
  if (error)
    DBUG_RETURN(error);

  error = ensure_database_open(from);
  if (error)
    DBUG_RETURN(error);

  TABLE_SHARE *tmp_table_share = tmp_share->table_share;

  uint i;
  for (i = 0; i < tmp_table_share->keys; i++) {
    const char *mysql_index_name = tmp_table_share->key_info[i].name;
    mrn::IndexTableName from_index_table_name(from_table_name, mysql_index_name);
    mrn::IndexTableName  to_index_table_name(to_table_name, mysql_index_name);
    grn_obj *index_table;
    index_table = grn_ctx_get(ctx,
                              from_index_table_name.c_str(),
                              from_index_table_name.length());
    if (index_table != NULL) {
      rc = grn_table_rename(ctx, index_table,
                            to_index_table_name.c_str(),
                            to_index_table_name.length());
      if (rc != GRN_SUCCESS) {
        error = ER_CANT_OPEN_FILE;
        my_message(error, ctx->errbuf, MYF(0));
        DBUG_RETURN(error);
      }
    }
  }

  grn_obj *table = grn_ctx_get(ctx, from_table_name, strlen(from_table_name));
  if (ctx->rc != GRN_SUCCESS) {
    error = ER_CANT_OPEN_FILE;
    my_message(error, ctx->errbuf, MYF(0));
    DBUG_RETURN(error);
  }
  rc = grn_table_rename(ctx, table, to_table_name,
                        strlen(to_table_name));
  if (rc != GRN_SUCCESS) {
    error = ER_CANT_OPEN_FILE;
    my_message(error, ctx->errbuf, MYF(0));
    DBUG_RETURN(error);
  }
  DBUG_RETURN(0);
}

int ha_mroonga::storage_rename_table(const char *from, const char *to,
                                     MRN_SHARE *tmp_share,
                                     const char *from_table_name,
                                     const char *to_table_name)
{
  int error;
  grn_rc rc;
  TABLE_SHARE *tmp_table_share = tmp_share->table_share;
  MRN_DBUG_ENTER_METHOD();
  error = mrn_change_encoding(ctx, system_charset_info);
  if (error)
    DBUG_RETURN(error);

  error = ensure_database_open(from);
  if (error)
    DBUG_RETURN(error);

  uint i;
  for (i = 0; i < tmp_table_share->keys; i++) {
    const char *mysql_index_name = tmp_table_share->key_info[i].name;
    mrn::IndexTableName from_index_table_name(from_table_name,
                                              mysql_index_name);
    mrn::IndexTableName to_index_table_name(to_table_name,
                                            mysql_index_name);
    grn_obj *index_table;
    index_table = grn_ctx_get(ctx,
                              from_index_table_name.c_str(),
                              from_index_table_name.length());
    if (index_table != NULL) {
      rc = grn_table_rename(ctx, index_table,
                            to_index_table_name.c_str(),
                            to_index_table_name.length());
      if (rc != GRN_SUCCESS) {
        error = ER_CANT_OPEN_FILE;
        my_message(error, ctx->errbuf, MYF(0));
        DBUG_RETURN(error);
      }
    }
  }

  grn_obj *table_obj = grn_ctx_get(ctx, from_table_name, strlen(from_table_name));
  if (ctx->rc != GRN_SUCCESS) {
    error = ER_CANT_OPEN_FILE;
    my_message(error, ctx->errbuf, MYF(0));
    DBUG_RETURN(error);
  }
  rc = grn_table_rename(ctx, table_obj, to_table_name,
                        strlen(to_table_name));
  if (rc != GRN_SUCCESS) {
    error = ER_CANT_OPEN_FILE;
    my_message(error, ctx->errbuf, MYF(0));
    DBUG_RETURN(error);
  }
  DBUG_RETURN(0);
}

int ha_mroonga::rename_table(const char *from, const char *to)
{
  int error = 0;
  TABLE_LIST table_list;
  TABLE_SHARE *tmp_table_share;
  TABLE tmp_table;
  MRN_SHARE *tmp_share;
  MRN_DBUG_ENTER_METHOD();
  mrn::PathMapper to_mapper(to, mrn_database_path_prefix);
  mrn::PathMapper from_mapper(from, mrn_database_path_prefix);
  if (strcmp(from_mapper.db_name(), to_mapper.db_name()))
    DBUG_RETURN(HA_ERR_WRONG_COMMAND);

#ifdef MRN_TABLE_LIST_INIT_REQUIRE_ALIAS
  table_list.init_one_table(from_mapper.db_name(),
                            strlen(from_mapper.db_name()),
                            from_mapper.mysql_table_name(),
                            strlen(from_mapper.mysql_table_name()),
                            from_mapper.mysql_table_name(), TL_WRITE);
#else
  table_list.init_one_table(from_mapper.db_name(),
                            from_mapper.mysql_table_name(),
                            TL_WRITE);
#endif
  mrn_open_mutex_lock();
  tmp_table_share = mrn_create_tmp_table_share(&table_list, from, &error);
  mrn_open_mutex_unlock();
  if (!tmp_table_share) {
    DBUG_RETURN(error);
  }
  tmp_table.s = tmp_table_share;
#ifdef WITH_PARTITION_STORAGE_ENGINE
  tmp_table.part_info = NULL;
#endif
  if (!(tmp_share = mrn_get_share(from, &tmp_table, &error)))
  {
    mrn_open_mutex_lock();
    mrn_free_tmp_table_share(tmp_table_share);
    mrn_open_mutex_unlock();
    DBUG_RETURN(error);
  }

  if (tmp_share->wrapper_mode)
  {
    error = wrapper_rename_table(from, to, tmp_share,
                                 from_mapper.table_name(),
                                 to_mapper.table_name());
  } else {
    error = storage_rename_table(from, to, tmp_share,
                                 from_mapper.table_name(),
                                 to_mapper.table_name());
  }

  mrn_free_share(tmp_share);
  if (!error && to_mapper.table_name()[0] == '#') {
    if ((error = alter_share_add(to, tmp_table_share)))
      DBUG_RETURN(error);
  } else if (error && from_mapper.table_name()[0] == '#') {
    alter_share_add(from, tmp_table_share);
  } else {
    mrn_open_mutex_lock();
    mrn_free_tmp_table_share(tmp_table_share);
    mrn_open_mutex_unlock();
  }
  DBUG_RETURN(error);
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

bool ha_mroonga::storage_is_crashed() const
{
  MRN_DBUG_ENTER_METHOD();
  bool crashed = handler::is_crashed();
  DBUG_RETURN(crashed);
}

bool ha_mroonga::is_crashed() const
{
  MRN_DBUG_ENTER_METHOD();
  int crashed;
  if (share->wrapper_mode)
  {
    crashed = wrapper_is_crashed();
  } else {
    crashed = storage_is_crashed();
  }
  DBUG_RETURN(crashed);
}

bool ha_mroonga::wrapper_auto_repair(int error) const
{
  bool crashed;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
#ifdef MRN_HANDLER_AUTO_REPAIR_HAVE_ERROR
  crashed = wrap_handler->auto_repair(error);
#else
  crashed = wrap_handler->auto_repair();
#endif
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(crashed);
}

bool ha_mroonga::storage_auto_repair(int error) const
{
  MRN_DBUG_ENTER_METHOD();
  bool crashed;
#ifdef MRN_HANDLER_AUTO_REPAIR_HAVE_ERROR
  crashed = handler::auto_repair(error);
#else
  crashed = handler::auto_repair();
#endif
  DBUG_RETURN(crashed);
}

bool ha_mroonga::auto_repair(int error) const
{
  MRN_DBUG_ENTER_METHOD();
  bool crashed;
  // TODO: We should consider about creating share for error =
  // ER_CANT_OPEN_FILE. The following code just ignores the error.
  if (share && share->wrapper_mode)
  {
    crashed = wrapper_auto_repair(error);
  } else {
    crashed = storage_auto_repair(error);
  }
  DBUG_RETURN(crashed);
}

bool ha_mroonga::auto_repair() const
{
  MRN_DBUG_ENTER_METHOD();
  bool crashed = auto_repair(HA_ERR_CRASHED_ON_USAGE);
  DBUG_RETURN(crashed);
}

int ha_mroonga::wrapper_disable_indexes(uint mode)
{
  int error = 0;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  error = wrap_handler->ha_disable_indexes(mode);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  if (error == HA_ERR_WRONG_COMMAND) {
    error = 0;
  }
  if (!error) {
    if (mode == HA_KEY_SWITCH_NONUNIQ_SAVE || mode == HA_KEY_SWITCH_ALL) {
      uint i;
      for (i = 0; i < table_share->keys; i++) {
        if (i == table->s->primary_key) {
          continue;
        }
        if (share->wrap_key_nr[i] < MAX_KEY) {
          continue;
        }
        if (!grn_index_tables[i]) {
          DBUG_PRINT("info", ("mroonga: keys are disabled already %u", i));
          DBUG_RETURN(0);
        }
      }
      KEY *key_info = table_share->key_info;
      mrn::PathMapper mapper(share->table_name, mrn_database_path_prefix);
      for (i = 0; i < table_share->keys; i++) {
        if (!(key_info[i].flags & HA_FULLTEXT) &&
          !mrn_is_geo_key(&key_info[i])) {
          continue;
        }

        mrn::IndexTableName index_table_name(mapper.table_name(),
                                             key_info[i].name);
        grn_obj *index_table = grn_ctx_get(ctx,
                                           index_table_name.c_str(),
                                           index_table_name.length());
        if (index_table != NULL) {
          grn_obj_remove(ctx, index_table);
        }
        grn_index_tables[i] = NULL;
        grn_index_columns[i] = NULL;
      }
    } else {
      error = HA_ERR_WRONG_COMMAND;
    }
  }
  DBUG_RETURN(error);
}

int ha_mroonga::storage_disable_indexes(uint mode)
{
  MRN_DBUG_ENTER_METHOD();
  if (mode == HA_KEY_SWITCH_NONUNIQ_SAVE || mode == HA_KEY_SWITCH_ALL) {
    uint i;
    for (i = 0; i < table_share->keys; i++) {
      if (i == table->s->primary_key) {
        continue;
      }
      if (!grn_index_tables[i]) {
        DBUG_PRINT("info", ("mroonga: keys are disabled already %u", i));
        DBUG_RETURN(0);
      }
    }
    KEY *key_info = table_share->key_info;
    mrn::PathMapper mapper(share->table_name, mrn_database_path_prefix);
    for (i = 0; i < table_share->keys; i++) {
      if (i == table->s->primary_key) {
        continue;
      }
      if (mode == HA_KEY_SWITCH_NONUNIQ_SAVE &&
          (key_info[i].flags & HA_NOSAME)) {
        continue;
      }

      mrn::IndexTableName index_table_name(mapper.table_name(),
                                           key_info[i].name);
      grn_obj *index_table = grn_ctx_get(ctx,
                                         index_table_name.c_str(),
                                         index_table_name.length());
      if (index_table != NULL) {
        grn_obj_remove(ctx, index_table);
      }
      grn_index_tables[i] = NULL;
      grn_index_columns[i] = NULL;
    }
  } else {
    DBUG_RETURN(HA_ERR_WRONG_COMMAND);
  }
  DBUG_RETURN(0);
}

int ha_mroonga::disable_indexes(uint mode)
{
  int error = 0;
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
  {
    error = wrapper_disable_indexes(mode);
  } else {
    error = storage_disable_indexes(mode);
  }
  DBUG_RETURN(error);
}

int ha_mroonga::wrapper_enable_indexes(uint mode)
{
  int error = 0, tmp_error = 0;
  MRN_DBUG_ENTER_METHOD();
  if (mode == HA_KEY_SWITCH_NONUNIQ_SAVE || mode == HA_KEY_SWITCH_ALL) {
    uint i, j;
    for (i = 0; i < table_share->keys; i++) {
      if (i == table->s->primary_key) {
        continue;
      }
      if (share->wrap_key_nr[i] < MAX_KEY) {
        continue;
      }
      if (!grn_index_tables[i]) {
        break;
      }
    }
    if (i == table_share->keys) {
      DBUG_PRINT("info", ("mroonga: keys are enabled already"));
      DBUG_RETURN(0);
    }
    KEY *p_key_info = &table->key_info[table_share->primary_key];
    KEY *key_info = table_share->key_info;
    uint n_keys = table_share->keys;
    grn_obj *index_tables[n_keys];
    grn_obj *index_columns[n_keys];
    bitmap_clear_all(table->read_set);
    mrn_set_bitmap_by_key(table->read_set, p_key_info);
    mrn::PathMapper mapper(share->table_name, mrn_database_path_prefix);
    for (i = 0, j = 0; i < n_keys; i++) {
      if (!(key_info[i].flags & HA_FULLTEXT) &&
        !mrn_is_geo_key(&key_info[i])) {
        j++;
        continue;
      }

      if ((error = mrn_add_index_param(share, &key_info[i], i)))
      {
        break;
      }
      index_tables[i] = NULL;
      index_columns[i] = NULL;
      if (!grn_index_tables[i]) {
        if (
          (key_info[i].flags & HA_FULLTEXT) &&
          (error = wrapper_create_index_fulltext(mapper.table_name(),
                                                 i, &key_info[i],
                                                 index_tables, index_columns,
                                                 share))
        ) {
          break;
        } else if (
          mrn_is_geo_key(&key_info[i]) &&
          (error = wrapper_create_index_geo(mapper.table_name(),
                                            i, &key_info[i],
                                            index_tables, index_columns,
                                            share))
        ) {
          break;
        }
        grn_index_columns[i] = index_columns[i];
      }
      mrn_set_bitmap_by_key(table->read_set, &key_info[i]);
    }
    if (!error && i > j)
    {
      error = wrapper_fill_indexes(ha_thd(), table->key_info, index_columns,
                                   n_keys);
    }
    bitmap_set_all(table->read_set);
  } else {
    tmp_error = HA_ERR_WRONG_COMMAND;
  }

  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  error = wrap_handler->ha_enable_indexes(mode);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  if (error == HA_ERR_WRONG_COMMAND) {
    error = tmp_error;
  }
  DBUG_RETURN(error);
}

int ha_mroonga::storage_enable_indexes(uint mode)
{
  int error = 0;
  uint n_keys = table_share->keys;
  grn_obj *index_tables[n_keys];
  grn_obj *index_columns[n_keys];
  bool have_multiple_column_index = false;
  bool skip_unique_key = (mode == HA_KEY_SWITCH_NONUNIQ_SAVE);
  MRN_DBUG_ENTER_METHOD();
  if (mode == HA_KEY_SWITCH_NONUNIQ_SAVE || mode == HA_KEY_SWITCH_ALL) {
    uint i;
    for (i = 0; i < table_share->keys; i++) {
      if (i == table->s->primary_key) {
        continue;
      }
      if (!grn_index_tables[i]) {
        break;
      }
    }
    if (i == table_share->keys) {
      DBUG_PRINT("info", ("mroonga: keys are enabled already"));
      DBUG_RETURN(0);
    }
    KEY *key_info = table_share->key_info;
    bitmap_clear_all(table->read_set);
    mrn::PathMapper mapper(share->table_name, mrn_database_path_prefix);
    for (i = 0; i < n_keys; i++) {
      if (i == table->s->primary_key) {
        continue;
      }
      if (skip_unique_key && (key_info[i].flags & HA_NOSAME)) {
        continue;
      }

      if ((error = mrn_add_index_param(share, &key_info[i], i)))
      {
        break;
      }
      index_tables[i] = NULL;
      if (!grn_index_tables[i]) {
        if ((error = storage_create_index(table, mapper.table_name(), grn_table,
                                          share, &key_info[i], index_tables,
                                          index_columns, i)))
        {
          break;
        }
        if (
          key_info[i].key_parts != 1 &&
          !(key_info[i].flags & HA_FULLTEXT)
        ) {
          mrn_set_bitmap_by_key(table->read_set, &key_info[i]);
          have_multiple_column_index = true;
        }
      } else {
        index_columns[i] = NULL;
      }
    }
    if (!error && have_multiple_column_index)
    {
      error = storage_add_index_multiple_columns(key_info, n_keys,
                                                 index_tables,
                                                 index_columns,
                                                 skip_unique_key);
    }
    bitmap_set_all(table->read_set);
  } else {
    DBUG_RETURN(HA_ERR_WRONG_COMMAND);
  }
  DBUG_RETURN(error);
}

int ha_mroonga::enable_indexes(uint mode)
{
  int error = 0;
  MRN_DBUG_ENTER_METHOD();
  share->disable_keys = FALSE;
  if (share->wrapper_mode)
  {
    error = wrapper_enable_indexes(mode);
  } else {
    error = storage_enable_indexes(mode);
  }
  DBUG_RETURN(error);
}

int ha_mroonga::wrapper_check(THD* thd, HA_CHECK_OPT* check_opt)
{
  int error = 0;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  error = wrap_handler->ha_check(thd, check_opt);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::storage_check(THD* thd, HA_CHECK_OPT* check_opt)
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(HA_ADMIN_NOT_IMPLEMENTED);
}

int ha_mroonga::check(THD* thd, HA_CHECK_OPT* check_opt)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  if (share->wrapper_mode)
  {
    error = wrapper_check(thd, check_opt);
  } else {
    error = storage_check(thd, check_opt);
  }
  DBUG_RETURN(error);
}

int ha_mroonga::wrapper_fill_indexes(THD *thd, KEY *key_info,
                                     grn_obj **index_columns, uint n_keys)
{
  int error = 0;
  KEY *p_key_info = &table->key_info[table_share->primary_key];
  KEY *tmp_key_info;
  MRN_DBUG_ENTER_METHOD();
  DBUG_PRINT("info", ("mroonga: n_keys=%u", n_keys));
  if (
    mrn_lock_type != F_UNLCK || !(error = wrapper_external_lock(thd, F_WRLCK))
  ) {
    if (
      !(error = wrapper_start_stmt(thd, thr_lock_data.type)) &&
      !(error = wrapper_rnd_init(true))
    ) {
      grn_obj key;
      GRN_TEXT_INIT(&key, 0);
      grn_bulk_space(ctx, &key, p_key_info->key_length);
      while (!(error = wrapper_rnd_next(table->record[0])))
      {
        key_copy((uchar *)(GRN_TEXT_VALUE(&key)), table->record[0],
                 p_key_info, p_key_info->key_length);
        int added;
        grn_id record_id;
        mrn_change_encoding(ctx, NULL);
        record_id = grn_table_add(ctx, grn_table,
                                  GRN_TEXT_VALUE(&key), GRN_TEXT_LEN(&key),
                                  &added);
        if (record_id == GRN_ID_NIL)
        {
          char error_message[MRN_MESSAGE_BUFFER_SIZE];
          snprintf(error_message, MRN_MESSAGE_BUFFER_SIZE,
                   "failed to add a new record into groonga: key=<%.*s>",
                   (int) GRN_TEXT_LEN(&key), GRN_TEXT_VALUE(&key));
          error = ER_ERROR_ON_WRITE;
          my_message(error, error_message, MYF(0));
        }
        grn_obj_unlink(ctx, &key);
        if (error)
          break;

        uint k;
        for (k = 0; k < n_keys; k++) {
          tmp_key_info = &key_info[k];
          if (!(tmp_key_info->flags & HA_FULLTEXT) &&
            !mrn_is_geo_key(tmp_key_info)) {
            continue;
          }
          if (!index_columns[k]) {
            continue;
          }

          uint l;
          for (l = 0; l < tmp_key_info->key_parts; l++) {
            Field *field = tmp_key_info->key_part[l].field;

            if (field->is_null())
              continue;
            error = mrn_change_encoding(ctx, field->charset());
            if (error)
              break;

            generic_store_bulk(field, &new_value_buffer);

            grn_obj *index_column = index_columns[k];
            grn_rc rc;
            rc = grn_column_index_update(ctx, index_column, record_id, l + 1,
                                         NULL, &new_value_buffer);
            grn_obj_unlink(ctx, index_column);
            if (rc) {
              error = ER_ERROR_ON_WRITE;
              my_message(error, ctx->errbuf, MYF(0));
              break;
            }
          }
          if (error)
            break;
        }
        if (error)
          break;
      }
      if (error != HA_ERR_END_OF_FILE)
        wrapper_rnd_end();
      else
        error = wrapper_rnd_end();
    }
    if (mrn_lock_type == F_UNLCK)
      wrapper_external_lock(thd, F_UNLCK);
  }
  DBUG_RETURN(error);
}

int ha_mroonga::wrapper_recreate_indexes(THD *thd)
{
  int error;
  uint i, n_keys = table_share->keys;
  KEY *p_key_info = &table->key_info[table_share->primary_key];
  KEY *key_info = table->key_info;
  MRN_DBUG_ENTER_METHOD();
  mrn::PathMapper mapper(table_share->normalized_path.str, mrn_database_path_prefix);
  bitmap_clear_all(table->read_set);
  clear_indexes();
  remove_grn_obj_force(mapper.table_name());
  grn_table = NULL;
  mrn_set_bitmap_by_key(table->read_set, p_key_info);
  for (i = 0; i < n_keys; i++) {
    if (!(key_info[i].flags & HA_FULLTEXT) && !mrn_is_geo_key(&key_info[i])) {
      continue;
    }
    mrn::IndexTableName index_table_name(mapper.table_name(),
                                         table_share->key_info[i].name);
    char index_column_full_name[MRN_MAX_PATH_SIZE];
    snprintf(index_column_full_name, MRN_MAX_PATH_SIZE,
             "%s.%s", index_table_name.c_str(), index_column_name);
    remove_grn_obj_force(index_column_full_name);
    remove_grn_obj_force(index_table_name.c_str());
    mrn_set_bitmap_by_key(table->read_set, &key_info[i]);
  }
  error = wrapper_create_index(table_share->normalized_path.str, table,
                               NULL, share, mapper.table_name());
  if (error)
    DBUG_RETURN(error);
  error = wrapper_open_indexes(table_share->normalized_path.str, false);
  if (error)
    DBUG_RETURN(error);
  error = wrapper_fill_indexes(thd, key_info, grn_index_columns, n_keys);
  bitmap_set_all(table->read_set);
  DBUG_RETURN(error);
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
  if (error && error != HA_ADMIN_NOT_IMPLEMENTED)
    DBUG_RETURN(error);
  error = wrapper_recreate_indexes(thd);
  DBUG_RETURN(error);
}

int ha_mroonga::storage_repair(THD* thd, HA_CHECK_OPT* check_opt)
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(HA_ADMIN_NOT_IMPLEMENTED);
}

int ha_mroonga::repair(THD* thd, HA_CHECK_OPT* check_opt)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  share->disable_keys = FALSE;
  if (share->wrapper_mode)
  {
    error = wrapper_repair(thd, check_opt);
  } else {
    error = storage_repair(thd, check_opt);
  }
  DBUG_RETURN(error);
}

bool ha_mroonga::wrapper_check_and_repair(THD *thd)
{
  // XXX: success is valid variable name?
  bool success;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  success = wrap_handler->ha_check_and_repair(thd);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(success);
}

bool ha_mroonga::storage_check_and_repair(THD *thd)
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(true);
}

bool ha_mroonga::check_and_repair(THD *thd)
{
  MRN_DBUG_ENTER_METHOD();
  // XXX: success is valid variable name?
  bool success;
  if (share->wrapper_mode)
  {
    success = wrapper_check_and_repair(thd);
  } else {
    success = storage_check_and_repair(thd);
  }
  DBUG_RETURN(success);
}

int ha_mroonga::wrapper_analyze(THD* thd, HA_CHECK_OPT* check_opt)
{
  int error = 0;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  error = wrap_handler->ha_analyze(thd, check_opt);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::storage_analyze(THD* thd, HA_CHECK_OPT* check_opt)
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(HA_ADMIN_NOT_IMPLEMENTED);
}

int ha_mroonga::analyze(THD* thd, HA_CHECK_OPT* check_opt)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  if (share->wrapper_mode)
  {
    error = wrapper_analyze(thd, check_opt);
  } else {
    error = storage_analyze(thd, check_opt);
  }
  DBUG_RETURN(error);
}

int ha_mroonga::wrapper_optimize(THD* thd, HA_CHECK_OPT* check_opt)
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(HA_ADMIN_TRY_ALTER);
}

int ha_mroonga::storage_optimize(THD* thd, HA_CHECK_OPT* check_opt)
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(HA_ADMIN_NOT_IMPLEMENTED);
}

int ha_mroonga::optimize(THD* thd, HA_CHECK_OPT* check_opt)
{
  MRN_DBUG_ENTER_METHOD();
  int error = 0;
  if (share->wrapper_mode)
  {
    error = wrapper_optimize(thd, check_opt);
  } else {
    error = storage_optimize(thd, check_opt);
  }
  DBUG_RETURN(error);
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

bool ha_mroonga::storage_is_fatal_error(int error_num, uint flags)
{
  MRN_DBUG_ENTER_METHOD();
  bool is_fatal_error = handler::is_fatal_error(error_num, flags);
  DBUG_RETURN(is_fatal_error);
}

bool ha_mroonga::is_fatal_error(int error_num, uint flags)
{
  MRN_DBUG_ENTER_METHOD();
  bool is_fatal_error;
  if (share->wrapper_mode)
  {
    is_fatal_error = wrapper_is_fatal_error(error_num, flags);
  } else {
    is_fatal_error = storage_is_fatal_error(error_num, flags);
  }
  DBUG_RETURN(is_fatal_error);
}

bool ha_mroonga::wrapper_check_if_incompatible_data(
  HA_CREATE_INFO *create_info, uint table_changes)
{
  bool res;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  res = wrap_handler->check_if_incompatible_data(create_info, table_changes);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(res);
}

bool ha_mroonga::storage_check_if_incompatible_data(
  HA_CREATE_INFO *create_info, uint table_changes)
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(COMPATIBLE_DATA_YES);
}

bool ha_mroonga::check_if_incompatible_data(
  HA_CREATE_INFO *create_info, uint table_changes)
{
  MRN_DBUG_ENTER_METHOD();
  bool res;
  if (
    create_info->comment.str != table_share->comment.str ||
    create_info->connect_string.str != table_share->connect_string.str
  ) {
    DBUG_RETURN(COMPATIBLE_DATA_NO);
  }
  if (share->wrapper_mode)
  {
    res = wrapper_check_if_incompatible_data(create_info, table_changes);
  } else {
    res = storage_check_if_incompatible_data(create_info, table_changes);
  }
  DBUG_RETURN(res);
}

uint ha_mroonga::wrapper_alter_table_flags(uint flags)
{
  uint res;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  res = wrap_handler->alter_table_flags(flags);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(res);
}

uint ha_mroonga::storage_alter_table_flags(uint flags)
{
  MRN_DBUG_ENTER_METHOD();
  uint res = handler::alter_table_flags(flags);
  DBUG_RETURN(res);
}

uint ha_mroonga::alter_table_flags(uint flags)
{
  MRN_DBUG_ENTER_METHOD();
  uint res;
  if (share->wrapper_mode)
  {
    res = wrapper_alter_table_flags(flags);
  } else {
    res = storage_alter_table_flags(flags);
  }
  DBUG_RETURN(res);
}

#ifdef MRN_HANDLER_HAVE_FINAL_ADD_INDEX
int ha_mroonga::wrapper_add_index(TABLE *table_arg, KEY *key_info,
                                  uint num_of_keys, handler_add_index **add)
#else
int ha_mroonga::wrapper_add_index(TABLE *table_arg, KEY *key_info,
                                  uint num_of_keys)
#endif
{
  int error = 0;
  uint i, j, k;
  uint n_keys = table->s->keys;
  grn_obj *index_tables[num_of_keys + n_keys];
  grn_obj *index_columns[num_of_keys + n_keys];
  THD *thd = ha_thd();
  MRN_SHARE *tmp_share;
  TABLE_SHARE tmp_table_share;
  char **key_parser;
  uint *key_parser_length;
  MRN_DBUG_ENTER_METHOD();
  KEY *wrap_key_info = (KEY *) thd->alloc(sizeof(KEY) * num_of_keys);
  KEY *p_key_info = &table->key_info[table_share->primary_key], *tmp_key_info;
  tmp_table_share.keys = n_keys + num_of_keys;
  if (!(tmp_share = (MRN_SHARE *)
    my_multi_malloc(MYF(MY_WME | MY_ZEROFILL),
      &tmp_share, sizeof(*tmp_share),
      &key_parser, sizeof(char *) * (n_keys + num_of_keys),
      &key_parser_length, sizeof(uint) * (n_keys + num_of_keys),
      NullS))
  ) {
    DBUG_RETURN(HA_ERR_OUT_OF_MEM);
  }
  tmp_share->engine = NULL;
  tmp_share->table_share = &tmp_table_share;
  tmp_share->key_parser = key_parser;
  tmp_share->key_parser_length = key_parser_length;
#ifdef MRN_HANDLER_HAVE_FINAL_ADD_INDEX
  hnd_add_index = NULL;
#endif
  bitmap_clear_all(table->read_set);
  mrn_set_bitmap_by_key(table->read_set, p_key_info);
  mrn::PathMapper mapper(share->table_name, mrn_database_path_prefix);
  for (i = 0, j = 0; i < num_of_keys; i++) {
    if (!(key_info[i].flags & HA_FULLTEXT) && !mrn_is_geo_key(&key_info[i])) {
      wrap_key_info[j] = key_info[i];
      j++;
      continue;
    }
    if (share->disable_keys) {
      continue;
    }
    if ((error = mrn_add_index_param(tmp_share, &key_info[i], i + n_keys)))
    {
      break;
    }
    index_tables[i + n_keys] = NULL;
    if (
      (key_info[i].flags & HA_FULLTEXT) &&
      (error = wrapper_create_index_fulltext(mapper.table_name(),
                                             i + n_keys,
                                             &key_info[i], index_tables, NULL,
                                             tmp_share))
    ) {
      break;
    } else if (
      mrn_is_geo_key(&key_info[i]) &&
      (error = wrapper_create_index_geo(mapper.table_name(),
                                        i + n_keys, &key_info[i],
                                        index_tables, NULL, tmp_share))
    ) {
      break;
    }
    mrn_set_bitmap_by_key(table->read_set, &key_info[i]);
  }
  if (!error && i > j && !share->disable_keys) {
    for (k = 0; k < num_of_keys; k++) {
      tmp_key_info = &key_info[k];
      if (!(tmp_key_info->flags & HA_FULLTEXT) &&
        !mrn_is_geo_key(tmp_key_info)) {
        continue;
      }
      index_columns[k + n_keys] = grn_obj_column(ctx,
                                                 index_tables[k + n_keys],
                                                 index_column_name,
                                                 strlen(index_column_name));
    }
    error = wrapper_fill_indexes(thd, key_info, &index_columns[n_keys],
                                 num_of_keys);
  }
  bitmap_set_all(table->read_set);

  if (!error && j)
  {
    MRN_SET_WRAP_SHARE_KEY(share, table->s);
    MRN_SET_WRAP_TABLE_KEY(this, table);
#ifdef MRN_HANDLER_HAVE_FINAL_ADD_INDEX
    error = wrap_handler->add_index(table_arg, wrap_key_info, j, &hnd_add_index);
#else
    error = wrap_handler->add_index(table_arg, wrap_key_info, j);
#endif
    MRN_SET_BASE_SHARE_KEY(share, table->s);
    MRN_SET_BASE_TABLE_KEY(this, table);
  }
  if (error)
  {
    for (k = 0; k < i; k++) {
      if (!(key_info[k].flags & HA_FULLTEXT) && !mrn_is_geo_key(&key_info[k]))
      {
        continue;
      }
      if (index_tables[k + n_keys])
      {
        grn_obj_remove(ctx, index_tables[k + n_keys]);
      }
    }
  }
#ifdef MRN_HANDLER_HAVE_FINAL_ADD_INDEX
  else {
    *add = new handler_add_index(table_arg, key_info, num_of_keys);
  }
#endif
  mrn_free_share_alloc(tmp_share);
  my_free(tmp_share, MYF(0));
  DBUG_RETURN(error);
}

#ifdef MRN_HANDLER_HAVE_FINAL_ADD_INDEX
int ha_mroonga::storage_add_index(TABLE *table_arg, KEY *key_info,
                                  uint num_of_keys, handler_add_index **add)
#else
int ha_mroonga::storage_add_index(TABLE *table_arg, KEY *key_info,
                                  uint num_of_keys)
#endif
{
  int error = 0;
  uint i;
  uint n_keys = table->s->keys;
  grn_obj *index_tables[num_of_keys + n_keys];
  grn_obj *index_columns[num_of_keys + n_keys];
  MRN_SHARE *tmp_share;
  TABLE_SHARE tmp_table_share;
  char **key_parser;
  uint *key_parser_length;
  bool have_multiple_column_index = false;

  MRN_DBUG_ENTER_METHOD();
  tmp_table_share.keys = n_keys + num_of_keys;
  if (!(tmp_share = (MRN_SHARE *)
    my_multi_malloc(MYF(MY_WME | MY_ZEROFILL),
      &tmp_share, sizeof(*tmp_share),
      &key_parser, sizeof(char *) * (n_keys + num_of_keys),
      &key_parser_length, sizeof(uint) * (n_keys + num_of_keys),
      NullS))
  ) {
    DBUG_RETURN(HA_ERR_OUT_OF_MEM);
  }
  tmp_share->engine = NULL;
  tmp_share->table_share = &tmp_table_share;
  tmp_share->key_parser = key_parser;
  tmp_share->key_parser_length = key_parser_length;
  bitmap_clear_all(table->read_set);
  mrn::PathMapper mapper(share->table_name, mrn_database_path_prefix);
  for (i = 0; i < num_of_keys; i++) {
    if (share->disable_keys && !(key_info[i].flags & HA_NOSAME)) {
      continue; // key is disabled
    }
    index_tables[i + n_keys] = NULL;
    index_columns[i + n_keys] = NULL;
    if ((error = mrn_add_index_param(tmp_share, &key_info[i], i + n_keys)))
    {
      break;
    }
    if ((error = storage_create_index(table, mapper.table_name(), grn_table,
                                      tmp_share, &key_info[i], index_tables,
                                      index_columns, i + n_keys)))
    {
      break;
    }
    if (
      key_info[i].key_parts == 1 &&
      (key_info[i].flags & HA_NOSAME) &&
      grn_table_size(ctx, grn_table) !=
        grn_table_size(ctx, index_tables[i + n_keys])
    ) {
      error = HA_ERR_FOUND_DUPP_UNIQUE;
      i++;
      break;
    }
    if (
      key_info[i].key_parts != 1 &&
      !(key_info[i].flags & HA_FULLTEXT)
    ) {
      mrn_set_bitmap_by_key(table->read_set, &key_info[i]);
      have_multiple_column_index = true;
    }
  }
  if (!error && have_multiple_column_index)
  {
    error = storage_add_index_multiple_columns(key_info, num_of_keys,
                                               index_tables + n_keys,
                                               index_columns + n_keys, FALSE);
  }
  bitmap_set_all(table->read_set);
  if (error)
  {
    for (uint j = 0; j < i; j++) {
      if (index_tables[j + n_keys])
      {
        grn_obj_remove(ctx, index_columns[j + n_keys]);
        grn_obj_remove(ctx, index_tables[j + n_keys]);
      }
    }
  }
#ifdef MRN_HANDLER_HAVE_FINAL_ADD_INDEX
  else {
    *add = new handler_add_index(table_arg, key_info, num_of_keys);
  }
#endif
  mrn_free_share_alloc(tmp_share);
  my_free(tmp_share, MYF(0));
  DBUG_RETURN(error);
}

int ha_mroonga::storage_add_index_multiple_columns(KEY *key_info,
                                                   uint num_of_keys,
                                                   grn_obj **index_tables,
                                                   grn_obj **index_columns,
                                                   bool skip_unique_key)
{
  MRN_DBUG_ENTER_METHOD();

  int error = 0;

  if (!(error = storage_rnd_init(true)))
  {
    while (!(error = storage_rnd_next(table->record[0])))
    {
      for (uint i = 0; i < num_of_keys; i++) {
        KEY *current_key_info = key_info + i;
        if (
          current_key_info->key_parts == 1 ||
          (current_key_info->flags & HA_FULLTEXT)
          ) {
          continue;
        }
        if (skip_unique_key && (key_info[i].flags & HA_NOSAME)) {
          continue;
        }
        if (!index_columns[i]) {
          continue;
        }

        /* fix key_info.key_length */
        for (uint j = 0; j < current_key_info->key_parts; j++) {
          if (
            !current_key_info->key_part[j].null_bit &&
            current_key_info->key_part[j].field->null_bit
            ) {
            current_key_info->key_length++;
            current_key_info->key_part[j].null_bit =
              current_key_info->key_part[j].field->null_bit;
          }
        }
        if (key_info[i].flags & HA_NOSAME) {
          grn_id key_id;
          if ((error = storage_write_row_unique_index(table->record[0],
                                                      record_id,
                                                      current_key_info,
                                                      index_tables[i],
                                                      &key_id)))
          {
            if (error == HA_ERR_FOUND_DUPP_KEY)
            {
              error = HA_ERR_FOUND_DUPP_UNIQUE;
            }
            break;
          }
        }
        if ((error = storage_write_row_multiple_column_index(table->record[0],
                                                             record_id,
                                                             current_key_info,
                                                             index_columns[i])))
        {
          break;
        }
      }
      if (error)
        break;
    }
    if (error != HA_ERR_END_OF_FILE) {
      storage_rnd_end();
    } else {
      error = storage_rnd_end();
    }
  }

  DBUG_RETURN(error);
}

#ifdef MRN_HANDLER_HAVE_FINAL_ADD_INDEX
int ha_mroonga::add_index(TABLE *table_arg, KEY *key_info,
                          uint num_of_keys, handler_add_index **add)
{
  MRN_DBUG_ENTER_METHOD();
  int error;
  if (share->wrapper_mode)
  {
    error = wrapper_add_index(table_arg, key_info, num_of_keys, add);
  } else {
    error = storage_add_index(table_arg, key_info, num_of_keys, add);
  }
  DBUG_RETURN(error);
}
#else
int ha_mroonga::add_index(TABLE *table_arg, KEY *key_info,
                          uint num_of_keys)
{
  MRN_DBUG_ENTER_METHOD();
  int error;
  if (share->wrapper_mode)
  {
    error = wrapper_add_index(table_arg, key_info, num_of_keys);
  } else {
    error = storage_add_index(table_arg, key_info, num_of_keys);
  }
  DBUG_RETURN(error);
}
#endif

#ifdef MRN_HANDLER_HAVE_FINAL_ADD_INDEX
int ha_mroonga::wrapper_final_add_index(handler_add_index *add, bool commit)
{
  int error = 0;
  MRN_DBUG_ENTER_METHOD();
  if (hnd_add_index)
  {
    MRN_SET_WRAP_SHARE_KEY(share, table->s);
    MRN_SET_WRAP_TABLE_KEY(this, table);
    error = wrap_handler->final_add_index(hnd_add_index, commit);
    MRN_SET_BASE_SHARE_KEY(share, table->s);
    MRN_SET_BASE_TABLE_KEY(this, table);
  }
  if (add)
  {
    delete add;
  }
  DBUG_RETURN(error);
}

int ha_mroonga::storage_final_add_index(handler_add_index *add, bool commit)
{
  MRN_DBUG_ENTER_METHOD();
  if (add)
  {
    delete add;
  }
  DBUG_RETURN(0);
}

int ha_mroonga::final_add_index(handler_add_index *add, bool commit)
{
  MRN_DBUG_ENTER_METHOD();
  int error;
  if (share->wrapper_mode)
  {
    error = wrapper_final_add_index(add, commit);
  } else {
    error = storage_final_add_index(add, commit);
  }
  DBUG_RETURN(error);
}
#endif

int ha_mroonga::wrapper_prepare_drop_index(TABLE *table_arg, uint *key_num,
  uint num_of_keys)
{
  int res = 0;
  uint wrap_key_num[num_of_keys], i, j;
  KEY *key_info = table_share->key_info;
  MRN_DBUG_ENTER_METHOD();
  res = mrn_change_encoding(ctx, system_charset_info);
  if (res)
    DBUG_RETURN(res);

  mrn::PathMapper mapper(share->table_name, mrn_database_path_prefix);
  for (i = 0, j = 0; i < num_of_keys; i++) {
    if (!(key_info[key_num[i]].flags & HA_FULLTEXT) &&
      !mrn_is_geo_key(&key_info[key_num[i]])) {
      wrap_key_num[j] = share->wrap_key_nr[key_num[i]];
      j++;
      continue;
    }

    mrn::IndexTableName index_table_name(mapper.table_name(),
                                         key_info[key_num[i]].name);
    grn_obj *index_table = grn_ctx_get(ctx,
                                       index_table_name.c_str(),
                                       index_table_name.length());
    if (index_table != NULL) {
      grn_obj_remove(ctx, index_table);
    }
    grn_index_tables[key_num[i]] = NULL;
    grn_index_columns[key_num[i]] = NULL;
  }
  if (j)
  {
    MRN_SET_WRAP_SHARE_KEY(share, table->s);
    MRN_SET_WRAP_TABLE_KEY(this, table);
    res = wrap_handler->prepare_drop_index(table_arg, wrap_key_num, j);
    MRN_SET_BASE_SHARE_KEY(share, table->s);
    MRN_SET_BASE_TABLE_KEY(this, table);
  }
  DBUG_RETURN(res);
}

int ha_mroonga::storage_prepare_drop_index(TABLE *table_arg, uint *key_num,
                                           uint num_of_keys)
{
  int error;
  uint i;
  KEY *key_info = table_share->key_info;
  MRN_DBUG_ENTER_METHOD();
  error = mrn_change_encoding(ctx, system_charset_info);
  if (error)
    DBUG_RETURN(error);

  mrn::PathMapper mapper(share->table_name, mrn_database_path_prefix);
  for (i = 0; i < num_of_keys; i++) {
    mrn::IndexTableName index_table_name(mapper.table_name(),
                                         key_info[key_num[i]].name);
    grn_obj *index_table = grn_ctx_get(ctx,
                                       index_table_name.c_str(),
                                       index_table_name.length());
    if (index_table != NULL) {
      grn_obj_remove(ctx, index_table);
    }
    grn_index_tables[key_num[i]] = NULL;
    grn_index_columns[key_num[i]] = NULL;
  }
  DBUG_RETURN(0);
}

int ha_mroonga::prepare_drop_index(TABLE *table_arg, uint *key_num,
  uint num_of_keys)
{
  MRN_DBUG_ENTER_METHOD();
  int res;
  if (share->wrapper_mode)
  {
    res = wrapper_prepare_drop_index(table_arg, key_num, num_of_keys);
  } else {
    res = storage_prepare_drop_index(table_arg, key_num, num_of_keys);
  }
  DBUG_RETURN(res);
}

int ha_mroonga::wrapper_final_drop_index(TABLE *table_arg)
{
  uint res;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  res = wrap_handler->final_drop_index(table_arg);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(res);
}

int ha_mroonga::storage_final_drop_index(TABLE *table_arg)
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(0);
}

int ha_mroonga::final_drop_index(TABLE *table_arg)
{
  MRN_DBUG_ENTER_METHOD();
  uint res;
  if (share->wrapper_mode)
  {
    res = wrapper_final_drop_index(table_arg);
  } else {
    res = storage_final_drop_index(table_arg);
  }
  DBUG_RETURN(res);
}

int ha_mroonga::wrapper_update_auto_increment()
{
  int res;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  res = wrap_handler->update_auto_increment();
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(res);
}

int ha_mroonga::storage_update_auto_increment()
{
  MRN_DBUG_ENTER_METHOD();
  int res = handler::update_auto_increment();
  DBUG_RETURN(res);
}

int ha_mroonga::update_auto_increment()
{
  MRN_DBUG_ENTER_METHOD();
  int res;
  if (share->wrapper_mode)
  {
    res = wrapper_update_auto_increment();
  } else {
    res = storage_update_auto_increment();
  }
  DBUG_RETURN(res);
}

void ha_mroonga::wrapper_set_next_insert_id(ulonglong id)
{
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  wrap_handler->set_next_insert_id(id);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_VOID_RETURN;
}

void ha_mroonga::storage_set_next_insert_id(ulonglong id)
{
  MRN_DBUG_ENTER_METHOD();
  handler::set_next_insert_id(id);
  DBUG_VOID_RETURN;
}

void ha_mroonga::set_next_insert_id(ulonglong id)
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
  {
    wrapper_set_next_insert_id(id);
  } else {
    storage_set_next_insert_id(id);
  }
  DBUG_VOID_RETURN;
}

void ha_mroonga::wrapper_get_auto_increment(ulonglong offset,
                                            ulonglong increment,
                                            ulonglong nb_desired_values,
                                            ulonglong *first_value,
                                            ulonglong *nb_reserved_values)
{
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  wrap_handler->get_auto_increment(offset, increment, nb_desired_values,
                                      first_value, nb_reserved_values);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_VOID_RETURN;
}

void ha_mroonga::storage_get_auto_increment(ulonglong offset,
                                            ulonglong increment,
                                            ulonglong nb_desired_values,
                                            ulonglong *first_value,
                                            ulonglong *nb_reserved_values)
{
  MRN_DBUG_ENTER_METHOD();
  handler::get_auto_increment(offset, increment, nb_desired_values,
                              first_value, nb_reserved_values);
  DBUG_VOID_RETURN;
}

void ha_mroonga::get_auto_increment(ulonglong offset, ulonglong increment,
                                    ulonglong nb_desired_values,
                                    ulonglong *first_value,
                                    ulonglong *nb_reserved_values)
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
  {
    wrapper_get_auto_increment(offset, increment, nb_desired_values,
                               first_value, nb_reserved_values);
  } else {
    storage_get_auto_increment(offset, increment, nb_desired_values,
                               first_value, nb_reserved_values);
  }
  DBUG_VOID_RETURN;
}

void ha_mroonga::wrapper_restore_auto_increment(ulonglong prev_insert_id)
{
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  wrap_handler->restore_auto_increment(prev_insert_id);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_VOID_RETURN;
}

void ha_mroonga::storage_restore_auto_increment(ulonglong prev_insert_id)
{
  MRN_DBUG_ENTER_METHOD();
  handler::restore_auto_increment(prev_insert_id);
  DBUG_VOID_RETURN;
}

void ha_mroonga::restore_auto_increment(ulonglong prev_insert_id)
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
  {
    wrapper_restore_auto_increment(prev_insert_id);
  } else {
    storage_restore_auto_increment(prev_insert_id);
  }
  DBUG_VOID_RETURN;
}

void ha_mroonga::wrapper_release_auto_increment()
{
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  wrap_handler->ha_release_auto_increment();
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_VOID_RETURN;
}

void ha_mroonga::storage_release_auto_increment()
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_VOID_RETURN;
}

void ha_mroonga::release_auto_increment()
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
  {
    wrapper_release_auto_increment();
  } else {
    storage_release_auto_increment();
  }
  DBUG_VOID_RETURN;
}

int ha_mroonga::wrapper_check_for_upgrade(HA_CHECK_OPT *check_opt)
{
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  int error = wrap_handler->ha_check_for_upgrade(check_opt);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(error);
}

int ha_mroonga::storage_check_for_upgrade(HA_CHECK_OPT *check_opt)
{
  MRN_DBUG_ENTER_METHOD();
  for (uint i = 0; i < table->s->fields; ++i) {
    grn_obj *column = grn_columns[i];
    if (!column) {
      continue;
    }
    Field *field = table->field[i];
    grn_id column_range = grn_obj_get_range(ctx, column);
    switch (field->real_type()) {
    case MYSQL_TYPE_ENUM:
      if (column_range != GRN_DB_UINT16) {
        DBUG_RETURN(HA_ADMIN_NEEDS_ALTER);
      }
      break;
    case MYSQL_TYPE_SET:
      if (column_range != GRN_DB_UINT64) {
        DBUG_RETURN(HA_ADMIN_NEEDS_ALTER);
      }
      break;
    default:
      break;
    }
  }
  DBUG_RETURN(HA_ADMIN_OK);
}

int ha_mroonga::check_for_upgrade(HA_CHECK_OPT *check_opt)
{
  MRN_DBUG_ENTER_METHOD();
  int error;
  if (share->wrapper_mode) {
    error = wrapper_check_for_upgrade(check_opt);
  } else {
    error = storage_check_for_upgrade(check_opt);
  }
  DBUG_RETURN(error);
}

int ha_mroonga::wrapper_reset_auto_increment(ulonglong value)
{
  int res;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  res = wrap_handler->ha_reset_auto_increment(value);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(res);
}

int ha_mroonga::storage_reset_auto_increment(ulonglong value)
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_RETURN(HA_ERR_WRONG_COMMAND);
}

int ha_mroonga::reset_auto_increment(ulonglong value)
{
  MRN_DBUG_ENTER_METHOD();
  int res;
  if (share->wrapper_mode)
  {
    res = wrapper_reset_auto_increment(value);
  } else {
    res = storage_reset_auto_increment(value);
  }
  DBUG_RETURN(res);
}

void ha_mroonga::set_pk_bitmap()
{
  KEY key_info = table->key_info[table_share->primary_key];
  uint j;
  MRN_DBUG_ENTER_METHOD();
  for (j = 0; j < key_info.key_parts; j++) {
    Field *field = key_info.key_part[j].field;
    bitmap_set_bit(table->read_set, field->field_index);
  }
  DBUG_VOID_RETURN;
}

bool ha_mroonga::wrapper_was_semi_consistent_read()
{
  bool res;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  res = wrap_handler->was_semi_consistent_read();
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(res);
}

bool ha_mroonga::storage_was_semi_consistent_read()
{
  bool res;
  MRN_DBUG_ENTER_METHOD();
  res = handler::was_semi_consistent_read();
  DBUG_RETURN(res);
}

bool ha_mroonga::was_semi_consistent_read()
{
  bool res;
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
  {
    res = wrapper_was_semi_consistent_read();
  } else {
    res = storage_was_semi_consistent_read();
  }
  DBUG_RETURN(res);
}

void ha_mroonga::wrapper_try_semi_consistent_read(bool yes)
{
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  wrap_handler->try_semi_consistent_read(yes);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_VOID_RETURN;
}

void ha_mroonga::storage_try_semi_consistent_read(bool yes)
{
  MRN_DBUG_ENTER_METHOD();
  handler::try_semi_consistent_read(yes);
  DBUG_VOID_RETURN;
}

void ha_mroonga::try_semi_consistent_read(bool yes)
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
  {
    wrapper_try_semi_consistent_read(yes);
  } else {
    storage_try_semi_consistent_read(yes);
  }
  DBUG_VOID_RETURN;
}

void ha_mroonga::wrapper_unlock_row()
{
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  wrap_handler->unlock_row();
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_VOID_RETURN;
}

void ha_mroonga::storage_unlock_row()
{
  MRN_DBUG_ENTER_METHOD();
  handler::unlock_row();
  DBUG_VOID_RETURN;
}

void ha_mroonga::unlock_row()
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
  {
    wrapper_unlock_row();
  } else {
    storage_unlock_row();
  }
  DBUG_VOID_RETURN;
}

int ha_mroonga::wrapper_start_stmt(THD *thd, thr_lock_type lock_type)
{
  int res;
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  res = wrap_handler->start_stmt(thd, lock_type);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(res);
}

int ha_mroonga::storage_start_stmt(THD *thd, thr_lock_type lock_type)
{
  int res;
  MRN_DBUG_ENTER_METHOD();
  res = handler::start_stmt(thd, lock_type);
  DBUG_RETURN(res);
}

int ha_mroonga::start_stmt(THD *thd, thr_lock_type lock_type)
{
  int res;
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
  {
    res = wrapper_start_stmt(thd, lock_type);
  } else {
    res = storage_start_stmt(thd, lock_type);
  }
  DBUG_RETURN(res);
}

void ha_mroonga::wrapper_change_table_ptr(TABLE *table_arg,
                                          TABLE_SHARE *share_arg)
{
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  wrap_handler->change_table_ptr(table_arg, share->wrap_table_share);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_VOID_RETURN;
}

void ha_mroonga::storage_change_table_ptr(TABLE *table_arg,
                                          TABLE_SHARE *share_arg)
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_VOID_RETURN;
}

void ha_mroonga::change_table_ptr(TABLE *table_arg, TABLE_SHARE *share_arg)
{
  MRN_DBUG_ENTER_METHOD();
  handler::change_table_ptr(table_arg, share_arg);
  if (share && share->wrapper_mode)
  {
    wrapper_change_table_ptr(table_arg, share_arg);
  } else {
    storage_change_table_ptr(table_arg, share_arg);
  }
  DBUG_VOID_RETURN;
}

bool ha_mroonga::wrapper_primary_key_is_clustered()
{
  MRN_DBUG_ENTER_METHOD();
  bool is_clustered;
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  is_clustered = wrap_handler->primary_key_is_clustered();
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(is_clustered);
}

bool ha_mroonga::storage_primary_key_is_clustered()
{
  MRN_DBUG_ENTER_METHOD();
  bool is_clustered = handler::primary_key_is_clustered();
  DBUG_RETURN(is_clustered);
}

bool ha_mroonga::primary_key_is_clustered()
{
  MRN_DBUG_ENTER_METHOD();
  bool is_clustered;
  if (share->wrapper_mode)
  {
    is_clustered = wrapper_primary_key_is_clustered();
  } else {
    is_clustered = storage_primary_key_is_clustered();
  }
  DBUG_RETURN(is_clustered);
}

bool ha_mroonga::wrapper_is_fk_defined_on_table_or_index(uint index)
{
  MRN_DBUG_ENTER_METHOD();
  bool res;
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  res = wrap_handler->is_fk_defined_on_table_or_index(index);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(res);
}

bool ha_mroonga::storage_is_fk_defined_on_table_or_index(uint index)
{
  MRN_DBUG_ENTER_METHOD();
  bool res = handler::is_fk_defined_on_table_or_index(index);
  DBUG_RETURN(res);
}

bool ha_mroonga::is_fk_defined_on_table_or_index(uint index)
{
  MRN_DBUG_ENTER_METHOD();
  bool res;
  if (share->wrapper_mode)
  {
    res = wrapper_is_fk_defined_on_table_or_index(index);
  } else {
    res = storage_is_fk_defined_on_table_or_index(index);
  }
  DBUG_RETURN(res);
}

char *ha_mroonga::wrapper_get_foreign_key_create_info()
{
  MRN_DBUG_ENTER_METHOD();
  char *res;
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  res = wrap_handler->get_foreign_key_create_info();
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(res);
}

char *ha_mroonga::storage_get_foreign_key_create_info()
{
  MRN_DBUG_ENTER_METHOD();
  char *res = handler::get_foreign_key_create_info();
  DBUG_RETURN(res);
}

char *ha_mroonga::get_foreign_key_create_info()
{
  MRN_DBUG_ENTER_METHOD();
  char *res;
  if (share->wrapper_mode)
  {
    res = wrapper_get_foreign_key_create_info();
  } else {
    res = storage_get_foreign_key_create_info();
  }
  DBUG_RETURN(res);
}

#ifdef MRN_HANDLER_HAVE_GET_TABLESPACE_NAME
char *ha_mroonga::wrapper_get_tablespace_name(THD *thd, char *name,
                                              uint name_len)
{
  MRN_DBUG_ENTER_METHOD();
  char *res;
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  res = wrap_handler->get_tablespace_name(thd, name, name_len);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(res);
}

char *ha_mroonga::storage_get_tablespace_name(THD *thd, char *name,
                                              uint name_len)
{
  MRN_DBUG_ENTER_METHOD();
  char *res = handler::get_tablespace_name(thd, name, name_len);
  DBUG_RETURN(res);
}

char *ha_mroonga::get_tablespace_name(THD *thd, char *name, uint name_len)
{
  MRN_DBUG_ENTER_METHOD();
  char *res;
  if (share->wrapper_mode)
  {
    res = wrapper_get_tablespace_name(thd, name, name_len);
  } else {
    res = storage_get_tablespace_name(thd, name, name_len);
  }
  DBUG_RETURN(res);
}
#endif

bool ha_mroonga::wrapper_can_switch_engines()
{
  MRN_DBUG_ENTER_METHOD();
  bool res;
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  res = wrap_handler->can_switch_engines();
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(res);
}

bool ha_mroonga::storage_can_switch_engines()
{
  MRN_DBUG_ENTER_METHOD();
  bool res = handler::can_switch_engines();
  DBUG_RETURN(res);
}

bool ha_mroonga::can_switch_engines()
{
  MRN_DBUG_ENTER_METHOD();
  bool res;
  if (share->wrapper_mode)
  {
    res = wrapper_can_switch_engines();
  } else {
    res = storage_can_switch_engines();
  }
  DBUG_RETURN(res);
}

int ha_mroonga::wrapper_get_foreign_key_list(THD *thd,
                                           List<FOREIGN_KEY_INFO> *f_key_list)
{
  MRN_DBUG_ENTER_METHOD();
  int res;
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  res = wrap_handler->get_foreign_key_list(thd, f_key_list);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(res);
}

int ha_mroonga::storage_get_foreign_key_list(THD *thd,
                                           List<FOREIGN_KEY_INFO> *f_key_list)
{
  MRN_DBUG_ENTER_METHOD();
  int res = handler::get_foreign_key_list(thd, f_key_list);
  DBUG_RETURN(res);
}

int ha_mroonga::get_foreign_key_list(THD *thd,
                                     List<FOREIGN_KEY_INFO> *f_key_list)
{
  MRN_DBUG_ENTER_METHOD();
  int res;
  if (share->wrapper_mode)
  {
    res = wrapper_get_foreign_key_list(thd, f_key_list);
  } else {
    res = storage_get_foreign_key_list(thd, f_key_list);
  }
  DBUG_RETURN(res);
}

#ifdef MRN_HANDLER_HAVE_GET_PARENT_FOREIGN_KEY_LIST
int ha_mroonga::wrapper_get_parent_foreign_key_list(THD *thd,
                                            List<FOREIGN_KEY_INFO> *f_key_list)
{
  MRN_DBUG_ENTER_METHOD();
  int res;
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  res = wrap_handler->get_parent_foreign_key_list(thd, f_key_list);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(res);
}

int ha_mroonga::storage_get_parent_foreign_key_list(THD *thd,
                                            List<FOREIGN_KEY_INFO> *f_key_list)
{
  MRN_DBUG_ENTER_METHOD();
  int res = handler::get_parent_foreign_key_list(thd, f_key_list);
  DBUG_RETURN(res);
}

int ha_mroonga::get_parent_foreign_key_list(THD *thd,
                                            List<FOREIGN_KEY_INFO> *f_key_list)
{
  MRN_DBUG_ENTER_METHOD();
  int res;
  if (share->wrapper_mode)
  {
    res = wrapper_get_parent_foreign_key_list(thd, f_key_list);
  } else {
    res = storage_get_parent_foreign_key_list(thd, f_key_list);
  }
  DBUG_RETURN(res);
}
#endif

uint ha_mroonga::wrapper_referenced_by_foreign_key()
{
  MRN_DBUG_ENTER_METHOD();
  uint res;
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  res = wrap_handler->referenced_by_foreign_key();
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(res);
}

uint ha_mroonga::storage_referenced_by_foreign_key()
{
  MRN_DBUG_ENTER_METHOD();
  uint res = handler::referenced_by_foreign_key();
  DBUG_RETURN(res);
}

uint ha_mroonga::referenced_by_foreign_key()
{
  MRN_DBUG_ENTER_METHOD();
  uint res;
  if (share->wrapper_mode)
  {
    res = wrapper_referenced_by_foreign_key();
  } else {
    res = storage_referenced_by_foreign_key();
  }
  DBUG_RETURN(res);
}

void ha_mroonga::wrapper_init_table_handle_for_HANDLER()
{
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  wrap_handler->init_table_handle_for_HANDLER();
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_VOID_RETURN;
}

void ha_mroonga::storage_init_table_handle_for_HANDLER()
{
  MRN_DBUG_ENTER_METHOD();
  handler::init_table_handle_for_HANDLER();
  DBUG_VOID_RETURN;
}

void ha_mroonga::init_table_handle_for_HANDLER()
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
  {
    wrapper_init_table_handle_for_HANDLER();
  } else {
    storage_init_table_handle_for_HANDLER();
  }
  DBUG_VOID_RETURN;
}

void ha_mroonga::wrapper_free_foreign_key_create_info(char* str)
{
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  wrap_handler->free_foreign_key_create_info(str);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_VOID_RETURN;
}

void ha_mroonga::storage_free_foreign_key_create_info(char* str)
{
  MRN_DBUG_ENTER_METHOD();
  handler::free_foreign_key_create_info(str);
  DBUG_VOID_RETURN;
}

void ha_mroonga::free_foreign_key_create_info(char* str)
{
  MRN_DBUG_ENTER_METHOD();
  if (share->wrapper_mode)
  {
    wrapper_free_foreign_key_create_info(str);
  } else {
    storage_free_foreign_key_create_info(str);
  }
  DBUG_VOID_RETURN;
}

bool ha_mroonga::check_written_by_row_based_binlog()
{
  MRN_DBUG_ENTER_METHOD();
  THD *thd = ha_thd();
  DBUG_RETURN(
#ifdef MRN_ROW_BASED_CHECK_IS_METHOD
    thd->is_current_stmt_binlog_format_row() &&
#else
    thd->current_stmt_binlog_row_based &&
#endif
    table->s->tmp_table == NO_TMP_TABLE &&
    binlog_filter->db_ok(table->s->db.str) &&
#ifdef MRN_OPTION_BITS_IS_UNDER_VARIABLES
    (thd->variables.option_bits & OPTION_BIN_LOG) &&
#else
    (thd->options & OPTION_BIN_LOG) &&
#endif
    mysql_bin_log.is_open()
  );
}

#ifdef MRN_HAVE_HA_REBIND_PSI
void ha_mroonga::wrapper_unbind_psi()
{
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  wrap_handler->unbind_psi();
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_VOID_RETURN;
}

void ha_mroonga::storage_unbind_psi()
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_VOID_RETURN;
}

void ha_mroonga::unbind_psi()
{
  MRN_DBUG_ENTER_METHOD();
  handler::unbind_psi();
  if (share->wrapper_mode)
  {
    wrapper_unbind_psi();
  } else {
    storage_unbind_psi();
  }
  DBUG_VOID_RETURN;
}

void ha_mroonga::wrapper_rebind_psi()
{
  MRN_DBUG_ENTER_METHOD();
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  wrap_handler->rebind_psi();
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_VOID_RETURN;
}

void ha_mroonga::storage_rebind_psi()
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_VOID_RETURN;
}

void ha_mroonga::rebind_psi()
{
  MRN_DBUG_ENTER_METHOD();
  handler::rebind_psi();
  if (share->wrapper_mode)
  {
    wrapper_rebind_psi();
  } else {
    storage_rebind_psi();
  }
  DBUG_VOID_RETURN;
}
#endif

my_bool ha_mroonga::wrapper_register_query_cache_table(THD *thd,
                                                       char *table_key,
                                                       uint key_length,
                                                       qc_engine_callback
                                                       *engine_callback,
                                                       ulonglong *engine_data)
{
  MRN_DBUG_ENTER_METHOD();
  my_bool res;
  MRN_SET_WRAP_SHARE_KEY(share, table->s);
  MRN_SET_WRAP_TABLE_KEY(this, table);
  res = wrap_handler->register_query_cache_table(thd,
                                                 table_key,
                                                 key_length,
                                                 engine_callback,
                                                 engine_data);
  MRN_SET_BASE_SHARE_KEY(share, table->s);
  MRN_SET_BASE_TABLE_KEY(this, table);
  DBUG_RETURN(res);
}

my_bool ha_mroonga::storage_register_query_cache_table(THD *thd,
                                                       char *table_key,
                                                       uint key_length,
                                                       qc_engine_callback
                                                       *engine_callback,
                                                       ulonglong *engine_data)
{
  MRN_DBUG_ENTER_METHOD();
  my_bool res = handler::register_query_cache_table(thd,
                                                    table_key,
                                                    key_length,
                                                    engine_callback,
                                                    engine_data);
  DBUG_RETURN(res);
}

my_bool ha_mroonga::register_query_cache_table(THD *thd,
                                               char *table_key,
                                               uint key_length,
                                               qc_engine_callback
                                               *engine_callback,
                                               ulonglong *engine_data)
{
  MRN_DBUG_ENTER_METHOD();
  my_bool res;
  if (share->wrapper_mode)
  {
    res = wrapper_register_query_cache_table(thd,
                                             table_key,
                                             key_length,
                                             engine_callback,
                                             engine_data);
  } else {
    res = storage_register_query_cache_table(thd,
                                             table_key,
                                             key_length,
                                             engine_callback,
                                             engine_data);
  }
  DBUG_RETURN(res);
}

COND *ha_mroonga::get_select_cond()
{
  MRN_DBUG_ENTER_METHOD();
  st_select_lex *select_lex = table->pos_in_table_list->select_lex;
  DBUG_PRINT("info", ("mroonga: select_lex = %p", select_lex));
  JOIN *join = select_lex->join;
  DBUG_PRINT("info", ("mroonga: join = %p", join));
  JOIN_TAB *join_tab = join->join_tab + join->const_tables;
  DBUG_PRINT("info", ("mroonga: join_tab = %p", join_tab));
  COND *select_cond = join_tab->select_cond;
  DBUG_PRINT("info", ("mroonga: select_cond = %p", select_cond));
  DBUG_RETURN(select_cond);
}

Item_func_match *ha_mroonga::get_tgt_ft_item(Item_func_match *current)
{
  MRN_DBUG_ENTER_METHOD();
  st_select_lex *select_lex = table->pos_in_table_list->select_lex;
  DBUG_PRINT("info", ("mroonga: select_lex = %p", select_lex));
  List_iterator<Item_func_match> li(*(select_lex->ftfunc_list));
  Item_func_match *item_func_match;
  while ((item_func_match = li++)) {
    if (current) {
      if (current == item_func_match) {
        current = NULL;
      }
      continue;
    }
    if (item_func_match->table == table) {
      if (item_func_match->master && item_func_match->master->ft_handler) {
        continue;
      }
      DBUG_PRINT("info", ("mroonga: item_func_match = %p", item_func_match));
      DBUG_RETURN(item_func_match);
    }
  }
  DBUG_RETURN(NULL);
}

bool ha_mroonga::check_ft_in_where_item_type(Item *item, Item *ft_item)
{
  MRN_DBUG_ENTER_METHOD();
  DBUG_PRINT("info", ("mroonga: COND type=%d", item->type()));
  switch (item->type())
  {
    case Item::FUNC_ITEM:
      DBUG_RETURN(check_ft_in_where_item_func((Item_func *) item, ft_item));
    case Item::COND_ITEM:
      DBUG_RETURN(check_ft_in_where_item_cond((Item_cond *) item, ft_item));
    default:
      break;
  }
  DBUG_RETURN(FALSE);
}

bool ha_mroonga::check_ft_in_where_item_cond(Item_cond *item_cond,
                                             Item *ft_item)
{
  MRN_DBUG_ENTER_METHOD();
  if (item_cond->functype() == Item_func::COND_AND_FUNC) {
    List_iterator_fast<Item> lif(*(item_cond->argument_list()));
    Item *item;
    while ((item = lif++))
    {
      if (check_ft_in_where_item_type(item, ft_item))
      {
        DBUG_RETURN(TRUE);
      }
    }
  }
  DBUG_RETURN(FALSE);
}

bool ha_mroonga::check_ft_in_where_item_func(Item_func *item_func,
                                             Item *ft_item)
{
  MRN_DBUG_ENTER_METHOD();
  if (item_func->functype() == Item_func::FT_FUNC)
  {
    Item_func_match *item_func_match = (Item_func_match *)item_func;
    while (item_func_match->master) {
      DBUG_PRINT("info", ("mroonga: has master child=%p", item_func_match));
      item_func_match = item_func_match->master;
    }
    Item_func_match *current_func_match = (Item_func_match *)ft_item;
    while (current_func_match->master) {
      DBUG_PRINT("info", ("mroonga: current has master child=%p",
        current_func_match));
      current_func_match = current_func_match->master;
    }
    DBUG_PRINT("info", ("mroonga: item_func_match=%p current_func_match=%p",
      item_func_match, current_func_match));
    if (item_func_match == current_func_match) {
      DBUG_PRINT("info", ("mroonga: match ft item"));
      DBUG_RETURN(TRUE);
    }
  }
  DBUG_RETURN(FALSE);
}

#ifdef __cplusplus
}
#endif
