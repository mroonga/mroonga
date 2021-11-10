/* -*- c-basic-offset: 2 -*- */
/*
  Copyright(C) 2011-2021  Sutou Kouhei <kou@clear-code.com>
  Copyright(C) 2020-2021  Horimoto Yasuhiro <horimoto@clear-code.com>

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

#pragma once

#include "mrn_mysql.h"

#if MYSQL_VERSION_ID >= 50604
#  define MRN_HAVE_MYSQL_TYPE_TIMESTAMP2
#  define MRN_HAVE_MYSQL_TYPE_DATETIME2
#  define MRN_HAVE_MYSQL_TYPE_TIME2
#endif

#if MYSQL_VERSION_ID >= 50709 && !defined(MRN_MARIADB_P)
#  define MRN_HAVE_MYSQL_TYPE_JSON
#endif

#if MYSQL_VERSION_ID >= 100302 && defined(MRN_MARIADB_P)
#  define MRN_HAVE_MYSQL_TYPE_VARCHAR_COMPRESSED
#  define MRN_HAVE_MYSQL_TYPE_BLOB_COMPRESSED
#endif

#if MYSQL_VERSION_ID >= 80002 && !defined(MRN_MARIADB_P)
#  define MRN_HAVE_SQL_DD_TYPES_TABLE_H
#endif

#if MYSQL_VERSION_ID >= 80019 && !defined(MRN_MARIADB_P)
#  define MRN_HAVE_CREATE_FIELD_H
#endif

#if MYSQL_VERSION_ID < 50603
  typedef MYSQL_ERROR Sql_condition;
#endif

#if defined(MRN_MARIADB_P)
#  if MYSQL_VERSION_ID < 100000
     typedef COST_VECT Cost_estimate;
#  endif
#endif

#ifndef MRN_MARIADB_P
  typedef char *range_id_t;
#endif

#if MYSQL_VERSION_ID >= 50607
#  define MRN_SUPPORT_FOREIGN_KEYS 1
#endif

#if MYSQL_VERSION_ID >= 50609
#  define MRN_KEY_HAVE_USER_DEFINED_KEYPARTS
#endif

#ifdef MRN_KEY_HAVE_USER_DEFINED_KEYPARTS
#  define KEY_N_KEY_PARTS(key) (key)->user_defined_key_parts
#else
#  define KEY_N_KEY_PARTS(key) (key)->key_parts
#endif

#if (MYSQL_VERSION_ID >= 50706 && !defined(MRN_MARIADB_P)) || \
    (MYSQL_VERSION_ID >= 100302)
#  define MRN_FOREIGN_KEY_USE_CONST_STRING
#endif

#ifdef MRN_FOREIGN_KEY_USE_CONST_STRING
  typedef LEX_CSTRING mrn_foreign_key_name;
#else
  typedef LEX_STRING mrn_foreign_key_name;
#endif

#if MYSQL_VERSION_ID >= 100302 && defined(MRN_MARIADB_P)
#  define MRN_KEY_NAME_IS_LEX_STRING
#  define MRN_FIELD_FIELD_NAME_IS_LEX_STRING
#  define MRN_FIELD_SET_USE_LEX_STRING
#  define MRN_CREATE_FIELD_USE_LEX_STRING
#endif

#ifdef MRN_KEY_NAME_IS_LEX_STRING
#  define KEY_NAME_PTR(key) ((key)->name.str)
#  define KEY_NAME_LENGTH(key) ((key)->name.length)
#  define KEY_NAME_EQUAL_KEY(key1, key2)                \
  ((key1)->name.length == (key2)->name.length &&        \
   strncmp((key1)->name.str,                            \
           (key2)->name.str,                            \
           (key1)->name.length) == 0)
#  define KEY_NAME_FORMAT "%.*s"
#  define KEY_NAME_FORMAT_VALUE(key)            \
  static_cast<int>((key)->name.length),         \
  (key)->name.str
#else
#  define KEY_NAME_PTR(key) (key)->name
#  define KEY_NAME_LENGTH(key) strlen((key)->name)
#  define KEY_NAME_EQUAL_KEY(key1, key2)        \
  (strcmp((key1)->name, (key2)->name) == 0)
#  define KEY_NAME_FORMAT "%s"
#  define KEY_NAME_FORMAT_VALUE(key) (key)->name
#endif
#define KEY_NAME(key) KEY_NAME_PTR(key), KEY_NAME_LENGTH(key)

#ifdef MRN_FIELD_FIELD_NAME_IS_LEX_STRING
#  define FIELD_NAME(field)                             \
  (field)->field_name.str, (field)->field_name.length
#  define FIELD_NAME_EQUAL(field, name)                                 \
  ((field)->field_name.length == strlen(name) &&                        \
   strncmp((field)->field_name.str, name, (field)->field_name.length) == 0)
#  define FIELD_NAME_EQUAL_STRING(field, string)                        \
  ((field)->field_name.length == string->length &&                      \
   strncmp((field)->field_name.str, string->str, string->length) == 0)
#  define FIELD_NAME_EQUAL_FIELD(field1, field2)                        \
  ((field1)->field_name.length == (field2)->field_name.length &&        \
   strncmp((field1)->field_name.str,                                    \
           (field2)->field_name.str,                                    \
           (field1)->field_name.length) == 0)
#  define FIELD_NAME_TO_LEX_STRING(thd, field)                          \
  thd_make_lex_string((thd),                                            \
                      NULL,                                             \
                      (field)->field_name.str,                          \
                      (field)->field_name.length,                       \
                      true);
#  define FIELD_NAME_FORMAT "%.*s"
#  define FIELD_NAME_FORMAT_VALUE(field)                                \
  static_cast<int>((field)->field_name.length), (field)->field_name.str
#else
#  define FIELD_NAME(field)                             \
  (field)->field_name, strlen((field)->field_name)
#  define FIELD_NAME_EQUAL(field, name)                                 \
  (strcmp((field)->field_name, name) == 0)
#  define FIELD_NAME_EQUAL_STRING(field, string)                        \
  (strlen((field)->field_name) == string->length &&                     \
   strncmp((field)->field_name, string->str, string->length) == 0)
#  define FIELD_NAME_EQUAL_FIELD(field1, field2)                        \
  (strcmp((field1)->field_name, (field2)->field_name) == 0)
#  define FIELD_NAME_TO_LEX_STRING(thd, field)          \
   thd_make_lex_string((thd),                           \
                       NULL,                            \
                       (field)->field_name,             \
                       strlen((field)->field_name),     \
                       true);
#  define FIELD_NAME_FORMAT "%s"
#  define FIELD_NAME_FORMAT_VALUE(field) (field)->field_name
#endif

#if (MYSQL_VERSION_ID >= 100302 && defined(MRN_MARIADB_P)) ||   \
  (MYSQL_VERSION_ID >= 80011 && !defined(MRN_MARIADB_P))
#  define MRN_KEY_PART_SPEC_FIELD_NAME_USE_CONST_STRING
#endif

#ifdef MRN_KEY_PART_SPEC_FIELD_NAME_USE_CONST_STRING
#  if (MYSQL_VERSION_ID >= 80013 && !defined(MRN_MARIADB_P))
     typedef char mrn_key_part_spec_field_name;
#  else
     typedef LEX_CSTRING mrn_key_part_spec_field_name;
#  endif
#else
  typedef LEX_STRING mrn_key_part_spec_field_name;
#endif

#if MYSQL_VERSION_ID >= 80013 && !defined(MRN_MARIADB_P)
#  define MRN_KEY_PART_SPEC_FIELD_NAME_USE_ACCESSSOR_FUNCTION
#  define MRN_KEY_PART_SPEC_FIELD_NAME(key_part_spec)			\
     (key_part_spec)->get_field_name()
#  define MRN_KEY_PART_SPEC_FIELD_NAME_FORMAT "%s"
#  define MRN_KEY_PART_SPEC_FIELD_NAME_VALUE(key_part_spec)		\
     (key_part_spec)
#  define MRN_FIELD_NAME_EQUAL_KEY_PART_SPEC_FIELD_NAME(field, name)	\
     (strcmp((field)->field_name, name) == 0)
#else
#  define MRN_KEY_PART_SPEC_FIELD_NAME(key_part_spec)			\
     &(key_part_spec)->field_name
#  define MRN_KEY_PART_SPEC_FIELD_NAME_FORMAT "%.*s"
#  define MRN_KEY_PART_SPEC_FIELD_NAME_VALUE(key_part_spec)		\
     static_cast<int>((key_part_spec)->length), (key_part_spec)->str
#  ifdef MRN_FIELD_FIELD_NAME_IS_LEX_STRING
#    define MRN_FIELD_NAME_EQUAL_KEY_PART_SPEC_FIELD_NAME(field, string)	\
       FIELD_NAME_EQUAL_STRING(field, string)
#  else
#    define MRN_FIELD_NAME_EQUAL_KEY_PART_SPEC_FIELD_NAME(field, string)	\
       (strlen((field)->field_name) == string->length &&			\
        strncmp((field)->field_name, string->str, string->length) == 0)
#  endif
#endif

#if MYSQL_VERSION_ID >= 100302 && defined(MRN_MARIADB_P)
#  define MRN_THD_MAKE_LEX_STRING_USE_CONST_STRING
#endif

#ifdef MRN_THD_MAKE_LEX_STRING_USE_CONST_STRING
  typedef MYSQL_CONST_LEX_STRING mrn_thd_lex_string;
#else
  typedef MYSQL_LEX_STRING mrn_thd_lex_string;
#endif

#if defined(MRN_MARIADB_P) && MYSQL_VERSION_ID >= 100504
#  define mrn_init_alloc_root(root, name, block_size, pre_alloc_size, flags) \
  init_alloc_root(mrn_memory_key, root, block_size, pre_alloc_size, flags)
#elif defined(MRN_MARIADB_P) && MYSQL_VERSION_ID >= 100306
#  define mrn_init_alloc_root(root, name, block_size, pre_alloc_size, flags) \
  init_alloc_root(root, name, block_size, pre_alloc_size, flags)
#elif defined(MRN_MARIADB_P) && MYSQL_VERSION_ID >= 100000
#  define mrn_init_alloc_root(root, name, block_size, pre_alloc_size, flags) \
  init_alloc_root(root, block_size, pre_alloc_size, flags)
#elif MYSQL_VERSION_ID >= 80027
#  define mrn_init_alloc_root(root, name, block_size, pre_alloc_size, flags) \
  ::new ((void *)root) MEM_ROOT(mrn_memory_key, block_size)
#else
#  define mrn_init_alloc_root(root, name, block_size, pre_alloc_size, flags) \
  init_alloc_root(mrn_memory_key, root, block_size, pre_alloc_size)
#endif

#if MYSQL_VERSION_ID < 100002 || !defined(MRN_MARIADB_P)
#  define GTS_TABLE 0
#endif

#if MYSQL_VERSION_ID >= 50607
#  if MYSQL_VERSION_ID >= 100007 && defined(MRN_MARIADB_P)
#    define MRN_GET_ERROR_MESSAGE thd_get_error_message(current_thd)
#    define MRN_GET_ERROR_NUMBER thd_get_error_number(current_thd)
#    define MRN_GET_CURRENT_ROW_FOR_WARNING(thd) thd_get_error_row(thd)
#  else
#    define MRN_GET_ERROR_MESSAGE current_thd->get_stmt_da()->message()
#    define MRN_GET_ERROR_NUMBER current_thd->get_stmt_da()->sql_errno()
#    if MYSQL_VERSION_ID >= 50706
#      define MRN_GET_CURRENT_ROW_FOR_WARNING(thd) \
  thd->get_stmt_da()->current_row_for_condition()
#    else
#      define MRN_GET_CURRENT_ROW_FOR_WARNING(thd) \
  thd->get_stmt_da()->current_row_for_warning()
#    endif
#  endif
#else
#  define MRN_GET_ERROR_MESSAGE current_thd->stmt_da->message()
#  define MRN_GET_ERROR_NUMBER current_thd->stmt_da->sql_errno()
#  define MRN_GET_CURRENT_ROW_FOR_WARNING(thd) thd->warning_info->current_row_for_warning()
#endif

#if MYSQL_VERSION_ID >= 50607 && !defined(MRN_MARIADB_P)
#  define MRN_ITEM_HAVE_ITEM_NAME
#endif

#if MYSQL_VERSION_ID >= 100302 && defined(MRN_MARIADB_P)
#  define MRN_ITEM_ITEM_NAME_IS_LEX_STRING
#endif

#if MYSQL_VERSION_ID < 100000
#  define MRN_HAVE_TABLE_DEF_CACHE
#  if MYSQL_VERSION_ID >= 80011 && !defined(MRN_MARIADB_P)
#    define MRN_TABLE_DEF_CACHE_TYPE_IS_MAP
#  endif

#  ifdef MRN_TABLE_DEF_CACHE_TYPE_IS_MAP
struct Table_share_deleter;
typedef
  malloc_unordered_map<std::string,
                       std::unique_ptr<TABLE_SHARE, Table_share_deleter>> *
  mrn_table_def_cache_type;
#  else
typedef HASH mrn_table_def_cache_type;
#  endif
#endif

#if defined(MRN_MARIADB_P) && MYSQL_VERSION_ID >= 100009
#  define MRN_HAVE_TDC_ACQUIRE_SHARE
#  if MYSQL_VERSION_ID < 100200
#    define MRN_TDC_ACQUIRE_SHARE_REQUIRE_KEY
#  endif
#endif

#if MYSQL_VERSION_ID >= 50613
#  define MRN_HAVE_ALTER_INFO
#endif

#if MYSQL_VERSION_ID >= 50603
#  define MRN_HAVE_GET_TABLE_DEF_KEY
#endif

#if defined(MRN_MARIADB_P) && MYSQL_VERSION_ID >= 100004
#  define MRN_TABLE_SHARE_HAVE_LOCK_SHARE
#endif

#ifndef TIME_FUZZY_DATE
/* For MariaDB 10. */
#  ifdef TIME_FUZZY_DATES
#    define TIME_FUZZY_DATE TIME_FUZZY_DATES
#  endif
#endif

#if MYSQL_VERSION_ID >= 100007 && defined(MRN_MARIADB_P)
#  define MRN_USE_MYSQL_DATA_HOME
#endif

#if MYSQL_VERSION_ID >= 50706 && !defined(MRN_MARIADB_P)
#  define MRN_SEVERITY_WARNING Sql_condition::SL_WARNING
#else
#  define MRN_SEVERITY_WARNING Sql_condition::WARN_LEVEL_WARN
#endif

#if (MYSQL_VERSION_ID >= 50706 && !defined(MRN_MARIADB_P)) || \
  (MYSQL_VERSION_ID >= 100504 && defined(MRN_MARIADB_P))
#  define MRN_HAVE_PSI_MEMORY_KEY
#endif

#ifdef HAVE_PSI_INTERFACE
#  define MRN_HAVE_PSI_FILE_KEY
#endif

#ifdef MRN_HAVE_PSI_MEMORY_KEY
#  define mrn_my_malloc(size, flags) \
  my_malloc(mrn_memory_key, size, flags)
#  define mrn_my_strdup(string, flags) \
  my_strdup(mrn_memory_key, string, flags)
#  define mrn_my_strndup(string, size, flags) \
  my_strndup(mrn_memory_key, string, size, flags)
#  define mrn_my_multi_malloc(flags, ...) \
  my_multi_malloc(mrn_memory_key, flags, __VA_ARGS__)
#else
#  define mrn_my_malloc(size, flags) my_malloc(size, flags)
#  define mrn_my_strdup(string, flags) my_strdup(string, flags)
#  define mrn_my_strndup(string, size, flags) \
  my_strndup(string, size, flags)
#  define mrn_my_multi_malloc(flags, ...) \
  my_multi_malloc(flags, __VA_ARGS__)
#endif

#if MYSQL_VERSION_ID >= 50706 && !defined(MRN_MARIADB_P)
#  define MRN_STRING_FREE(string) string.mem_free();
#else
#  define MRN_STRING_FREE(string) string.free();
#endif

#if MYSQL_VERSION_ID >= 50706 && !defined(MRN_MARIADB_P)
#  define MRN_THD_DB_PATH(thd) ((thd)->db().str)
#elif defined(MRN_MARIADB_P) && MYSQL_VERSION_ID >= 100306
#  define MRN_THD_DB_PATH(thd) ((thd)->db.str)
#else
#  define MRN_THD_DB_PATH(thd) ((thd)->db)
#endif

#ifndef INT_MAX64
#  define INT_MAX64 LONGLONG_MAX
#endif

#ifdef UINT_MAX
#  define UINT_MAX64 UINT_MAX
#else
#  define UINT_MAX64 LONGLONG_MAX
#endif

#if MYSQL_VERSION_ID >= 50706 && !defined(MRN_MARIADB_P)
#  define mrn_my_stpmov(dst, src) my_stpmov(dst, src)
#else
#  define mrn_my_stpmov(dst, src) strmov(dst, src)
#endif

#if MYSQL_VERSION_ID >= 50607
#  if !defined(MRN_MARIADB_P)
#    define MRN_HAVE_SQL_OPTIMIZER_H
#  endif
#endif

#if MYSQL_VERSION_ID >= 50600 && !defined(MRN_MARIADB_P)
#  define MRN_HAVE_BINLOG_H
#endif

#if MYSQL_VERSION_ID >= 50720 && !defined(MRN_MARIADB_P)
#  define MRN_HAVE_MYSQL_PSI_MYSQL_MEMORY_H
#endif

#if MYSQL_VERSION_ID >= 80011 && !defined(MRN_MARIADB_P)
#  define MRN_HAVE_SQL_DERROR_H
#endif

#if !defined(MRN_MARIADB_P) ||                                     \
  (defined(MRN_MARIADB_P) && MYSQL_VERSION_ID >= 100000)
#  define MRN_HAVE_MY_BYTEORDER_H
#endif

#if (defined(MRN_MARIADB_P) && MYSQL_VERSION_ID >= 100504)
#  define MRN_HAVE_SQL_TYPE_GEOM_H
#endif

#if MYSQL_VERSION_ID >= 50706 && !defined(MRN_MARIADB_P)
#  define MRN_HAVE_SPATIAL
#  if MYSQL_VERSION_ID >= 80000
#    define MRN_HAVE_SRID
#    include <sql/gis/srid.h>
using mrn_srid = gis::srid_t;
#    define MRN_FIELD_GEOM_GET_SRID(field)      \
  (field->get_srid().has_value() ?              \
   field->get_srid().value() :                  \
   0)
#    define MRN_HAVE_SRS
#  else
using mrn_srid = uint32_t;
#    define MRN_FIELD_GEOM_GET_SRID(field) 0
#  endif
#elif defined(HAVE_SPATIAL)
#  define MRN_HAVE_SPATIAL
#  define MRN_HAVE_SRID
typedef uint mrn_srid;
#    define MRN_FIELD_GEOM_GET_SRID(field) (field->get_srid())
#endif

#if MYSQL_VERSION_ID >= 50706 && !defined(MRN_MARIADB_P)
#  define MRN_FORMAT_STRING_LENGTH "zu"
#else
#  define MRN_FORMAT_STRING_LENGTH "u"
#endif

#ifdef MRN_MARIADB_P
#  define MRN_SUPPORT_CUSTOM_OPTIONS
#endif

#ifdef MRN_MARIADB_P
#  define MRN_HAVE_ITEM_EQUAL_FIELDS_ITERATOR
#endif

#if MYSQL_VERSION_ID >= 50706 && !defined(MRN_MARIADB_P)
#  define MRN_QUERY_BLOCK_GET_WHERE_COND(query_block) \
  ((query_block)->where_cond())
#  define MRN_QUERY_BLOCK_GET_HAVING_COND(query_block) \
  ((query_block)->having_cond())
#  define MRN_QUERY_BLOCK_GET_ACTIVE_OPTIONS(query_block) \
  ((query_block)->active_options())
#else
#  define MRN_QUERY_BLOCK_GET_WHERE_COND(query_block) \
  ((query_block)->where)
#  define MRN_QUERY_BLOCK_GET_HAVING_COND(query_block) \
  ((query_block)->having)
#  define MRN_QUERY_BLOCK_GET_ACTIVE_OPTIONS(query_block) \
  ((query_block)->options)
#endif

#if MYSQL_VERSION_ID >= 80021 && !defined(MRN_MARIADB_P)
#  define MRN_QUERY_BLOCK_GET_FIELDS_LIST(query_block) \
  ((query_block)->fields_list)
#else
#  define MRN_QUERY_BLOCK_GET_FIELDS_LIST(query_block) \
  ((query_block)->item_list)
#endif

#if MYSQL_VERSION_ID >= 80022 && !defined(MRN_MARIADB_P)
#  define MRN_QUERY_BLOCK_GET_NUM_VISIBLE_FIELDS(query_block) \
  ((unsigned int)(query_block->num_visible_fields()))
#else
#  define MRN_QUERY_BLOCK_GET_NUM_VISIBLE_FIELDS(query_block) \
  (MRN_QUERY_BLOCK_GET_FIELDS_LIST(query_block).elements)
#endif

#if MYSQL_VERSION_ID >= 80022 && !defined(MRN_MARIADB_P)
#  define MRN_QUERY_BLOCK_GET_FIRST_VISIBLE_FIELD(query_block) \
  (*((query_block)->visible_fields().begin()))
#else
#  define MRN_QUERY_BLOCK_GET_FIRST_VISIBLE_FIELD(query_block) \
  (static_cast<Item *>(MRN_QUERY_BLOCK_GET_FIELDS_LIST(query_block).first_node()->info))
#endif

#if MYSQL_VERSION_ID >= 80022 && !defined(MRN_MARIADB_P)
#  define MRN_QUERY_BLOCK_HAS_LIMIT(query_block) \
  (query_block->has_limit())
#elif MYSQL_VERSION_ID >= 100603
#  define MRN_QUERY_BLOCK_HAS_LIMIT(query_block) \
  (query_block->limit_params.explicit_limit)
#else
#  define MRN_QUERY_BLOCK_HAS_LIMIT(query_block) \
  (query_block->explicit_limit)
#endif

#if MYSQL_VERSION_ID >= 100603 && defined(MRN_MARIADB_P)
#  define MRN_QUERY_BLOCK_SELECT_LIMIT(query_block) \
  (query_block->limit_params.select_limit)
#  define MRN_QUERY_BLOCK_OFFSET_LIMIT(query_block) \
  (query_block->limit_params.offset_limit)
#else
#  define MRN_QUERY_BLOCK_SELECT_LIMIT(query_block) \
  (query_block->select_limit)
#  define MRN_QUERY_BLOCK_OFFSET_LIMIT(query_block) \
  (query_block->offset_limit)
#endif

#if defined(MRN_MARIADB_P) && MYSQL_VERSION_ID >= 100000
#  if MYSQL_VERSION_ID >= 100504
#    define mrn_init_sql_alloc(thd, name, mem_root)                     \
  init_sql_alloc(mrn_memory_key,                                        \
                 mem_root,                                              \
                 TABLE_ALLOC_BLOCK_SIZE,                                \
                 0,                                                     \
                 MYF(thd->slave_thread ? 0 : MY_THREAD_SPECIFIC))
#  elif MYSQL_VERSION_ID >= 100306
#    define mrn_init_sql_alloc(thd, name, mem_root)                     \
  init_sql_alloc(mem_root,                                              \
                 name,                                                  \
                 TABLE_ALLOC_BLOCK_SIZE,                                \
                 0,                                                     \
                 MYF(thd->slave_thread ? 0 : MY_THREAD_SPECIFIC))
#  elif MYSQL_VERSION_ID >= 100104
#    define mrn_init_sql_alloc(thd, name, mem_root)                     \
  init_sql_alloc(mem_root,                                              \
                 TABLE_ALLOC_BLOCK_SIZE,                                \
                 0,                                                     \
                 MYF(thd->slave_thread ? 0 : MY_THREAD_SPECIFIC))
#  else
#    define mrn_init_sql_alloc(thd, name, mem_root)      \
  init_sql_alloc(mem_root,                              \
                 TABLE_ALLOC_BLOCK_SIZE,                \
                 0,                                     \
                 MYF(0))
#  endif
#else
#  if MYSQL_VERSION_ID >= 50709
#    define mrn_init_sql_alloc(thd, name, mem_root)     \
  init_sql_alloc(mrn_memory_key,                        \
                 mem_root,                              \
                 TABLE_ALLOC_BLOCK_SIZE,                \
                 0)
#  else
#    define mrn_init_sql_alloc(thd, name, mem_root)     \
  init_sql_alloc(mem_root,                              \
                 TABLE_ALLOC_BLOCK_SIZE,                \
                 0)
#  endif
#endif

#ifdef MRN_MARIADB_P
#  define MRN_ABORT_ON_WARNING(thd) thd->abort_on_warning
#else
#  if MYSQL_VERSION_ID >= 50706
#    define MRN_ABORT_ON_WARNING(thd) thd->is_strict_mode()
#  else
#    define MRN_ABORT_ON_WARNING(thd) thd->abort_on_warning
#  endif
#endif

#define MRN_ERROR_CODE_DATA_TRUNCATE(thd)                               \
  (MRN_ABORT_ON_WARNING(thd) ? ER_WARN_DATA_OUT_OF_RANGE : WARN_DATA_TRUNCATED)

#if defined(MRN_MARIADB_P) && MYSQL_VERSION_ID >= 100000
#  define mrn_strconvert(from_cs,               \
                         from,                  \
                         from_length,           \
                         to_cs,                 \
                         to,                    \
                         to_length,             \
                         errors)                \
  strconvert((from_cs),                         \
             (from),                            \
             (from_length),                     \
             (to_cs),                           \
             (to),                              \
             (to_length),                       \
             (errors))
#else
#  define mrn_strconvert(from_cs,               \
                         from,                  \
                         from_length,           \
                         to_cs,                 \
                         to,                    \
                         to_length,             \
                         errors)                \
  strconvert((from_cs),                         \
             (from),                            \
             (to_cs),                           \
             (to),                              \
             (to_length),                       \
             (errors))
#endif

#if MYSQL_VERSION_ID >= 50717 && !defined(MRN_MARIADB_P)
#  define mrn_is_directory_separator(c)         \
  is_directory_separator((c))
#else
#  define mrn_is_directory_separator(c)         \
  (c == FN_LIBCHAR || c == FN_LIBCHAR2)
#endif

#if ((MYSQL_VERSION_ID < 50636) || \
    (MYSQL_VERSION_ID >= 50700 && MYSQL_VERSION_ID < 50718)) && !defined(MRN_MARIADB_P)
#  define MRN_HAVE_MYSQL_FIELD_PART_OF_KEY_NOT_CLUSTERED
#endif

#if defined(MRN_MARIADB_P) &&                                           \
  ((MYSQL_VERSION_ID >= 100207) ||                                      \
   ((MYSQL_VERSION_ID >= 100126) && (MYSQL_VERSION_ID < 100200)) ||     \
   ((MYSQL_VERSION_ID >= 100032) && (MYSQL_VERSION_ID < 100100)) ||     \
   ((MYSQL_VERSION_ID >= 50557) && (MYSQL_VERSION_ID < 100000)))
#  define mrn_create_partition_name(out,                                \
                                    out_length,                         \
                                    in1,                                \
                                    in2,                                \
                                    name_variant,                       \
                                    translate)                          \
  create_partition_name(out, out_length, in1, in2, name_variant, translate)
#  define mrn_create_subpartition_name(out,             \
                                       out_length,      \
                                       in1,             \
                                       in2,             \
                                       in3,             \
                                       name_variant)    \
  create_subpartition_name(out, out_length, in1, in2, in3, name_variant)
#else
#  define mrn_create_partition_name(out,                                \
                                    out_length,                         \
                                    in1,                                \
                                    in2,                                \
                                    name_variant,                       \
                                    translate)                          \
  (create_partition_name(out, in1, in2, name_variant, translate), 0)
#  define mrn_create_subpartition_name(out,             \
                                       out_length,      \
                                       in1,             \
                                       in2,             \
                                       in3,             \
                                       name_variant)    \
  (create_subpartition_name(out, in1, in2, in3, name_variant), 0)
#endif

#if (!defined(MRN_MARIADB_P) && MYSQL_VERSION_ID >= 80024)
#  define ITEM_SUM_GET_NEST_LEVEL(sum_item) (sum_item)->base_query_block->nest_level
#  define ITEM_SUM_GET_AGGR_LEVEL(sum_item) (sum_item)->aggr_query_block->nest_level
#  define ITEM_SUM_GET_MAX_AGGR_LEVEL(sum_item) (sum_item)->max_aggr_level
#elif (!defined(MRN_MARIADB_P) && MYSQL_VERSION_ID >= 80002)
#  define ITEM_SUM_GET_NEST_LEVEL(sum_item) (sum_item)->base_select->nest_level
#  define ITEM_SUM_GET_AGGR_LEVEL(sum_item) (sum_item)->aggr_select->nest_level
#  define ITEM_SUM_GET_MAX_AGGR_LEVEL(sum_item) (sum_item)->max_aggr_level
#else
#  define ITEM_SUM_GET_NEST_LEVEL(sum_item) (sum_item)->nest_level
#  define ITEM_SUM_GET_AGGR_LEVEL(sum_item) (sum_item)->aggr_level
#  define ITEM_SUM_GET_MAX_AGGR_LEVEL(sum_item) (sum_item)->max_arg_level
#endif

#if (!defined(MRN_MARIADB_P) && MYSQL_VERSION_ID >= 80002)
  typedef bool mrn_bool;
#else
  typedef my_bool mrn_bool;
#endif

#define MRN_ALLOCATE_VARIABLE_LENGTH_ARRAYS(type, variable_name, variable_size) \
  type *variable_name =                                                 \
    (type *)mrn_my_malloc(sizeof(type) * (variable_size), MYF(MY_WME))
#define MRN_FREE_VARIABLE_LENGTH_ARRAYS(variable_name) \
  my_free(variable_name)

#if (defined(HA_CAN_VIRTUAL_COLUMNS) || defined(HA_GENERATED_COLUMNS))
#  define MRN_SUPPORT_GENERATED_COLUMNS
#endif

#ifdef MRN_MARIADB_P
#  if (MYSQL_VERSION_ID >= 100200)
#    define MRN_GENERATED_COLUMNS_FIELD_IS_VIRTUAL(field) \
       (!field->stored_in_db())
#    define MRN_GENERATED_COLUMNS_FIELD_IS_STORED(field) \
       (field->vcol_info && field->vcol_info->is_stored())
#  elif (MYSQL_VERSION_ID >= 50500)
#    define MRN_GENERATED_COLUMNS_FIELD_IS_VIRTUAL(field) \
       (!field->stored_in_db)
#    define MRN_GENERATED_COLUMNS_FIELD_IS_STORED(field) \
       (field->vcol_info && field->vcol_info->is_stored())
#  else
#    define MRN_GENERATED_COLUMNS_FIELD_IS_VIRTUAL(field) false
#    define MRN_GENERATED_COLUMNS_FIELD_IS_STORED(field) false
#  endif
#else
#  if (MYSQL_VERSION_ID >= 50708)
#    define MRN_GENERATED_COLUMNS_FIELD_IS_VIRTUAL(field) \
       (field->is_virtual_gcol())
#    define MRN_GENERATED_COLUMNS_FIELD_IS_STORED(field) \
       (field->is_gcol() && !field->is_virtual_gcol())
#  elif (MYSQL_VERSION_ID >= 50706)
#    define MRN_GENERATED_COLUMNS_FIELD_IS_VIRTUAL(field) \
       (!field->stored_in_db)
#    define MRN_GENERATED_COLUMNS_FIELD_IS_STORED(field) \
       (field->gcol_info && field->gcol_info->get_field_stored())
#  else
#    define MRN_GENERATED_COLUMNS_FIELD_IS_VIRTUAL(field) false
#    define MRN_GENERATED_COLUMNS_FIELD_IS_STORED(field) false
#  endif
#endif

#ifdef MRN_MARIADB_P
#  if (MYSQL_VERSION_ID >= 100203)
#    define MRN_GENERATED_COLUMNS_UPDATE_VIRTUAL_FIELD(table, field) \
       (table->update_virtual_field(field))
#  else
#    define MRN_GENERATED_COLUMNS_UPDATE_VIRTUAL_FIELD(table, field) \
       (field->vcol_info->expr_item->save_in_field(field, 0))
#  endif
#endif

#ifdef MRN_PERCONA_P
#  if MYSQL_VERSION_ID < 80000 && \
  (((MYSQL_VERSION_ID >= 50634) && (MYSQL_VERSION_ID < 50700)) ||       \
   (MYSQL_VERSION_ID >= 50721))
#    define MRN_NEED_ROCKSDB_DB_TYPE_FIX
#  endif
#endif

#if defined(MRN_MARIADB_P) && MYSQL_VERSION_ID >= 100306
#  define MRN_TABLE_LIST_TABLE_NAME_DATA(table_list)    \
  (table_list)->table_name.str
#  define MRN_TABLE_LIST_TABLE_NAME_LENGTH(table_list)  \
  (table_list)->table_name.length
#else
#  define MRN_TABLE_LIST_TABLE_NAME_DATA(table_list)    \
  (table_list)->table_name
#  define MRN_TABLE_LIST_TABLE_NAME_LENGTH(table_list)  \
  (table_list)->table_name_length
#endif

#if !defined(MRN_MARIADB_P) && MYSQL_VERSION_ID >= 80024
#  define MRN_TABLE_LIST_QUERY_BLOCK(table_list)    \
   (table_list)->query_block
#else
#  define MRN_TABLE_LIST_QUERY_BLOCK(table_list)    \
   (table_list)->select_lex
#endif

#if !defined(MRN_MARIADB_P) && MYSQL_VERSION_ID >= 80018
#  define MRN_DECLARE_TABLE_LIST(variable_name,                         \
                                 db_name,                               \
                                 db_name_length,                        \
                                 table_name,                            \
                                 table_name_length,                     \
                                 alias,                                 \
                                 lock_type)                             \
    TABLE_LIST variable_name((db_name),                                 \
                             (db_name_length),                          \
                             (table_name),                              \
                             (table_name_length),                       \
                             (alias),                                   \
                             (lock_type))
#elif defined(MRN_MARIADB_P) && MYSQL_VERSION_ID >= 100306
#  define MRN_DECLARE_TABLE_LIST(variable_name,                         \
                                 db_name,                               \
                                 db_name_length,                        \
                                 table_name,                            \
                                 table_name_length,                     \
                                 alias,                                 \
                                 lock_type)                             \
    TABLE_LIST variable_name;                                           \
    do {                                                                \
      LEX_CSTRING db_name_ = {(db_name), (db_name_length)};             \
      LEX_CSTRING table_name_ = {(table_name), (table_name_length)};    \
      LEX_CSTRING alias_ = {(alias), strlen((alias))};                  \
      variable_name.init_one_table(&db_name_,                           \
                                   &table_name_,                        \
                                   &alias_,                             \
                                   (lock_type));                        \
    } while(false)
#else
#  define MRN_DECLARE_TABLE_LIST(variable_name,                 \
                                 db_name,                       \
                                 db_name_length,                \
                                 table_name,                    \
                                 table_name_length,             \
                                 alias,                         \
                                 lock_type)                     \
    TABLE_LIST variable_name;                                   \
    variable_name.init_one_table((db_name),                     \
                                 (db_name_length),              \
                                 (table_name),                  \
                                 (table_name_length),           \
                                 (alias),                       \
                                 (lock_type))
#endif

#if MYSQL_VERSION_ID >= 80011 && !defined(MRN_MARIADB_P)
  typedef Key_spec mrn_key_spec;
#  define MRN_KEY_SPEC_LIST_EACH_BEGIN(spec_list, spec) do {            \
    for (size_t spec_i = 0; spec_i < spec_list.size(); ++spec_i) {      \
      const mrn_key_spec *spec = spec_list[spec_i];
#  define MRN_KEY_SPEC_LIST_EACH_END()          \
    }                                           \
  } while (false)
#else
  typedef Key mrn_key_spec;
#  define MRN_KEY_SPEC_LIST_EACH_BEGIN(spec_list, spec) do {    \
    List<mrn_key_spec> spec_list_ = spec_list;                  \
    List_iterator<mrn_key_spec> spec_iterator(spec_list_);      \
    const mrn_key_spec *spec;                                   \
    while ((spec = spec_iterator++))
#  define MRN_KEY_SPEC_LIST_EACH_END()          \
  } while (false)
#endif

#if MYSQL_VERSION_ID >= 80011 && !defined(MRN_MARIADB_P)
#  define MRN_KEY_PART_SPEC_LIST_N_ELEMENTS(spec_list)      \
  spec_list.size()
#  define MRN_KEY_PART_SPEC_LIST_EACH_BEGIN(spec_list, spec) do {       \
    for (size_t spec_i = 0; spec_i < spec_list.size(); ++spec_i) {      \
      const Key_part_spec *spec = spec_list[spec_i];
#  define MRN_KEY_PART_SPEC_LIST_EACH_END()     \
    }                                           \
  } while (false)
#else
#  define MRN_KEY_PART_SPEC_LIST_N_ELEMENTS(spec_list)      \
  spec_list.elements
#  define MRN_KEY_PART_SPEC_LIST_EACH_BEGIN(spec_list, spec) do {       \
    List<Key_part_spec> spec_list_ = spec_list;                         \
    List_iterator<Key_part_spec> spec_iterator(spec_list_);             \
    const Key_part_spec *spec;                                          \
    while ((spec = spec_iterator++))
#  define MRN_KEY_PART_SPEC_LIST_EACH_END()     \
  } while (false)
#endif

#if MYSQL_VERSION_ID >= 80011 && !defined(MRN_MARIADB_P)
  typedef Foreign_key_spec mrn_foreign_key_spec;
#else
  typedef Foreign_key mrn_foreign_key_spec;
#endif

#if MYSQL_VERSION_ID >= 80024 && !defined(MRN_MARIADB_P)
  class Query_block;
  typedef Query_block mrn_query_block;
#elif MYSQL_VERSION_ID >= 80011 && !defined(MRN_MARIADB_P)
  class SELECT_LEX;
  typedef SELECT_LEX mrn_query_block;
#else
  typedef st_select_lex mrn_query_block;
#endif

#if MYSQL_VERSION_ID >= 80024 && !defined(MRN_MARIADB_P)
  typedef Query_expression mrn_query_expression;
#else
  typedef SELECT_LEX_UNIT mrn_query_expression;
#endif

#if MYSQL_VERSION_ID >= 80024 && !defined(MRN_MARIADB_P)
#  define MRN_QUERY_EXPRESSION_FIRST_QUERY_BLOCK(query_expression)  \
   (query_expression)->first_query_block()
#else
#  define MRN_QUERY_EXPRESSION_FIRST_QUERY_BLOCK(query_expression)  \
   (query_expression)->first_select()
#endif

#if MYSQL_VERSION_ID >= 80011 && !defined(MRN_MARIADB_P)
#  define MRN_FIELD_HAVE_AUTO_FLAGS
#endif

#if MYSQL_VERSION_ID >= 80011 && !defined(MRN_MARIADB_P)
#  define MRN_OPEN_TABLE_DEF_USE_TABLE_DEFINITION
#endif

#if MYSQL_VERSION_ID >= 100007 && defined(MRN_MARIADB_P)
#  define MRN_GET_ERR_MSG(code) my_get_err_msg(code)
#else
#  if MYSQL_VERSION_ID >= 80011 && !defined(MRN_MARIADB_P)
#    define MRN_GET_ERR_MSG(code) ER_DEFAULT(code)
#  else
#    define MRN_GET_ERR_MSG(code) ER(code)
#  endif
#endif

#if MYSQL_VERSION_ID >= 100504 && defined(MRN_MARIADB_P)
#  define mrn_thd_set_ha_data(thd, hton, ha_data) \
  thd_set_ha_data(thd, hton, ha_data)
#else
#  define mrn_thd_set_ha_data(thd, hton, ha_data) \
  *thd_ha_data(thd, hton) = ha_data
#endif

#if MYSQL_VERSION_ID >= 80011 && !defined(MRN_MARIADB_P)
#  define mrn_destroy(pointer) destroy(pointer)
#else
#  define mrn_destroy(pointer) delete pointer
#endif

#if MYSQL_VERSION_ID >= 80013 && !defined(MRN_MARIADB_P)
#  define mrn_alloc_table_share(table_list, key, key_length) \
  alloc_table_share((table_list)->db,                        \
                    (table_list)->table_name,                \
                    (key),                                   \
                    (key_length),                            \
                    false)
#elif MYSQL_VERSION_ID >= 100306 && defined(MRN_MARIADB_P)
#  define mrn_alloc_table_share(table_list, key, key_length) \
  alloc_table_share((table_list)->db.str,                    \
                    (table_list)->table_name.str,            \
                    (key),                                   \
                    (key_length))
#elif (MYSQL_VERSION_ID >= 100002 && defined(MRN_MARIADB_P)) || \
  (MYSQL_VERSION_ID >= 80011 && !defined(MRN_MARIADB_P))
#  define mrn_alloc_table_share(table_list, key, key_length)    \
  alloc_table_share((table_list)->db,                           \
                    (table_list)->table_name,                   \
                    (key),                                      \
                    (key_length))
#else
#  define mrn_alloc_table_share(table_list, key, key_length)    \
  alloc_table_share((table_list), (key), (key_length))
#endif

#if (MYSQL_VERSION_ID >= 80016 && !defined(MRN_MARIADB_P))
#  define mrn_TIME_to_longlong_datetime_packed(mysql_time)      \
  TIME_to_longlong_datetime_packed(mysql_time)
#else
#  define mrn_TIME_to_longlong_datetime_packed(mysql_time)      \
  TIME_to_longlong_datetime_packed(&mysql_time)
#endif

#if (MYSQL_VERSION_ID >= 80016 && !defined(MRN_MARIADB_P))
#  define MRN_STRING_APPEND append
#else
#  define MRN_STRING_APPEND q_append
#endif

#if (MYSQL_VERSION_ID >= 80017 && !defined(MRN_MARIADB_P))
  using my_ptrdiff_t = ptrdiff_t;
#endif

#if (MYSQL_VERSION_ID >= 80017 && !defined(MRN_MARIADB_P))
#  define MRN_MI_FLOAT8GET(result, data) (result) = mi_float8get((data))
#else
#  define MRN_MI_FLOAT8GET(result, data) mi_float8get((result), (data))
#endif

#if defined(MRN_MARIADB_P) && (MYSQL_VERSION_ID >= 100400)
#  define MRN_ITEM_IS_STRING_TYPE(item) \
  ((item)->is_of_type(Item::CONST_ITEM, STRING_RESULT))
#  define MRN_ITEM_IS_INT_TYPE(item) \
  ((item)->is_of_type(Item::CONST_ITEM, INT_RESULT))
#else
#  define MRN_ITEM_IS_STRING_TYPE(item) ((item)->type() == Item::STRING_ITEM)
#  define MRN_ITEM_IS_INT_TYPE(item) ((item)->type() == Item::INT_ITEM)
#endif

#if defined(MRN_MARIADB_P) && (MYSQL_VERSION_ID >= 100400)
#  define MRN_ITEM_GET_TIME(item, time, thd)                    \
  ((item)->get_date((thd), (time), Time::Options((thd))))
#  define MRN_ITEM_GET_DATE_FUZZY(item, time, thd)                      \
  ((item)->get_date((thd), (time), Time::Options(TIME_FUZZY_DATES, (thd))))
#else
#  define MRN_ITEM_GET_TIME(item, time, thd)    \
  ((item)->get_time((time)))
#  define MRN_ITEM_GET_DATE_FUZZY(item, time, thd)      \
  ((item)->get_date((time), TIME_FUZZY_DATE))
#endif

#if defined(MRN_MARIADB_P) && (MYSQL_VERSION_ID >= 100400)
#  define MRN_FIELD_GET_TIME(field, time, thd)  \
  ((field)->get_date((time), Time::Options((thd))))
#  define MRN_FIELD_GET_DATE_NO_FUZZY(field, time, thd)  \
  ((field)->get_date((time), Time::Options(TIME_CONV_NONE, (thd))))
#else
#  define MRN_FIELD_GET_TIME(field, time, thd)  \
  ((field)->get_time((time)))
#  define MRN_FIELD_GET_DATE_NO_FUZZY(field, time, thd)  \
  ((field)->get_date((time), 0))
#endif

#if MYSQL_VERSION_ID >= 80021 && !defined(MRN_MARIADB_P)
#  define MRN_FIELD_IS_UNSIGNED(field) \
  ((field)->is_unsigned())
#  define MRN_FIELD_FIELD_INDEX(field) \
  ((field)->field_index())
#  define MRN_FIELD_ALL_FLAGS(field) \
  ((field)->all_flags())
#  define MRN_FIELD_FIELD_PTR(field) \
  ((field)->field_ptr())
#  define MRN_FIELD_SET_FIELD_PTR(field, new_ptr) \
  ((field)->set_field_ptr((new_ptr)))
#else
#  define MRN_FIELD_IS_UNSIGNED(field) \
  ((field)->unsigned_flag)
#  define MRN_FIELD_FIELD_INDEX(field) \
  ((field)->field_index)
#  define MRN_FIELD_ALL_FLAGS(field) \
  ((field)->flags)
#  define MRN_FIELD_FIELD_PTR(field) \
  ((field)->ptr)
#  define MRN_FIELD_SET_FIELD_PTR(field, new_ptr) \
  (field)->ptr = (new_ptr)
#endif

#if defined(MRN_MARIADB_P) && (MYSQL_VERSION_ID >= 100400)
#  define MRN_TABLE_RESET(table) ((table)->reset())
#  define MRN_TABLE_SHARE_RESET(table_share) ((table_share)->reset())
#else
#  define MRN_TABLE_RESET(table) \
  (memset((table), 0, sizeof(TABLE)))
#  define MRN_TABLE_SHARE_RESET(table_share) \
  (memset((table_share), 0, sizeof(TABLE_SHARE)))
#endif

#if MYSQL_VERSION_ID >= 80019 && !defined(MRN_MARIADB_P)
#  define MRN_BITMAP_INIT(map, buf, n_bits) \
    bitmap_init((map), (buf), (n_bits))
#else
#  define MRN_BITMAP_INIT(map, buf, n_bits) \
    bitmap_init((map), (buf), (n_bits), false)
#endif

#if defined(MRN_MARIADB_P) && ((MYSQL_VERSION_ID >= 100237 && MYSQL_VERSION_ID < 100300) || \
                               (MYSQL_VERSION_ID >= 100328 && MYSQL_VERSION_ID < 100400) || \
                               (MYSQL_VERSION_ID >= 100418 && MYSQL_VERSION_ID < 100500) || \
                               (MYSQL_VERSION_ID >= 100509))
#  define MRN_DBUG_TMP_USE_BITMAP_PP
#endif

#if defined(MRN_MARIADB_P) && (MYSQL_VERSION_ID >= 100603)
#  define MRN_CHARSET_CSNAME(charset) \
  ((charset)->cs_name.str)
#  define MRN_CHARSET_NAME(charset) \
  ((charset)->coll_name.str)
#else
#  define MRN_CHARSET_CSNAME(charset) \
  ((charset)->csname)
#  define MRN_CHARSET_NAME(charset) \
  ((charset)->name)
#endif

#if MYSQL_VERSION_ID >= 100603 && defined(MRN_MARIADB_P)
#  define MRN_MATCH_ITEM_FLAGS(match_item) \
  ((match_item)->match_flags)
#else
#  define MRN_MATCH_ITEM_FLAGS(match_item) \
  ((match_item)->flags)
#endif

#if MYSQL_VERSION_ID >= 80027 && !defined(MRN_MARIADB_P)
#  define MRN_FREE_ROOT(root) (root)->Clear()
#else
#  define MRN_FREE_ROOT(root) free_root((root), MYF(0))
#endif

#if (MYSQL_VERSION_ID >= 80000 && !defined(MRN_MARIADB_P))
#  define MRN_LIST_SIZE(list) ((list)->size())
#else
#  define MRN_LIST_SIZE(list) ((list)->elements)
#endif
