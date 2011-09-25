/* -*- c-basic-offset: 2 -*- */
/*
  Copyright(C) 2011 Kentoku SHIBA
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

#define MYSQL_SERVER 1
#include "mysql_version.h"

#if MYSQL_VERSION_ID < 50500
#  include <mysql_priv.h>
#  include <mysql/plugin.h>
#else
#  include <sql_priv.h>
#  include <sql_class.h>
#  include <probes_mysql.h>
#  include <sql_partition.h>
#  include <sql_servers.h>
#  include <sql_base.h>
#endif
#include "mrn_err.h"
#include "mrn_sys.h"
#include "mrn_table.h"

#if MYSQL_VERSION_ID >= 50603
#  define MRN_HA_RESOLVE_BY_NAME(name) ha_resolve_by_name(NULL, (name), TRUE)
#else
#  define MRN_HA_RESOLVE_BY_NAME(name) ha_resolve_by_name(NULL, (name))
#endif

#define MRN_DEFAULT_STR "DEFAULT"
#define MRN_DEFAULT_LEN (sizeof(MRN_DEFAULT_STR) - 1)
#define MRN_GROONGA_STR "GROONGA"
#define MRN_GROONGA_LEN (sizeof(MRN_GROONGA_STR) - 1)

extern HASH mrn_open_tables;
extern pthread_mutex_t mrn_open_tables_mutex;
extern char *mrn_default_parser;

char *mrn_create_string(const char *str, uint length)
{
  char *res;
  DBUG_ENTER("mrn_create_string");
  if (!(res = (char*) my_malloc(length + 1, MYF(MY_WME))))
    DBUG_RETURN(NULL);
  memcpy(res, str, length);
  res[length] = '\0';
  DBUG_RETURN(res);
}

char *mrn_get_string_between_quote(char *ptr, bool alloc)
{
  char *start_ptr, *end_ptr, *tmp_ptr, *esc_ptr;
  bool find_flg = FALSE, esc_flg = FALSE;
  DBUG_ENTER("mrn_get_string_between_quote");

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
          find_flg = TRUE;
        else if (esc_ptr == end_ptr - 1)
        {
          esc_flg = TRUE;
          tmp_ptr = end_ptr + 1;
          break;
        } else {
          esc_flg = TRUE;
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
          find_flg = TRUE;
        else if (esc_ptr == end_ptr - 1)
        {
          esc_flg = TRUE;
          tmp_ptr = end_ptr + 1;
          break;
        } else {
          esc_flg = TRUE;
          esc_ptr += 2;
        }
      }
    }
  } else
    DBUG_RETURN(NULL);

  *end_ptr = '\0';
  if (esc_flg)
  {
    esc_ptr = start_ptr;
    while (TRUE)
    {
      esc_ptr = strchr(esc_ptr, '\\');
      if (!esc_ptr)
        break;
      switch(*(esc_ptr + 1))
      {
        case 'b':
          *esc_ptr = '\b';
          break;
        case 'n':
          *esc_ptr = '\n';
          break;
        case 'r':
          *esc_ptr = '\r';
          break;
        case 't':
          *esc_ptr = '\t';
          break;
        default:
          *esc_ptr = *(esc_ptr + 1);
          break;
      }
      esc_ptr++;
      strcpy(esc_ptr, esc_ptr + 1);
    }
  }
  if (alloc)
  {
    DBUG_RETURN(
      mrn_create_string(
      start_ptr,
      strlen(start_ptr))
    );
  } else {
    DBUG_RETURN(start_ptr);
  }
}

#ifdef WITH_PARTITION_STORAGE_ENGINE
void mrn_get_partition_info(const char *table_name, uint table_name_length,
                            const TABLE *table, partition_element **part_elem,
                            partition_element **sub_elem)
{
  char tmp_name[FN_LEN];
  partition_info *part_info = table->part_info;
  partition_element *tmp_part_elem = NULL, *tmp_sub_elem = NULL;
  bool tmp_flg = FALSE, tmp_find_flg = FALSE;
  DBUG_ENTER("mrn_get_partition_info");
  *part_elem = NULL;
  *sub_elem = NULL;
  if (!part_info)
    DBUG_VOID_RETURN;

  if (!memcmp(table_name + table_name_length - 5, "#TMP#", 5))
    tmp_flg = TRUE;

  DBUG_PRINT("info", ("mroonga table_name=%s", table_name));
  List_iterator<partition_element> part_it(part_info->partitions);
  while ((*part_elem = part_it++))
  {
    if ((*part_elem)->subpartitions.elements)
    {
      List_iterator<partition_element> sub_it((*part_elem)->subpartitions);
      while ((*sub_elem = sub_it++))
      {
        create_subpartition_name(tmp_name, table->s->path.str,
          (*part_elem)->partition_name, (*sub_elem)->partition_name,
          NORMAL_PART_NAME);
        DBUG_PRINT("info", ("mroonga tmp_name=%s", tmp_name));
        if (!memcmp(table_name, tmp_name, table_name_length + 1))
          DBUG_VOID_RETURN;
        if (
          tmp_flg &&
          *(tmp_name + table_name_length - 5) == '\0' &&
          !memcmp(table_name, tmp_name, table_name_length - 5)
        ) {
          tmp_part_elem = *part_elem;
          tmp_sub_elem = *sub_elem;
          tmp_flg = FALSE;
          tmp_find_flg = TRUE;
        }
      }
    } else {
      create_partition_name(tmp_name, table->s->path.str,
        (*part_elem)->partition_name, NORMAL_PART_NAME, TRUE);
      DBUG_PRINT("info", ("mroonga tmp_name=%s", tmp_name));
      if (!memcmp(table_name, tmp_name, table_name_length + 1))
        DBUG_VOID_RETURN;
      if (
        tmp_flg &&
        *(tmp_name + table_name_length - 5) == '\0' &&
        !memcmp(table_name, tmp_name, table_name_length - 5)
      ) {
        tmp_part_elem = *part_elem;
        tmp_flg = FALSE;
        tmp_find_flg = TRUE;
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
    DBUG_PRINT("info", ("mroonga "title_name" start")); \
    if (!share->param_name) \
    { \
      if ((share->param_name = mrn_get_string_between_quote( \
        start_ptr, TRUE))) \
        share->MRN_PARAM_STR_LEN(param_name) = strlen(share->param_name); \
      else { \
        error = ER_MRN_INVALID_TABLE_PARAM_NUM; \
        my_printf_error(error, ER_MRN_INVALID_TABLE_PARAM_STR, \
          MYF(0), tmp_ptr); \
        goto error; \
      } \
      DBUG_PRINT("info", ("mroonga "title_name"=%s", share->param_name)); \
    } \
    break; \
  }

#define MRN_PARAM_STR_LIST(title_name, param_name, param_pos) \
  if (!strncasecmp(tmp_ptr, title_name, title_length)) \
  { \
    DBUG_PRINT("info", ("mroonga "title_name" start")); \
    if (!share->param_name[param_pos]) \
    { \
      if ((share->param_name[param_pos] = mrn_get_string_between_quote( \
        start_ptr, TRUE))) \
        share->MRN_PARAM_STR_LEN(param_name)[param_pos] = \
          strlen(share->param_name[param_pos]); \
      else { \
        error = ER_MRN_INVALID_TABLE_PARAM_NUM; \
        my_printf_error(error, ER_MRN_INVALID_TABLE_PARAM_STR, \
          MYF(0), tmp_ptr); \
        goto error; \
      } \
      DBUG_PRINT("info", ("mroonga "title_name"[%d]=%s", param_pos, \
        share->param_name[param_pos])); \
    } \
    break; \
  }

int mrn_parse_table_param(MRN_SHARE *share, TABLE *table)
{
  int i, error;
  int title_length;
  char *param_string = NULL;
  char *sprit_ptr[2];
  char *tmp_ptr, *tmp_ptr2, *start_ptr;
#ifdef WITH_PARTITION_STORAGE_ENGINE
  partition_element *part_elem;
  partition_element *sub_elem;
#endif
  DBUG_ENTER("mrn_parse_table_param");
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
    if (param_string)
    {
      my_free(param_string, MYF(0));
      param_string = NULL;
    }
    switch (i)
    {
#ifdef WITH_PARTITION_STORAGE_ENGINE
      case 4:
        if (!sub_elem || !sub_elem->part_comment)
          continue;
        DBUG_PRINT("info", ("mroonga create sub comment string"));
        if (
          !(param_string = mrn_create_string(
            sub_elem->part_comment,
            strlen(sub_elem->part_comment)))
        ) {
          error = HA_ERR_OUT_OF_MEM;
          goto error_alloc_param_string;
        }
        DBUG_PRINT("info", ("mroonga sub comment string=%s", param_string));
        break;
      case 3:
        if (!part_elem || !part_elem->part_comment)
          continue;
        DBUG_PRINT("info", ("mroonga create part comment string"));
        if (
          !(param_string = mrn_create_string(
            part_elem->part_comment,
            strlen(part_elem->part_comment)))
        ) {
          error = HA_ERR_OUT_OF_MEM;
          goto error_alloc_param_string;
        }
        DBUG_PRINT("info", ("mroonga part comment string=%s", param_string));
        break;
#endif
      case 2:
        if (table->s->comment.length == 0)
          continue;
        DBUG_PRINT("info", ("mroonga create comment string"));
        if (
          !(param_string = mrn_create_string(
            table->s->comment.str,
            table->s->comment.length))
        ) {
          error = HA_ERR_OUT_OF_MEM;
          goto error_alloc_param_string;
        }
        DBUG_PRINT("info", ("mroonga comment string=%s", param_string));
        break;
      default:
        if (table->s->connect_string.length == 0)
          continue;
        DBUG_PRINT("info", ("mroonga create connect_string string"));
        if (
          !(param_string = mrn_create_string(
            table->s->connect_string.str,
            table->s->connect_string.length))
        ) {
          error = HA_ERR_OUT_OF_MEM;
          goto error_alloc_param_string;
        }
        DBUG_PRINT("info", ("mroonga connect_string=%s", param_string));
        break;
    }

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
        case 0:
          continue;
        case 6:
          MRN_PARAM_STR("engine", engine);
          error = ER_MRN_INVALID_TABLE_PARAM_NUM;
          my_printf_error(error, ER_MRN_INVALID_TABLE_PARAM_STR,
            MYF(0), tmp_ptr);
          goto error;
        default:
          error = ER_MRN_INVALID_TABLE_PARAM_NUM;
          my_printf_error(error, ER_MRN_INVALID_TABLE_PARAM_STR,
            MYF(0), tmp_ptr);
          goto error;
      }
    }
  }

  if (share->engine)
  {
    LEX_STRING engine_name;
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
      my_free(share->engine, MYF(0));
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
      share->hton = plugin_data(share->plugin, handlerton *);
      share->wrapper_mode = TRUE;
    }
  }

  if (param_string)
    my_free(param_string, MYF(0));
  DBUG_RETURN(0);

error:
  if (param_string)
    my_free(param_string, MYF(0));
error_alloc_param_string:
  DBUG_RETURN(error);
}

int mrn_add_index_param(MRN_SHARE *share, KEY *key_info, int i)
{
  int error;
  int title_length;
  char *param_string = NULL;
#if MYSQL_VERSION_ID >= 50500
  char *sprit_ptr[2];
  char *tmp_ptr, *tmp_ptr2, *start_ptr;
#endif
  DBUG_ENTER("mrn_add_index_param");

#if MYSQL_VERSION_ID >= 50500
  if (key_info->comment.length == 0)
  {
    if (
      !(share->key_parser[i] = mrn_create_string(
        mrn_default_parser,
        strlen(mrn_default_parser)))
    ) {
      error = HA_ERR_OUT_OF_MEM;
      goto error;
    }
    share->key_parser_length[i] = strlen(share->key_parser[i]);
    DBUG_RETURN(0);
  }
  DBUG_PRINT("info", ("mroonga create comment string"));
  if (
    !(param_string = mrn_create_string(
      key_info->comment.str,
      key_info->comment.length))
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
      case 0:
        continue;
      case 6:
        MRN_PARAM_STR_LIST("parser", key_parser, i);
        error = ER_MRN_INVALID_TABLE_PARAM_NUM;
        my_printf_error(error, ER_MRN_INVALID_TABLE_PARAM_STR,
          MYF(0), tmp_ptr);
        goto error;
      default:
        error = ER_MRN_INVALID_TABLE_PARAM_NUM;
        my_printf_error(error, ER_MRN_INVALID_TABLE_PARAM_STR,
          MYF(0), tmp_ptr);
        goto error;
    }
  }
#endif
  if (!share->key_parser[i]) {
    if (
      !(share->key_parser[i] = mrn_create_string(
        mrn_default_parser,
        strlen(mrn_default_parser)))
    ) {
      error = HA_ERR_OUT_OF_MEM;
      goto error;
    }
    share->key_parser_length[i] = strlen(share->key_parser[i]);
  }

  if (param_string)
    my_free(param_string, MYF(0));
  DBUG_RETURN(0);

error:
  if (param_string)
    my_free(param_string, MYF(0));
error_alloc_param_string:
  DBUG_RETURN(error);
}

int mrn_parse_index_param(MRN_SHARE *share, TABLE *table)
{
  int i, error;
  DBUG_ENTER("mrn_parse_index_param");
  for (i = 0; i < table->s->keys; i++)
  {
    KEY *key_info = &table->s->key_info[i];

    if (!(key_info->flags & HA_FULLTEXT)) {
      continue;
    }

    if ((error = mrn_add_index_param(share, key_info, i)))
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
  DBUG_ENTER("mrn_free_share_alloc");
  if (share->engine)
    my_free(share->engine, MYF(0));
  for (i = 0; i < share->table_share->keys; i++)
  {
    if (share->key_parser[i])
      my_free(share->key_parser[i], MYF(0));
  }
  DBUG_RETURN(0);
}

MRN_SHARE *mrn_get_share(const char *table_name, TABLE *table, int *error)
{
  MRN_SHARE *share;
  char *tmp_name, **key_parser;
  uint length, *wrap_key_nr, *key_parser_length, i, j;
  KEY *wrap_key_info;
  TABLE_SHARE *wrap_table_share;
  DBUG_ENTER("mrn_get_share");
  length = (uint) strlen(table_name);
  pthread_mutex_lock(&mrn_open_tables_mutex);
  if (!(share = (MRN_SHARE*) my_hash_search(&mrn_open_tables,
    (uchar*) table_name, length)))
  {
    if (!(share = (MRN_SHARE *)
      my_multi_malloc(MYF(MY_WME | MY_ZEROFILL),
        &share, sizeof(*share),
        &tmp_name, length + 1,
        &key_parser, sizeof(char *) * table->s->keys,
        &key_parser_length, sizeof(uint) * table->s->keys,
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
    share->key_parser = key_parser;
    share->key_parser_length = key_parser_length;
    strmov(share->table_name, table_name);
    share->table_share = table->s;

    if (
      (*error = mrn_parse_table_param(share, table)) ||
      (*error = mrn_parse_index_param(share, table))
    )
      goto error_parse_table_param;

    if (share->wrapper_mode)
    {
      j = 0;
      for (i = 0; i < table->s->keys; i++)
      {
        if (table->s->key_info[i].algorithm != HA_KEY_ALG_FULLTEXT)
        {
          wrap_key_nr[i] = j;
          memcpy(&wrap_key_info[j], &table->s->key_info[i],
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
      memcpy(wrap_table_share, table->s, sizeof(*wrap_table_share));
      wrap_table_share->keys = share->wrap_keys;
      wrap_table_share->key_info = share->wrap_key_info;
      wrap_table_share->primary_key = share->wrap_primary_key;
      share->wrap_table_share = wrap_table_share;
    }

    if (pthread_mutex_init(&share->mutex, MY_MUTEX_INIT_FAST))
    {
      *error = HA_ERR_OUT_OF_MEM;
      goto error_init_mutex;
    }
    thr_lock_init(&share->lock);
    if (my_hash_insert(&mrn_open_tables, (uchar*) share))
    {
      *error = HA_ERR_OUT_OF_MEM;
      goto error_hash_insert;
    }
  }
  share->use_count++;
  pthread_mutex_unlock(&mrn_open_tables_mutex);
  DBUG_RETURN(share);

error_hash_insert:
  pthread_mutex_destroy(&share->mutex);
error_init_mutex:
error_parse_table_param:
  mrn_free_share_alloc(share);
  my_free(share, MYF(0));
error_alloc_share:
  pthread_mutex_unlock(&mrn_open_tables_mutex);
  DBUG_RETURN(NULL);
}

int mrn_free_share(MRN_SHARE *share)
{
  DBUG_ENTER("mrn_free_share");
  pthread_mutex_lock(&mrn_open_tables_mutex);
  if (!--share->use_count)
  {
    my_hash_delete(&mrn_open_tables, (uchar*) share);
    if (share->wrapper_mode)
      plugin_unlock(NULL, share->plugin);
    mrn_free_share_alloc(share);
    thr_lock_delete(&share->lock);
    pthread_mutex_destroy(&share->mutex);
    my_free(share, MYF(0));
  }
  pthread_mutex_unlock(&mrn_open_tables_mutex);
  DBUG_RETURN(0);
}

TABLE_SHARE *mrn_get_table_share(TABLE_LIST *table_list, int *error)
{
  uint key_length;
  TABLE_SHARE *share;
#if MYSQL_VERSION_ID >= 50500
  my_hash_value_type hash_value;
#endif
  THD *thd = current_thd;
  DBUG_ENTER("mrn_get_table_share");
#if MYSQL_VERSION_ID >= 50603
  const char *key;
  key_length = get_table_def_key(table_list, &key);
#else
  char key[MAX_DBKEY_LENGTH];
  key_length = create_table_def_key(thd, key, table_list, FALSE);
#endif
#if MYSQL_VERSION_ID >= 50500
  hash_value = my_calc_hash(&table_def_cache, (uchar*) key, key_length);
  share = get_table_share(thd, table_list, key, key_length, 0, error,
                          hash_value);
#else
  share = get_table_share(thd, table_list, key, key_length, 0, error);
#endif
  DBUG_RETURN(share);
}

void mrn_free_table_share(TABLE_SHARE *share)
{
  DBUG_ENTER("mrn_free_table_share");
#if MYSQL_VERSION_ID >= 50500
  release_table_share(share);
#else
  release_table_share(share, RELEASE_NORMAL);
#endif
  DBUG_VOID_RETURN;
}

KEY *mrn_create_key_info_for_table(MRN_SHARE *share, TABLE *table, int *error)
{
  uint *wrap_key_nr = share->wrap_key_nr, i, j;
  KEY *wrap_key_info;
  DBUG_ENTER("mrn_create_key_info_for_table");
  if (share->wrap_keys)
  {
    if (!(wrap_key_info = (KEY *)
      my_multi_malloc(MYF(MY_WME | MY_ZEROFILL),
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
        memcpy(&wrap_key_info[j], &table->key_info[i],
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
  DBUG_ENTER("mrn_set_bitmap_by_key");
  for (i = 0; i < key_info->key_parts; i++)
  {
    Field *field = key_info->key_part[i].field;
    bitmap_set_bit(map, field->field_index);
  }
  DBUG_VOID_RETURN;
}
