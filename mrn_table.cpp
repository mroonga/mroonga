/* -*- c-basic-offset: 2 -*- */
/*
  Copyright(C) 2011-2013 Kentoku SHIBA
  Copyright(C) 2011-2021 Sutou Kouhei <kou@clear-code.com>
  Copyright(C) 2020-2021 Horimoto Yasuhiro <horimoto@clear-code.com>

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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "mrn_mysql.h"

#include <mysql/plugin.h>
#include <my_bitmap.h>
#include <sql_servers.h>
#include <sql_base.h>
#ifdef WITH_PARTITION_STORAGE_ENGINE
#  include <partition_info.h>
#endif
#include <sql_plugin.h>
#include <sql/field.h>
#include <sql/table.h>
#include <mysqld.h>

#include "mrn_err.h"
#include "mrn_mysql_compat.h"

#include "mrn_table.hpp"
#include "mrn_variables.hpp"
#include <mrn_lock.hpp>

#ifdef MRN_HAVE_SQL_DERROR_H
#  include <sql/derror.h>
#endif

#ifdef MRN_OPEN_TABLE_DEF_USE_TABLE_DEFINITION
#  include <sql/dd_table_share.h>
#endif

#include <cstring>

#ifdef MRN_MARIADB_P
#  if MYSQL_VERSION_ID >= 100300
     typedef LEX_CSTRING mrn_resolve_name;
#  else
     typedef LEX_STRING mrn_resolve_name;
#  endif
#  if MYSQL_VERSION_ID >= 100100
#    define MRN_HA_RESOLVE_BY_NAME(name) ha_resolve_by_name(NULL, (name), true)
#  else
#    define MRN_HA_RESOLVE_BY_NAME(name) ha_resolve_by_name(NULL, (name))
#  endif
#else
#  if MYSQL_VERSION_ID >= 80017
     using mrn_resolve_name = LEX_CSTRING;
#    define MRN_HA_RESOLVE_BY_NAME(name) ha_resolve_by_name(NULL, (name), true)
#  else
     typedef LEX_STRING mrn_resolve_name;
#    define MRN_HA_RESOLVE_BY_NAME(name) ha_resolve_by_name(NULL, (name), true)
#  endif
#endif

#ifdef MRN_MARIADB_P
#  define MRN_PLUGIN_DATA(plugin, type) plugin_data(plugin, type)
#else
#  define MRN_PLUGIN_DATA(plugin, type) plugin_data<type>(plugin)
#endif

#define LEX_STRING_IS_EMPTY(string)                                     \
  ((string).length == 0 || !(string).str || (string).str[0] == '\0')

#define MRN_DEFAULT_STR "DEFAULT"
#define MRN_DEFAULT_LEN (sizeof(MRN_DEFAULT_STR) - 1)
#define MRN_GROONGA_STR "GROONGA"
#define MRN_GROONGA_LEN (sizeof(MRN_GROONGA_STR) - 1)

#ifdef MRN_HAVE_TABLE_DEF_CACHE
extern mrn_table_def_cache_type *mrn_table_def_cache;
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAVE_PSI_INTERFACE
#  ifdef WIN32
#    ifdef MRN_TABLE_SHARE_HAVE_LOCK_SHARE
extern PSI_mutex_key *mrn_table_share_lock_share;
#    endif
extern PSI_mutex_key *mrn_table_share_lock_ha_data;
#  endif
extern PSI_mutex_key mrn_share_mutex_key;
extern PSI_mutex_key mrn_long_term_share_auto_inc_mutex_key;
#endif

extern grn_ctx mrn_ctx;
extern grn_hash *mrn_open_tables;
extern mysql_mutex_t mrn_open_tables_mutex;
extern grn_hash *mrn_long_term_shares;
extern mysql_mutex_t mrn_long_term_shares_mutex;
extern char *mrn_default_tokenizer;
extern char *mrn_default_wrapper_engine;
extern handlerton *mrn_hton_ptr;
extern grn_hash *mrn_allocated_thds;
extern mysql_mutex_t mrn_allocated_thds_mutex;

static char *mrn_get_string_between_quote(const char *ptr)
{
  const char *start_ptr, *end_ptr, *tmp_ptr, *esc_ptr;
  bool find_flg = false, esc_flg = false;
  MRN_DBUG_ENTER_FUNCTION();

  start_ptr = strchr(ptr, '\'');
  end_ptr = strchr(ptr, '"');
  if (start_ptr && (!end_ptr || start_ptr < end_ptr))
  {
    tmp_ptr = ++start_ptr;
    while (!find_flg)
    {
      if (!(end_ptr = strchr(tmp_ptr, '\'')))
        DBUG_RETURN(NULL);
      esc_ptr = tmp_ptr;
      while (!find_flg)
      {
        esc_ptr = strchr(esc_ptr, '\\');
        if (!esc_ptr || esc_ptr > end_ptr)
          find_flg = true;
        else if (esc_ptr == end_ptr - 1)
        {
          esc_flg = true;
          tmp_ptr = end_ptr + 1;
          break;
        } else {
          esc_flg = true;
          esc_ptr += 2;
        }
      }
    }
  } else if (end_ptr)
  {
    start_ptr = end_ptr;
    tmp_ptr = ++start_ptr;
    while (!find_flg)
    {
      if (!(end_ptr = strchr(tmp_ptr, '"')))
        DBUG_RETURN(NULL);
      esc_ptr = tmp_ptr;
      while (!find_flg)
      {
        esc_ptr = strchr(esc_ptr, '\\');
        if (!esc_ptr || esc_ptr > end_ptr)
          find_flg = true;
        else if (esc_ptr == end_ptr - 1)
        {
          esc_flg = true;
          tmp_ptr = end_ptr + 1;
          break;
        } else {
          esc_flg = true;
          esc_ptr += 2;
        }
      }
    }
  } else
    DBUG_RETURN(NULL);

  size_t length = end_ptr - start_ptr;
  char *extracted_string = (char *)mrn_my_malloc(length + 1, MYF(MY_WME));
  if (esc_flg) {
    size_t extracted_index = 0;
    const char *current_ptr = start_ptr;
    while (current_ptr < end_ptr) {
      if (*current_ptr != '\\') {
        extracted_string[extracted_index] = *current_ptr;
        ++extracted_index;
        current_ptr++;
        continue;
      }

      if (current_ptr + 1 == end_ptr) {
        break;
      }

      switch (*(current_ptr + 1))
      {
      case 'b':
        extracted_string[extracted_index] = '\b';
        break;
      case 'n':
        extracted_string[extracted_index] = '\n';
        break;
      case 'r':
        extracted_string[extracted_index] = '\r';
        break;
      case 't':
        extracted_string[extracted_index] = '\t';
        break;
      default:
        extracted_string[extracted_index] = *(current_ptr + 1);
        break;
      }
      ++extracted_index;
    }
  } else {
    grn_memcpy(extracted_string, start_ptr, length);
    extracted_string[length] = '\0';
  }

  DBUG_RETURN(extracted_string);
}

#ifdef WITH_PARTITION_STORAGE_ENGINE
void mrn_get_partition_info(const char *table_name, uint table_name_length,
                            const TABLE *table, partition_element **part_elem,
                            partition_element **sub_elem)
{
  partition_info *part_info = table->part_info;
  partition_element *tmp_part_elem = NULL, *tmp_sub_elem = NULL;
  bool tmp_flg = false, tmp_find_flg = false;
  MRN_DBUG_ENTER_FUNCTION();
  *part_elem = NULL;
  *sub_elem = NULL;
  if (!part_info)
    DBUG_VOID_RETURN;

  if (table_name && !memcmp(table_name + table_name_length - 5, "#TMP#", 5))
    tmp_flg = true;

  DBUG_PRINT("info", ("mroonga table_name=%s", table_name));
  List_iterator<partition_element> part_it(part_info->partitions);
  while ((*part_elem = part_it++))
  {
    if ((*part_elem)->subpartitions.elements)
    {
      List_iterator<partition_element> sub_it((*part_elem)->subpartitions);
      while ((*sub_elem = sub_it++))
      {
        char subpartition_name[FN_REFLEN + 1];
        int error = mrn_create_subpartition_name(subpartition_name,
                                                 sizeof(subpartition_name),
                                                 table->s->path.str,
                                                 (*part_elem)->partition_name,
                                                 (*sub_elem)->partition_name,
                                                 NORMAL_PART_NAME);
        if (error != 0)
          DBUG_VOID_RETURN;
        DBUG_PRINT("info", ("mroonga subpartition name=%s", subpartition_name));
        if (table_name &&
            memcmp(table_name, subpartition_name, table_name_length + 1) == 0)
          DBUG_VOID_RETURN;
        if (
          tmp_flg &&
          table_name &&
          *(subpartition_name + table_name_length - 5) == '\0' &&
          memcmp(table_name, subpartition_name, table_name_length - 5) == 0
        ) {
          tmp_part_elem = *part_elem;
          tmp_sub_elem = *sub_elem;
          tmp_flg = false;
          tmp_find_flg = true;
        }
      }
    } else {
      char partition_name[FN_REFLEN + 1];
      int error = mrn_create_partition_name(partition_name,
                                            sizeof(partition_name),
                                            table->s->path.str,
                                            (*part_elem)->partition_name,
                                            NORMAL_PART_NAME,
                                            true);
      if (error != 0)
        DBUG_VOID_RETURN;
      DBUG_PRINT("info", ("mroonga partition name=%s", partition_name));
      if (table_name &&
          memcmp(table_name, partition_name, table_name_length + 1) == 0)
        DBUG_VOID_RETURN;
      if (
        tmp_flg &&
        table_name &&
        *(partition_name + table_name_length - 5) == '\0' &&
        memcmp(table_name, partition_name, table_name_length - 5) == 0
      ) {
        tmp_part_elem = *part_elem;
        tmp_flg = false;
        tmp_find_flg = true;
      }
    }
  }
  if (tmp_find_flg)
  {
    *part_elem = tmp_part_elem;
    *sub_elem = tmp_sub_elem;
    DBUG_PRINT("info", ("mroonga tmp find"));
    DBUG_VOID_RETURN;
  }
  *part_elem = NULL;
  *sub_elem = NULL;
  DBUG_PRINT("info", ("mroonga no hit"));
  DBUG_VOID_RETURN;
}
#endif

#define MRN_PARAM_STR_LEN(name) name ## _length
#define MRN_PARAM_STR(title_name, param_name) \
  if (!strncasecmp(tmp_ptr, title_name, title_length)) \
  { \
    DBUG_PRINT("info", ("mroonga " title_name " start")); \
    if (!share->param_name) \
    { \
      if ((share->param_name = mrn_get_string_between_quote( \
        start_ptr))) \
        share->MRN_PARAM_STR_LEN(param_name) = strlen(share->param_name); \
      else { \
        error = ER_MRN_INVALID_TABLE_PARAM_NUM; \
        my_printf_error(error, ER_MRN_INVALID_TABLE_PARAM_STR, \
          MYF(0), tmp_ptr); \
        goto error; \
      } \
      DBUG_PRINT("info", ("mroonga " title_name "=%s", share->param_name)); \
    } \
    break; \
  }

#define MRN_PARAM_STR_LIST(title_name, param_name, param_pos) \
  if (!strncasecmp(tmp_ptr, title_name, title_length)) \
  { \
    DBUG_PRINT("info", ("mroonga " title_name " start")); \
    if (share->param_name && !share->param_name[param_pos]) \
    { \
      if ((share->param_name[param_pos] = mrn_get_string_between_quote( \
        start_ptr))) \
        share->MRN_PARAM_STR_LEN(param_name)[param_pos] = \
          strlen(share->param_name[param_pos]); \
      else { \
        error = ER_MRN_INVALID_TABLE_PARAM_NUM; \
        my_printf_error(error, ER_MRN_INVALID_TABLE_PARAM_STR, \
          MYF(0), tmp_ptr); \
        goto error; \
      } \
      DBUG_PRINT("info", ("mroonga " title_name "[%d]=%s", param_pos, \
        share->param_name[param_pos])); \
    } \
    break; \
  }

int mrn_parse_table_param(MRN_SHARE *share, TABLE *table)
{
  int i, error = 0;
  int title_length;
  const char *sprit_ptr[2];
  const char *tmp_ptr, *start_ptr;
  char *params_string = NULL;
#ifdef WITH_PARTITION_STORAGE_ENGINE
  partition_element *part_elem;
  partition_element *sub_elem;
#endif
  MRN_DBUG_ENTER_FUNCTION();
#ifdef WITH_PARTITION_STORAGE_ENGINE
  mrn_get_partition_info(share->table_name, share->table_name_length, table,
    &part_elem, &sub_elem);
#endif
#ifdef WITH_PARTITION_STORAGE_ENGINE
  for (i = 4; i > 0; i--)
#else
  for (i = 2; i > 0; i--)
#endif
  {
    const char *params_string_value = NULL;
    uint params_string_length = 0;
    switch (i)
    {
#ifdef WITH_PARTITION_STORAGE_ENGINE
      case 4:
        if (!sub_elem || !sub_elem->part_comment)
          continue;
        DBUG_PRINT("info", ("mroonga create sub comment string"));
        params_string_value = sub_elem->part_comment;
        params_string_length = strlen(params_string_value);
        DBUG_PRINT("info",
                   ("mroonga sub comment string=%s", params_string_value));
        break;
      case 3:
        if (!part_elem || !part_elem->part_comment)
          continue;
        DBUG_PRINT("info", ("mroonga create part comment string"));
        params_string_value = part_elem->part_comment;
        params_string_length = strlen(params_string_value);
        DBUG_PRINT("info",
                   ("mroonga part comment string=%s", params_string_value));
        break;
#endif
      case 2:
        if (LEX_STRING_IS_EMPTY(table->s->comment))
          continue;
        DBUG_PRINT("info", ("mroonga create comment string"));
        params_string_value = table->s->comment.str;
        params_string_length = table->s->comment.length;
        DBUG_PRINT("info",
                   ("mroonga comment string=%.*s",
                    params_string_length, params_string_value));
        break;
      default:
        if (LEX_STRING_IS_EMPTY(table->s->connect_string))
          continue;
        DBUG_PRINT("info", ("mroonga create connect_string string"));
        params_string_value = table->s->connect_string.str;
        params_string_length = table->s->connect_string.length;
        DBUG_PRINT("info",
                   ("mroonga connect_string=%.*s",
                    params_string_length, params_string_value));
        break;
    }

    if (!params_string_value) {
      continue;
    }

    {
      params_string = mrn_my_strndup(params_string_value,
                                     params_string_length,
                                     MYF(MY_WME));
      if (!params_string) {
        error = HA_ERR_OUT_OF_MEM;
        goto error;
      }

      sprit_ptr[0] = params_string;
      while (sprit_ptr[0])
      {
        if ((sprit_ptr[1] = strchr(sprit_ptr[0], ',')))
        {
          sprit_ptr[1]++;
        }
        tmp_ptr = sprit_ptr[0];
        sprit_ptr[0] = sprit_ptr[1];
        while (*tmp_ptr == ' ' || *tmp_ptr == '\r' ||
               *tmp_ptr == '\n' || *tmp_ptr == '\t')
          tmp_ptr++;

        if (*tmp_ptr == '\0')
          continue;

        DBUG_PRINT("info", ("mroonga title_str=%s", tmp_ptr));
        title_length = 0;
        start_ptr = tmp_ptr;
        while (*start_ptr != ' ' && *start_ptr != '\'' &&
               *start_ptr != '"' && *start_ptr != '\0' &&
               *start_ptr != '\r' && *start_ptr != '\n' &&
               *start_ptr != '\t' && *start_ptr != ',')
        {
          title_length++;
          start_ptr++;
        }
        DBUG_PRINT("info", ("mroonga title_length=%u", title_length));

        switch (title_length)
        {
        case 5:
          MRN_PARAM_STR("flags", table_flags);
          break;
        case 6:
          MRN_PARAM_STR("engine", engine);
          break;
        case 9:
          MRN_PARAM_STR("tokenizer", tokenizer);
          break;
        case 10:
          MRN_PARAM_STR("normalizer", normalizer);
          break;
        case 13:
          MRN_PARAM_STR("token_filters", token_filters);
          break;
        case 17:
          /* Deprecated */
          MRN_PARAM_STR("default_tokenizer", default_tokenizer);
          break;
        default:
          break;
        }
      }

      my_free(params_string);
      params_string = NULL;
    }
  }

  if (!share->engine && mrn_default_wrapper_engine)
  {
    share->engine_length = strlen(mrn_default_wrapper_engine);
    if (
      !(share->engine = mrn_my_strndup(mrn_default_wrapper_engine,
                                       share->engine_length,
                                       MYF(MY_WME)))
    ) {
      error = HA_ERR_OUT_OF_MEM;
      goto error;
    }
  }

  if (share->engine)
  {
    mrn_resolve_name engine_name;
    if (
      (
        share->engine_length == MRN_DEFAULT_LEN &&
        !strncasecmp(share->engine, MRN_DEFAULT_STR, MRN_DEFAULT_LEN)
      ) ||
      (
        share->engine_length == MRN_GROONGA_LEN &&
        !strncasecmp(share->engine, MRN_GROONGA_STR, MRN_GROONGA_LEN)
      )
    ) {
      my_free(share->engine);
      share->engine = NULL;
      share->engine_length = 0;
    } else {
      engine_name.str = share->engine;
      engine_name.length = share->engine_length;
      if (!(share->plugin = MRN_HA_RESOLVE_BY_NAME(&engine_name)))
      {
        my_error(ER_UNKNOWN_STORAGE_ENGINE, MYF(0), share->engine);
        error = ER_UNKNOWN_STORAGE_ENGINE;
        goto error;
      }
      share->hton = MRN_PLUGIN_DATA(share->plugin, handlerton *);
      share->wrapper_mode = true;
    }
  }

error:
  if (params_string)
    my_free(params_string);
  DBUG_RETURN(error);
}

bool mrn_is_geo_key(const KEY *key_info)
{
  return key_info->algorithm != HA_KEY_ALG_BTREE &&
    key_info->algorithm != HA_KEY_ALG_HASH &&
    key_info->algorithm != HA_KEY_ALG_FULLTEXT &&
    KEY_N_KEY_PARTS(key_info) == 1 &&
    key_info->key_part[0].field &&
    key_info->key_part[0].field->type() == MYSQL_TYPE_GEOMETRY;
}

int mrn_add_column_param(MRN_SHARE *share, Field *field, int i)
{
  int error;
  char *param_string = NULL;
  int title_length;
  char *sprit_ptr[2];
  char *tmp_ptr, *start_ptr;

  MRN_DBUG_ENTER_FUNCTION();

  if (share->wrapper_mode) {
    DBUG_RETURN(0);
  }

  DBUG_PRINT("info", ("mroonga create comment string"));
  if (
    !(param_string = mrn_my_strndup(field->comment.str,
                                    field->comment.length,
                                    MYF(MY_WME)))
  ) {
    error = HA_ERR_OUT_OF_MEM;
    goto error_alloc_param_string;
  }
  DBUG_PRINT("info", ("mroonga comment string=%s", param_string));

  sprit_ptr[0] = param_string;
  while (sprit_ptr[0])
  {
    if ((sprit_ptr[1] = strchr(sprit_ptr[0], ',')))
    {
      *sprit_ptr[1] = '\0';
      sprit_ptr[1]++;
    }
    tmp_ptr = sprit_ptr[0];
    sprit_ptr[0] = sprit_ptr[1];
    while (*tmp_ptr == ' ' || *tmp_ptr == '\r' ||
      *tmp_ptr == '\n' || *tmp_ptr == '\t')
      tmp_ptr++;

    if (*tmp_ptr == '\0')
      continue;

    title_length = 0;
    start_ptr = tmp_ptr;
    while (*start_ptr != ' ' && *start_ptr != '\'' &&
      *start_ptr != '"' && *start_ptr != '\0' &&
      *start_ptr != '\r' && *start_ptr != '\n' &&
      *start_ptr != '\t')
    {
      title_length++;
      start_ptr++;
    }

    switch (title_length)
    {
      case 4:
        MRN_PARAM_STR_LIST("type", col_type, i);
        break;
      case 5:
        MRN_PARAM_STR_LIST("flags", col_flags, i);
        break;
      case 12:
        MRN_PARAM_STR_LIST("groonga_type", col_type, i);
        break;
      default:
        break;
    }
  }

  if (param_string)
    my_free(param_string);
  DBUG_RETURN(0);

error:
  if (param_string)
    my_free(param_string);
error_alloc_param_string:
  DBUG_RETURN(error);
}

int mrn_parse_column_param(MRN_SHARE *share, TABLE *table)
{
  int error;
  MRN_DBUG_ENTER_FUNCTION();
  for (uint i = 0; i < table->s->fields; i++)
  {
    Field *field = table->s->field[i];

    if (LEX_STRING_IS_EMPTY(field->comment)) {
      continue;
    }

    if ((error = mrn_add_column_param(share, field, i)))
      goto error;
  }
  DBUG_RETURN(0);

error:
  DBUG_RETURN(error);
}

int mrn_free_share_alloc(
  MRN_SHARE *share
) {
  uint i;
  MRN_DBUG_ENTER_FUNCTION();
  if (share->table_flags)
    my_free(share->table_flags);
  if (share->engine)
    my_free(share->engine);
  if (share->tokenizer)
    my_free(share->tokenizer);
  if (share->default_tokenizer)
    my_free(share->default_tokenizer);
  if (share->normalizer)
    my_free(share->normalizer);
  if (share->token_filters)
    my_free(share->token_filters);
  for (i = 0; i < share->table_share->fields; i++)
  {
    if (share->col_flags && share->col_flags[i])
      my_free(share->col_flags[i]);
    if (share->col_type && share->col_type[i])
      my_free(share->col_type[i]);
  }
  DBUG_RETURN(0);
}

void mrn_free_long_term_share(MRN_LONG_TERM_SHARE *long_term_share)
{
  MRN_DBUG_ENTER_FUNCTION();
  {
    mrn::Lock lock(&mrn_long_term_shares_mutex);
    grn_hash_delete(&mrn_ctx,
                    mrn_long_term_shares,
                    long_term_share->table_name,
                    long_term_share->table_name_length,
                    NULL);
  }
  mysql_mutex_destroy(&long_term_share->auto_inc_mutex);
  my_free(long_term_share);
  DBUG_VOID_RETURN;
}

MRN_LONG_TERM_SHARE *mrn_get_long_term_share(const char *table_name,
                                             uint table_name_length,
                                             int *error)
{
  MRN_LONG_TERM_SHARE *long_term_share = NULL;
  char *tmp_name;
  MRN_DBUG_ENTER_FUNCTION();
  DBUG_PRINT("info", ("mroonga: table_name=%s", table_name));
  mrn::Lock lock(&mrn_long_term_shares_mutex);
  {
    void *long_term_share_address;
    if (grn_hash_get(&mrn_ctx,
                     mrn_long_term_shares,
                     table_name,
                     table_name_length,
                     &long_term_share_address) != GRN_ID_NIL) {
      grn_memcpy(&long_term_share,
                 long_term_share_address,
                 sizeof(long_term_share));
    }
  }
  if (!long_term_share) {
    if (!(long_term_share = (MRN_LONG_TERM_SHARE *)
      mrn_my_multi_malloc(MYF(MY_WME | MY_ZEROFILL),
        &long_term_share, sizeof(*long_term_share),
        &tmp_name, table_name_length + 1,
        NullS))
    ) {
      *error = HA_ERR_OUT_OF_MEM;
      goto error_alloc_long_term_share;
    }
    long_term_share->table_name = tmp_name;
    long_term_share->table_name_length = table_name_length;
    grn_memcpy(long_term_share->table_name, table_name, table_name_length);
    if (mysql_mutex_init(mrn_long_term_share_auto_inc_mutex_key,
                         &long_term_share->auto_inc_mutex,
                         MY_MUTEX_INIT_FAST) != 0)
    {
      *error = HA_ERR_OUT_OF_MEM;
      goto error_init_auto_inc_mutex;
    }
    {
      void *long_term_share_address;
      if (grn_hash_add(&mrn_ctx,
                       mrn_long_term_shares,
                       long_term_share->table_name,
                       long_term_share->table_name_length,
                       &long_term_share_address,
                       NULL) == GRN_ID_NIL) {
        *error = HA_ERR_OUT_OF_MEM;
        goto error_hash_insert;
      }
      grn_memcpy(long_term_share_address,
                 &long_term_share,
                 sizeof(long_term_share));
    }
  }
  DBUG_RETURN(long_term_share);

error_hash_insert:
  mysql_mutex_destroy(&long_term_share->auto_inc_mutex);
error_init_auto_inc_mutex:
  my_free(long_term_share);
error_alloc_long_term_share:
  DBUG_RETURN(NULL);
}

MRN_SHARE *mrn_get_share(const char *table_name, TABLE *table, int *error)
{
  MRN_SHARE *share = NULL;
  char *tmp_name, **col_flags, **col_type;
  uint length, *wrap_key_nr;
  uint *col_flags_length, *col_type_length, i, j;
  KEY *wrap_key_info;
  TABLE_SHARE *wrap_table_share;
  MRN_DBUG_ENTER_FUNCTION();
  length = (uint) strlen(table_name);
  mrn::Lock lock(&mrn_open_tables_mutex);
  {
    void *share_address = NULL;
    if (grn_hash_get(&mrn_ctx,
                     mrn_open_tables,
                     table_name,
                     length,
                     &share_address) != GRN_ID_NIL) {
      grn_memcpy(&share, share_address, sizeof(share));
    }
  }
  if (!share) {
    if (!(share = (MRN_SHARE *)
      mrn_my_multi_malloc(MYF(MY_WME | MY_ZEROFILL),
        &share, sizeof(*share),
        &tmp_name, length + 1,
        &col_flags, sizeof(char *) * table->s->fields,
        &col_flags_length, sizeof(uint) * table->s->fields,
        &col_type, sizeof(char *) * table->s->fields,
        &col_type_length, sizeof(uint) * table->s->fields,
        &wrap_key_nr, sizeof(*wrap_key_nr) * table->s->keys,
        &wrap_key_info, sizeof(*wrap_key_info) * table->s->keys,
        &wrap_table_share, sizeof(*wrap_table_share),
        NullS))
    ) {
      *error = HA_ERR_OUT_OF_MEM;
      goto error_alloc_share;
    }
    share->use_count = 0;
    share->table_name_length = length;
    share->table_name = tmp_name;
    share->col_flags = col_flags;
    share->col_flags_length = col_flags_length;
    share->col_type = col_type;
    share->col_type_length = col_type_length;
    mrn_my_stpmov(share->table_name, table_name);
    share->table_share = table->s;

    if (
      (*error = mrn_parse_table_param(share, table)) ||
      (*error = mrn_parse_column_param(share, table))
    )
      goto error_parse_table_param;

    if (share->wrapper_mode)
    {
      j = 0;
      for (i = 0; i < table->s->keys; i++)
      {
        if (table->s->key_info[i].algorithm != HA_KEY_ALG_FULLTEXT &&
          !mrn_is_geo_key(&table->s->key_info[i]))
        {
          wrap_key_nr[i] = j;
          grn_memcpy(&wrap_key_info[j], &table->s->key_info[i],
                     sizeof(*wrap_key_info));
          j++;
        } else {
          wrap_key_nr[i] = MAX_KEY;
        }
      }
      share->wrap_keys = j;
      share->base_keys = table->s->keys;
      share->base_key_info = table->s->key_info;
      share->base_primary_key = table->s->primary_key;
      if (i)
      {
        share->wrap_key_nr = wrap_key_nr;
        share->wrap_key_info = wrap_key_info;
        if (table->s->primary_key == MAX_KEY)
          share->wrap_primary_key = MAX_KEY;
        else
          share->wrap_primary_key = wrap_key_nr[table->s->primary_key];
      } else {
        share->wrap_key_nr = NULL;
        share->wrap_key_info = NULL;
        share->wrap_primary_key = MAX_KEY;
      }
      grn_memcpy(wrap_table_share, table->s, sizeof(*wrap_table_share));
      mrn_init_sql_alloc(current_thd,
                         "mroonga::wrap-table-share",
                         &(wrap_table_share->mem_root));
      wrap_table_share->keys = share->wrap_keys;
      wrap_table_share->key_info = share->wrap_key_info;
      wrap_table_share->primary_key = share->wrap_primary_key;
      wrap_table_share->keys_in_use.init(share->wrap_keys);
      wrap_table_share->keys_for_keyread.init(share->wrap_keys);
#ifdef MRN_TABLE_SHARE_HAVE_LOCK_SHARE
#  ifdef WIN32
      mysql_mutex_init(*mrn_table_share_lock_share,
                       &(wrap_table_share->LOCK_share), MY_MUTEX_INIT_SLOW);
#  else
      mysql_mutex_init(key_TABLE_SHARE_LOCK_share,
                       &(wrap_table_share->LOCK_share), MY_MUTEX_INIT_SLOW);
#  endif
#endif
#ifdef WIN32
      mysql_mutex_init(*mrn_table_share_lock_ha_data,
                       &(wrap_table_share->LOCK_ha_data), MY_MUTEX_INIT_FAST);
#else
      mysql_mutex_init(key_TABLE_SHARE_LOCK_ha_data,
                       &(wrap_table_share->LOCK_ha_data), MY_MUTEX_INIT_FAST);
#endif
      share->wrap_table_share = wrap_table_share;
    }

    if (mysql_mutex_init(mrn_share_mutex_key,
                         &share->record_mutex,
                         MY_MUTEX_INIT_FAST) != 0)
    {
      *error = HA_ERR_OUT_OF_MEM;
      goto error_init_mutex;
    }
    thr_lock_init(&share->lock);
    if (!(share->long_term_share = mrn_get_long_term_share(table_name, length,
                                                           error)))
    {
      goto error_get_long_term_share;
    }
    {
      void *share_address;
      if (grn_hash_add(&mrn_ctx,
                       mrn_open_tables,
                       share->table_name,
                       share->table_name_length,
                       &share_address,
                       NULL) == GRN_ID_NIL) {
        *error = HA_ERR_OUT_OF_MEM;
        goto error_hash_insert;
      }
      grn_memcpy(share_address, &share, sizeof(share));
    }
  }
  share->use_count++;
  DBUG_RETURN(share);

error_hash_insert:
error_get_long_term_share:
  mysql_mutex_destroy(&share->record_mutex);
error_init_mutex:
error_parse_table_param:
  mrn_free_share_alloc(share);
  my_free(share);
error_alloc_share:
  DBUG_RETURN(NULL);
}

int mrn_free_share(MRN_SHARE *share)
{
  MRN_DBUG_ENTER_FUNCTION();
  mrn::Lock lock(&mrn_open_tables_mutex);
  if (!--share->use_count)
  {
    grn_hash_delete(&mrn_ctx,
                    mrn_open_tables,
                    share->table_name,
                    share->table_name_length,
                    NULL);
    if (share->wrapper_mode)
      plugin_unlock(NULL, share->plugin);
    mrn_free_share_alloc(share);
    thr_lock_delete(&share->lock);
    mysql_mutex_destroy(&share->record_mutex);
    if (share->wrapper_mode) {
#ifdef MRN_TABLE_SHARE_HAVE_LOCK_SHARE
      mysql_mutex_destroy(&(share->wrap_table_share->LOCK_share));
#endif
      mysql_mutex_destroy(&(share->wrap_table_share->LOCK_ha_data));
      MRN_FREE_ROOT(&(share->wrap_table_share->mem_root));
    }
    my_free(share);
  }
  DBUG_RETURN(0);
}

TABLE_SHARE *mrn_get_table_share(TABLE_LIST *table_list, int *error)
{
  TABLE_SHARE *share;
  THD *thd = current_thd;
  MRN_DBUG_ENTER_FUNCTION();
#if defined(MRN_HAVE_TDC_ACQUIRE_SHARE) &&      \
  !defined(MRN_TDC_ACQUIRE_SHARE_REQUIRE_KEY)
  share = tdc_acquire_share(thd, table_list, GTS_TABLE);
#else
  uint key_length;
#  ifdef MRN_HAVE_GET_TABLE_DEF_KEY
  const char *key;
  key_length = get_table_def_key(table_list, &key);
#  else
  char key[MAX_DBKEY_LENGTH];
  key_length = create_table_def_key(thd, key, table_list, false);
#  endif
#  ifdef MRN_HAVE_TABLE_DEF_CACHE
#    ifdef MRN_TABLE_DEF_CACHE_TYPE_IS_MAP
  share = get_table_share(thd,
                          table_list->db,
                          table_list->table_name,
                          key,
                          key_length,
                          false);
#    else
  my_hash_value_type hash_value;
  hash_value = my_calc_hash(mrn_table_def_cache, (uchar*) key, key_length);
  share = get_table_share(thd, table_list, key, key_length, 0, error,
                          hash_value);
#    endif
#  elif defined(MRN_HAVE_TDC_ACQUIRE_SHARE)
  share = tdc_acquire_share(thd, table_list->db, table_list->table_name, key,
                            key_length,
                            table_list->mdl_request.key.tc_hash_value(),
                            GTS_TABLE, NULL);
#  else
  share = get_table_share(thd, table_list, key, key_length, 0, error);
#  endif
#endif
  DBUG_RETURN(share);
}

TABLE_SHARE *mrn_create_tmp_table_share(TABLE_LIST *table_list,
                                        const char *path,
#ifdef MRN_OPEN_TABLE_DEF_USE_TABLE_DEFINITION
                                        const dd::Table *table_def,
#endif
                                        int *error)
{
  uint key_length;
  TABLE_SHARE *share;
  THD *thd = current_thd;

  MRN_DBUG_ENTER_FUNCTION();
#ifdef MRN_HAVE_GET_TABLE_DEF_KEY
  const char *key;
  key_length = get_table_def_key(table_list, &key);
#else
  char key[MAX_DBKEY_LENGTH];
  key_length = create_table_def_key(thd, key, table_list, false);
#endif
  share = mrn_alloc_table_share(table_list, key, key_length);
  if (!share)
  {
    *error = ER_CANT_OPEN_FILE;
    DBUG_RETURN(NULL);
  }
  share->tmp_table = NO_TMP_TABLE; // TODO: is this right?
  share->path.str = (char *) path;
  share->path.length = strlen(share->path.str);
  share->normalized_path.str = mrn_my_strdup(path, MYF(MY_WME));
  share->normalized_path.length = strlen(share->normalized_path.str);
  int open_error = 1;
#ifdef MRN_OPEN_TABLE_DEF_USE_TABLE_DEFINITION
  if (table_def) {
    open_error = open_table_def(thd, share, *table_def);
  }
#else
  open_error = open_table_def(thd, share, GTS_TABLE);
#endif
  if (open_error != 0) {
    char *normalized_path = const_cast<char *>(share->normalized_path.str);
    free_table_share(share);
    my_free(normalized_path);
    *error = ER_CANT_OPEN_FILE;
    DBUG_RETURN(NULL);
  }
  DBUG_RETURN(share);
}

void mrn_free_tmp_table_share(TABLE_SHARE *tmp_table_share)
{
  MRN_DBUG_ENTER_FUNCTION();
  char *normalized_path =
    const_cast<char *>(tmp_table_share->normalized_path.str);
  free_table_share(tmp_table_share);
  my_free(normalized_path);
  DBUG_VOID_RETURN;
}

KEY *mrn_create_key_info_for_table(MRN_SHARE *share, TABLE *table, int *error)
{
  uint *wrap_key_nr = share->wrap_key_nr, i, j;
  KEY *wrap_key_info;
  MRN_DBUG_ENTER_FUNCTION();
  if (share->wrap_keys)
  {
    if (!(wrap_key_info = (KEY *)
      mrn_my_multi_malloc(MYF(MY_WME | MY_ZEROFILL),
        &wrap_key_info, sizeof(*wrap_key_info) * share->wrap_keys,
        NullS))
    ) {
      *error = HA_ERR_OUT_OF_MEM;
      DBUG_RETURN(NULL);
    }
    for (i = 0; i < table->s->keys; i++)
    {
      j = wrap_key_nr[i];
      if (j < MAX_KEY)
      {
        grn_memcpy(&wrap_key_info[j], &table->key_info[i],
                   sizeof(*wrap_key_info));
      }
    }
  } else
    wrap_key_info = NULL;
  *error = 0;
  DBUG_RETURN(wrap_key_info);
}

void mrn_set_bitmap_by_key(MY_BITMAP *map, KEY *key_info)
{
  uint i;
  MRN_DBUG_ENTER_FUNCTION();
  for (i = 0; i < KEY_N_KEY_PARTS(key_info); i++)
  {
    Field *field = key_info->key_part[i].field;
    bitmap_set_bit(map, MRN_FIELD_FIELD_INDEX(field));
  }
  DBUG_VOID_RETURN;
}

st_mrn_slot_data *mrn_get_slot_data(THD *thd, bool can_create)
{
  MRN_DBUG_ENTER_FUNCTION();
  st_mrn_slot_data *slot_data =
    static_cast<st_mrn_slot_data *>(thd_get_ha_data(thd, mrn_hton_ptr));
  if (slot_data == NULL) {
    slot_data =
      static_cast<st_mrn_slot_data *>(malloc(sizeof(st_mrn_slot_data)));
    slot_data->last_insert_record_id = GRN_ID_NIL;
    slot_data->first_wrap_hton = NULL;
    slot_data->alter_create_info = NULL;
    slot_data->disable_keys_create_info = NULL;
    slot_data->alter_connect_string = NULL;
    slot_data->alter_comment = NULL;
    mrn_thd_set_ha_data(thd, mrn_hton_ptr, slot_data);
    {
      mrn::Lock lock(&mrn_allocated_thds_mutex);
      if (grn_hash_add(&mrn_ctx,
                       mrn_allocated_thds,
                       &thd,
                       sizeof(thd),
                       NULL,
                       NULL) == GRN_ID_NIL) {
        free(slot_data);
        DBUG_RETURN(NULL);
      }
    }
  }
  DBUG_RETURN(slot_data);
}

void mrn_clear_slot_data(THD *thd)
{
  MRN_DBUG_ENTER_FUNCTION();
  st_mrn_slot_data *slot_data = mrn_get_slot_data(thd, false);
  if (slot_data) {
    if (slot_data->first_wrap_hton) {
      st_mrn_wrap_hton *tmp_wrap_hton;
      st_mrn_wrap_hton *wrap_hton = slot_data->first_wrap_hton;
      while (wrap_hton)
      {
        tmp_wrap_hton = wrap_hton->next;
        free(wrap_hton);
        wrap_hton = tmp_wrap_hton;
      }
      slot_data->first_wrap_hton = NULL;
    }
    slot_data->alter_create_info = NULL;
    slot_data->disable_keys_create_info = NULL;
    if (slot_data->alter_connect_string) {
      my_free(slot_data->alter_connect_string);
      slot_data->alter_connect_string = NULL;
    }
    if (slot_data->alter_comment) {
      my_free(slot_data->alter_comment);
      slot_data->alter_comment = NULL;
    }
  }
  DBUG_VOID_RETURN;
}

#ifdef __cplusplus
}
#endif
