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

#ifndef _ha_mroonga_hpp
#define _ha_mroonga_hpp

#ifdef USE_PRAGMA_INTERFACE
#pragma interface
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <groonga.h>
#include "mrn_sys.hpp"
#include "mrn_mysql_compat.h"

#if (MYSQL_VERSION_ID >= 50603) || \
    (MYSQL_VERSION_ID >= 50513 && MYSQL_VERSION_ID < 50600) || \
    (MYSQL_VERSION_ID >= 50158 && MYSQL_VERSION_ID < 50500)
#  define MRN_HANDLER_CLONE_NEED_NAME 1
#endif

#if (MYSQL_VERSION_ID >= 50603) || \
    (MYSQL_VERSION_ID >= 50514)
#  define MRN_HANDLER_HAVE_FINAL_ADD_INDEX 1
#endif

#if (MYSQL_VERSION_ID >= 50603) || \
    (defined(MRN_MARIADB_P) && MYSQL_VERSION_ID >= 50209)
#  define MRN_HANDLER_HAVE_HA_RND_NEXT 1
#  define MRN_HANDLER_HAVE_HA_RND_POS 1
#  define MRN_HANDLER_HAVE_HA_INDEX_READ_MAP 1
#  define MRN_HANDLER_HAVE_HA_INDEX_READ_IDX_MAP 1
#  define MRN_HANDLER_HAVE_HA_INDEX_NEXT 1
#  define MRN_HANDLER_HAVE_HA_INDEX_PREV 1
#  define MRN_HANDLER_HAVE_HA_INDEX_FIRST 1
#  define MRN_HANDLER_HAVE_HA_INDEX_LAST 1
#  define MRN_HANDLER_HAVE_HA_INDEX_NEXT_SAME 1
#endif

#if (MYSQL_VERSION_ID >= 50604) || \
    (defined(MRN_MARIADB_P) && MYSQL_VERSION_ID >= 50302)
#  define MRN_HANDLER_HAVE_HA_CLOSE 1
#  define MRN_HANDLER_HAVE_MULTI_RANGE_READ 1
#endif

#ifndef MRN_MARIADB_P
#  define MRN_HANDLER_HAVE_INDEX_READ_LAST_MAP
#endif

#if (defined(MRN_MARIADB_P) && MYSQL_VERSION_ID >= 50302)
#  define MRN_HANDLER_HAVE_MULTI_RANGE_READ_INFO_KEY_PARTS
#endif

#if MYSQL_VERSION_ID >= 50500
#  define MRN_HANDLER_HAVE_TRUNCATE
#  define MRN_HANDLER_HAVE_GET_PARENT_FOREIGN_KEY_LIST
#endif

#if (MYSQL_VERSION_ID < 50600)
#  define MRN_HANDLER_HAVE_GET_TABLESPACE_NAME 1
#endif

#if MYSQL_VERSION_ID >= 50500
#  define MRN_TABLE_LIST_INIT_REQUIRE_ALIAS
#endif

#ifdef BIG_TABLES
#  define MRN_HA_ROWS_FORMAT "llu"
#else
#  define MRN_HA_ROWS_FORMAT "lu"
#endif

#if MYSQL_VERSION_ID >= 50604
#  define MRN_HAVE_MYSQL_TYPE_TIMESTAMP2
#  define MRN_HAVE_MYSQL_TYPE_DATETIME2
#  define MRN_HAVE_MYSQL_TYPE_TIME2
#endif

#if (MYSQL_VERSION_ID < 50519) || \
    defined(MRN_MARIADB_P) || \
    (50600 <= MYSQL_VERSION_ID && MYSQL_VERSION_ID < 50604)
#  define MRN_NEED_FREE_STRING_MEMALLOC_PLUGIN_VAR
#endif

#if MYSQL_VERSION_ID >= 50500
#  define MRN_HAVE_HA_EXTRA_ADD_CHILDREN_LIST
#  define MRN_HAVE_HA_EXTRA_IS_ATTACHED_CHILDREN
#endif

#ifdef MRN_MARIADB_P
#  define MRN_HAVE_HA_EXTRA_DETACH_CHILD
#  define MRN_HAVE_HA_EXTRA_PREPARE_FOR_FORCED_CLOSE
#endif

#if MYSQL_VERSION_ID >= 50604
#  define MRN_TIMESTAMP_USE_TIMEVAL
#elif defined(MRN_MARIADB_P)
#  define MRN_TIMESTAMP_USE_MY_TIME_T
#else
#  define MRN_TIMESTAMP_USE_LONG
#endif

#if MYSQL_VERSION_ID < 50600 && !defined(MRN_MARIADB_P)
#  define MRN_FIELD_STORE_TIME_NEED_TYPE
#endif

#if MYSQL_VERSION_ID < 50500
#  define MRN_HAVE_TL_WRITE_ALLOW_READ
#endif

#if (defined(MRN_MARIADB_P) && \
     ((MYSQL_VERSION_ID >= 50306 && MYSQL_VERSION_ID < 50500) || \
      MYSQL_VERSION_ID >= 50523))
#  define MRN_HANDLER_AUTO_REPAIR_HAVE_ERROR
#endif

class ha_mroonga;

/* structs */
struct st_mrn_ft_info
{
  struct _ft_vft *please;
  grn_ctx *ctx;
  grn_obj *table;
  grn_obj *result;
  grn_obj *score_column;
  grn_obj key;
  grn_obj score;
  uint active_index;
  KEY *key_info;
  KEY *primary_key_info;
  ha_mroonga *mroonga;
};

/* handler class */
class ha_mroonga: public handler
{
public:
  handler   *wrap_handler;
  bool      is_clone;
  ha_mroonga *parent_for_clone;
  MEM_ROOT  *mem_root_for_clone;
  grn_obj   key_buffer;
  grn_id    record_id;
  grn_id    *key_id;
  grn_id    *del_key_id;
  MY_BITMAP multiple_column_key_bitmap;

private:
  THR_LOCK_DATA thr_lock_data;

  // for wrapper mode (TODO: need to be confirmed)
  MRN_SHARE *share;
  KEY       *wrap_key_info;
  KEY       *base_key_info;
  key_part_map pk_keypart_map;
  MEM_ROOT  mem_root;
  /// for create table and alter table
  mutable bool        analyzed_for_create;
  mutable TABLE       table_for_create;
  mutable MRN_SHARE   share_for_create;
  mutable TABLE_SHARE table_share_for_create;
  mutable MEM_ROOT    mem_root_for_create;
  mutable handler     *wrap_handler_for_create;
#ifdef MRN_HANDLER_HAVE_FINAL_ADD_INDEX
  handler_add_index *hnd_add_index;
#endif
  int mrn_lock_type;

  // for groonga objects
  grn_ctx ctx_entity_;
  grn_ctx *ctx;
  grn_obj *grn_table;
  grn_obj **grn_columns;
  grn_obj **grn_index_tables;
  grn_obj **grn_index_columns;

  // buffers
  grn_obj  encoded_key_buffer;
  grn_obj  old_value_buffer;
  grn_obj  new_value_buffer;
  grn_obj top_left_point;
  grn_obj bottom_right_point;
  grn_obj source_point;
  double top_left_longitude_in_degree;
  double bottom_right_longitude_in_degree;
  double bottom_right_latitude_in_degree;
  double top_left_latitude_in_degree;

  // for search
  grn_obj *grn_source_column_geo;
  grn_obj *cursor_geo;
  grn_table_cursor *cursor;
  grn_table_cursor *index_table_cursor;
  grn_obj *id_accessor;
  grn_obj *key_accessor;
  grn_obj *sorted_result;
  grn_obj *matched_record_keys;

  // for error report
  uint dup_key;

  // for optimization
  bool count_skip;
  bool fast_order_limit;
  bool fast_order_limit_with_index;

  // for context
  bool ignoring_duplicated_key;
  bool inserting_with_update;
  bool fulltext_searching;
  bool ignoring_no_key_columns;
  bool replacing_;

public:
  ha_mroonga(handlerton *hton, TABLE_SHARE *share_arg);
  ~ha_mroonga();
  const char *table_type() const;           // required
  const char *index_type(uint inx);
  const char **bas_ext() const;                                    // required

  ulonglong table_flags() const;                                   // required
  ulong index_flags(uint idx, uint part, bool all_parts) const;    // required

  int create(const char *name, TABLE *form, HA_CREATE_INFO *info); // required
  int open(const char *name, int mode, uint test_if_locked);       // required
#ifndef MRN_HANDLER_HAVE_HA_CLOSE
  int close();                                                     // required
#endif
  int info(uint flag);                                             // required

  uint lock_count() const;
  THR_LOCK_DATA **store_lock(THD *thd,                             // required
                             THR_LOCK_DATA **to,
                             enum thr_lock_type lock_type);
  int external_lock(THD *thd, int lock_type);

  int rnd_init(bool scan);                                         // required
  int rnd_end();
#ifndef MRN_HANDLER_HAVE_HA_RND_NEXT
  int rnd_next(uchar *buf);                                        // required
#endif
#ifndef MRN_HANDLER_HAVE_HA_RND_POS
  int rnd_pos(uchar *buf, uchar *pos);                             // required
#endif
  void position(const uchar *record);                              // required
  int extra(enum ha_extra_function operation);
  int extra_opt(enum ha_extra_function operation, ulong cache_size);

  int delete_table(const char *name);
  int write_row(uchar *buf);
  int update_row(const uchar *old_data, uchar *new_data);
  int delete_row(const uchar *buf);

  uint max_supported_record_length()   const;
  uint max_supported_keys()            const;
  uint max_supported_key_parts();
  uint max_supported_key_length()      const;
  uint max_supported_key_part_length() const;

  ha_rows records_in_range(uint inx, key_range *min_key, key_range *max_key);
  int index_init(uint idx, bool sorted);
  int index_end();
#ifndef MRN_HANDLER_HAVE_HA_INDEX_READ_MAP
  int index_read_map(uchar * buf, const uchar * key,
                     key_part_map keypart_map,
                     enum ha_rkey_function find_flag);
#endif
#ifdef MRN_HANDLER_HAVE_INDEX_READ_LAST_MAP
  int index_read_last_map(uchar *buf, const uchar *key,
                          key_part_map keypart_map);
#endif
#ifndef MRN_HANDLER_HAVE_HA_INDEX_NEXT
  int index_next(uchar *buf);
#endif
#ifndef MRN_HANDLER_HAVE_HA_INDEX_PREV
  int index_prev(uchar *buf);
#endif
#ifndef MRN_HANDLER_HAVE_HA_INDEX_FIRST
  int index_first(uchar *buf);
#endif
#ifndef MRN_HANDLER_HAVE_HA_INDEX_LAST
  int index_last(uchar *buf);
#endif
  int index_next_same(uchar *buf, const uchar *key, uint keylen);

  int read_range_first(const key_range *start_key,
                       const key_range *end_key,
                       bool eq_range, bool sorted);
  int read_range_next();

  int ft_init();
  void ft_end();
  FT_INFO *ft_init_ext(uint flags, uint inx, String *key);
  int ft_read(uchar *buf);

  const Item *cond_push(const Item *cond);
  void cond_pop();

  bool get_error_message(int error, String *buf);

  int reset();

#ifdef MRN_HANDLER_CLONE_NEED_NAME
  handler *clone(const char *name, MEM_ROOT *mem_root);
#else
  handler *clone(MEM_ROOT *mem_root);
#endif
  uint8 table_cache_type();
#ifdef MRN_HANDLER_HAVE_MULTI_RANGE_READ
  ha_rows multi_range_read_info_const(uint keyno, RANGE_SEQ_IF *seq,
                                      void *seq_init_param,
                                      uint n_ranges, uint *bufsz,
                                      uint *flags, Cost_estimate *cost);
  ha_rows multi_range_read_info(uint keyno, uint n_ranges, uint keys,
#ifdef MRN_HANDLER_HAVE_MULTI_RANGE_READ_INFO_KEY_PARTS
                                uint key_parts,
#endif
                                uint *bufsz, uint *flags, Cost_estimate *cost);
  int multi_range_read_init(RANGE_SEQ_IF *seq, void *seq_init_param,
                            uint n_ranges, uint mode,
                            HANDLER_BUFFER *buf);
  int multi_range_read_next(range_id_t *range_info);
#else // MRN_HANDLER_HAVE_MULTI_RANGE_READ
  int read_multi_range_first(KEY_MULTI_RANGE **found_range_p,
                             KEY_MULTI_RANGE *ranges,
                             uint range_count,
                             bool sorted,
                             HANDLER_BUFFER *buffer);
  int read_multi_range_next(KEY_MULTI_RANGE **found_range_p);
#endif // MRN_HANDLER_HAVE_MULTI_RANGE_READ
  void start_bulk_insert(ha_rows rows);
  int end_bulk_insert();
  int delete_all_rows();
#ifdef MRN_HANDLER_HAVE_TRUNCATE
  int truncate();
#endif // MRN_HANDLER_HAVE_TRUNCATE
  double scan_time();
  double read_time(uint index, uint ranges, ha_rows rows);
  const key_map *keys_to_use_for_scanning();
  ha_rows estimate_rows_upper_bound();
  void update_create_info(HA_CREATE_INFO* create_info);
  int rename_table(const char *from, const char *to);
  bool is_crashed() const;
  bool auto_repair(int error) const;
  bool auto_repair() const;
  int disable_indexes(uint mode);
  int enable_indexes(uint mode);
  int check(THD* thd, HA_CHECK_OPT* check_opt);
  int repair(THD* thd, HA_CHECK_OPT* check_opt);
  bool check_and_repair(THD *thd);
  int analyze(THD* thd, HA_CHECK_OPT* check_opt);
  int optimize(THD* thd, HA_CHECK_OPT* check_opt);
  bool is_fatal_error(int error_num, uint flags);
  bool check_if_incompatible_data(HA_CREATE_INFO *create_info,
                                  uint table_changes);
  uint alter_table_flags(uint flags);
#ifdef MRN_HANDLER_HAVE_FINAL_ADD_INDEX
  int add_index(TABLE *table_arg, KEY *key_info, uint num_of_keys,
                handler_add_index **add);
  int final_add_index(handler_add_index *add, bool commit);
#else
  int add_index(TABLE *table_arg, KEY *key_info, uint num_of_keys);
#endif
  int prepare_drop_index(TABLE *table_arg, uint *key_num, uint num_of_keys);
  int final_drop_index(TABLE *table_arg);
  int update_auto_increment();
  void set_next_insert_id(ulonglong id);
  void get_auto_increment(ulonglong offset, ulonglong increment, ulonglong nb_desired_values,
                          ulonglong *first_value, ulonglong *nb_reserved_values);
  void restore_auto_increment(ulonglong prev_insert_id);
  void release_auto_increment();
  int check_for_upgrade(HA_CHECK_OPT *check_opt);
  int reset_auto_increment(ulonglong value);
  bool was_semi_consistent_read();
  void try_semi_consistent_read(bool yes);
  void unlock_row();
  int start_stmt(THD *thd, thr_lock_type lock_type);

protected:
#ifdef MRN_HANDLER_HAVE_HA_RND_NEXT
  int rnd_next(uchar *buf);
#endif
#ifdef MRN_HANDLER_HAVE_HA_RND_POS
  int rnd_pos(uchar *buf, uchar *pos);
#endif
#ifdef MRN_HANDLER_HAVE_HA_INDEX_READ_MAP
  int index_read_map(uchar *buf, const uchar *key,
                     key_part_map keypart_map,
                     enum ha_rkey_function find_flag);
#endif
#ifdef MRN_HANDLER_HAVE_HA_INDEX_NEXT
  int index_next(uchar *buf);
#endif
#ifdef MRN_HANDLER_HAVE_HA_INDEX_PREV
  int index_prev(uchar *buf);
#endif
#ifdef MRN_HANDLER_HAVE_HA_INDEX_FIRST
  int index_first(uchar *buf);
#endif
#ifdef MRN_HANDLER_HAVE_HA_INDEX_LAST
  int index_last(uchar *buf);
#endif
  void change_table_ptr(TABLE *table_arg, TABLE_SHARE *share_arg);
  bool primary_key_is_clustered();
  bool is_fk_defined_on_table_or_index(uint index);
  char *get_foreign_key_create_info();
#ifdef MRN_HANDLER_HAVE_GET_TABLESPACE_NAME
  char *get_tablespace_name(THD *thd, char *name, uint name_len);
#endif
  bool can_switch_engines();
  int get_foreign_key_list(THD *thd, List<FOREIGN_KEY_INFO> *f_key_list);
#ifdef MRN_HANDLER_HAVE_GET_PARENT_FOREIGN_KEY_LIST
  int get_parent_foreign_key_list(THD *thd, List<FOREIGN_KEY_INFO> *f_key_list);
#endif
  uint referenced_by_foreign_key();
  void init_table_handle_for_HANDLER();
  void free_foreign_key_create_info(char* str);

private:
  void push_warning_unsupported_spatial_index_search(enum ha_rkey_function flag);
  void clear_cursor();
  void clear_cursor_geo();
  void clear_search_result();
  void clear_search_result_geo();
  void clear_indexes();
  int alter_share_add(const char *path, TABLE_SHARE *table_share);
  void remove_grn_obj_force(const char *name);
  grn_obj *find_tokenizer(const char *name, int name_length);
  int wrapper_get_next_record(uchar *buf);
  int storage_get_next_record(uchar *buf);
  void geo_store_rectangle(const uchar *rectangle);
  int generic_geo_open_cursor(const uchar *key, enum ha_rkey_function find_flag);

#ifdef MRN_HANDLER_HAVE_HA_CLOSE
  int close();
#endif
  bool is_dry_write();
  bool is_enable_optimization();
  bool is_need_normalize(Field *field) const;
  bool is_temporary_table_name(const char *name) const;
  void check_count_skip(key_part_map start_key_part_map,
                        key_part_map end_key_part_map, bool fulltext);
  bool is_groonga_layer_condition(const Item *item,
                                  const Item_func **match_against=NULL);
  bool is_fulltext_search_item(const Item *item);
  bool is_grn_zero_column_value(grn_obj *column, grn_obj *value);
  void check_fast_order_limit(grn_table_sort_key **sort_keys, int *n_sort_keys,
                              longlong *limit);

  long long int get_grn_time_from_timestamp_field(Field_timestamp *field);

  int generic_store_bulk_string(Field *field, grn_obj *buf);
  int generic_store_bulk_integer(Field *field, grn_obj *buf);
  int generic_store_bulk_float(Field *field, grn_obj *buf);
  int generic_store_bulk_timestamp(Field *field, grn_obj *buf);
  int generic_store_bulk_date(Field *field, grn_obj *buf);
  int generic_store_bulk_time(Field *field, grn_obj *buf);
  int generic_store_bulk_datetime(Field *field, grn_obj *buf);
  int generic_store_bulk_year(Field *field, grn_obj *buf);
  int generic_store_bulk_new_date(Field *field, grn_obj *buf);
#ifdef MRN_HAVE_MYSQL_TYPE_DATETIME2
  int generic_store_bulk_datetime2(Field *field, grn_obj *buf);
#endif
#ifdef MRN_HAVE_MYSQL_TYPE_TIME2
  int generic_store_bulk_time2(Field *field, grn_obj *buf);
#endif
  int generic_store_bulk_new_decimal(Field *field, grn_obj *buf);
  int generic_store_bulk_blob(Field *field, grn_obj *buf);
  int generic_store_bulk_geometry(Field *field, grn_obj *buf);
  int generic_store_bulk(Field *field, grn_obj *buf);

  void storage_store_field_string(Field *field,
                                  const char *value, uint value_length);
  void storage_store_field_integer(Field *field,
                                   const char *value, uint value_length);
  void storage_store_field_float(Field *field,
                                 const char *value, uint value_length);
  void storage_store_field_timestamp(Field *field,
                                     const char *value, uint value_length);
  void storage_store_field_date(Field *field,
                                const char *value, uint value_length);
  void storage_store_field_time(Field *field,
                                const char *value, uint value_length);
  void storage_store_field_datetime(Field *field,
                                    const char *value, uint value_length);
  void storage_store_field_year(Field *field,
                                const char *value, uint value_length);
  void storage_store_field_new_date(Field *field,
                                    const char *value, uint value_length);
#ifdef MRN_HAVE_MYSQL_TYPE_DATETIME2
  void storage_store_field_datetime2(Field *field,
                                     const char *value, uint value_length);
#endif
#ifdef MRN_HAVE_MYSQL_TYPE_TIME2
  void storage_store_field_time2(Field *field,
                                 const char *value, uint value_length);
#endif
  void storage_store_field_blob(Field *field,
                                const char *value, uint value_length);
  void storage_store_field_geometry(Field *field,
                                    const char *value, uint value_length);
  void storage_store_field(Field *field, const char *value, uint value_length);
  void storage_store_fields(uchar *buf, grn_id record_id);
  void storage_store_fields_for_prep_update(const uchar *old_data,
                                            uchar *new_data,
                                            grn_id record_id);
  void storage_store_fields_by_index(uchar *buf);

  int storage_encode_key_time(Field *field, const uchar *key,
                              uchar *buf, uint *size);
  int storage_encode_key_year(Field *field, const uchar *key,
                              uchar *buf, uint *size);
  int storage_encode_key_datetime(Field *field, const uchar *key,
                                  uchar *buf, uint *size);
#ifdef MRN_HAVE_MYSQL_TYPE_TIME2
  int storage_encode_key_time2(Field *field, const uchar *key,
                               uchar *buf, uint *size);
#endif
  int storage_encode_key_enum(Field *field, const uchar *key,
                              uchar *buf, uint *size);
  int storage_encode_key(Field *field, const uchar *key, uchar *buf, uint *size);
  void storage_encode_multiple_column_key_float(float value,
                                                uint data_size,
                                                uchar *buffer,
                                                bool decode);
  void storage_encode_multiple_column_key_double(double value,
                                                 uint data_size,
                                                 uchar *buffer,
                                                 bool decode);
  int storage_encode_multiple_column_key(KEY *key_info,
                                         const uchar *key, uint key_length,
                                         uchar *buffer, uint *encoded_length,
                                         bool decode);

  void set_pk_bitmap();
  int create_share_for_create() const;
  int wrapper_create(const char *name, TABLE *table,
                     HA_CREATE_INFO *info, MRN_SHARE *tmp_share);
  int storage_create(const char *name, TABLE *table,
                     HA_CREATE_INFO *info, MRN_SHARE *tmp_share);
  int wrapper_create_index_fulltext_validate(KEY *key_info);
  int wrapper_create_index_fulltext(const char *grn_table_name,
                                    int i,
                                    KEY *key_info,
                                    grn_obj **index_tables,
                                    MRN_SHARE *tmp_share);
  int wrapper_create_index_geo(const char *grn_table_name,
                               int i,
                               KEY *key_info,
                               grn_obj **index_tables,
                               MRN_SHARE *tmp_share);
  int wrapper_create_index(const char *name, TABLE *table,
                           HA_CREATE_INFO *info, MRN_SHARE *tmp_share,
                           const char *grn_table_name);
  int storage_create_validate_pseudo_column(TABLE *table);
  int storage_create_validate_index(TABLE *table);
  int storage_create_index(TABLE *table, const char *grn_table_name,
                           grn_obj *grn_table, MRN_SHARE *tmp_share,
                           KEY *key_info, grn_obj **index_tables,
                           grn_obj **index_columns, uint i);
  int storage_create_indexes(TABLE *table, const char *grn_table_name,
                             grn_obj *grn_table, MRN_SHARE *tmp_share);
  int close_databases();
  int ensure_database_create(const char *name);
  int ensure_database_open(const char *name);
  int wrapper_delete_table(const char *name, MRN_SHARE *tmp_share,
                           const char *table_name);
  int wrapper_delete_index(const char *name, MRN_SHARE *tmp_share,
                           const char *table_name);
  int storage_delete_table(const char *name, MRN_SHARE *tmp_share,
                           const char *table_name);
  int wrapper_open(const char *name, int mode, uint test_if_locked);
  int wrapper_open_indexes(const char *name, bool ignore_open_error);
  int storage_open(const char *name, int mode, uint test_if_locked);
  int open_table(const char *name);
  int storage_open_columns(void);
  int storage_open_indexes(const char *name);
  void wrapper_overwrite_index_bits();
  int wrapper_close();
  int storage_close();
  int generic_extra(enum ha_extra_function operation);
  int wrapper_extra(enum ha_extra_function operation);
  int storage_extra(enum ha_extra_function operation);
  int wrapper_extra_opt(enum ha_extra_function operation, ulong cache_size);
  int storage_extra_opt(enum ha_extra_function operation, ulong cache_size);
  int wrapper_reset();
  int storage_reset();
  uint wrapper_lock_count() const;
  uint storage_lock_count() const;
  THR_LOCK_DATA **wrapper_store_lock(THD *thd, THR_LOCK_DATA **to,
                                     enum thr_lock_type lock_type);
  THR_LOCK_DATA **storage_store_lock(THD *thd, THR_LOCK_DATA **to,
                                     enum thr_lock_type lock_type);
  int wrapper_external_lock(THD *thd, int lock_type);
  int storage_external_lock(THD *thd, int lock_type);
  void wrapper_start_bulk_insert(ha_rows rows);
  void storage_start_bulk_insert(ha_rows rows);
  int wrapper_end_bulk_insert();
  int storage_end_bulk_insert();
  bool wrapper_is_target_index(KEY *key_info);
  bool wrapper_have_target_index();
  int wrapper_write_row(uchar *buf);
  int wrapper_write_row_index(uchar *buf);
  int storage_write_row(uchar *buf);
  int storage_write_row_multiple_column_index(uchar *buf,
                                              grn_id record_id,
                                              KEY *key_info,
                                              grn_obj *index_column);
  int storage_write_row_multiple_column_indexes(uchar *buf, grn_id record_id);
  int storage_write_row_unique_index(uchar *buf, grn_id record_id,
                                     KEY *key_info, grn_obj *index_table,
                                     grn_id *key_id);
  int storage_write_row_unique_indexes(uchar *buf, grn_id record_id);
  int wrapper_get_record_id(uchar *data, grn_id *record_id, const char *context);
  int wrapper_update_row(const uchar *old_data, uchar *new_data);
  int wrapper_update_row_index(const uchar *old_data, uchar *new_data);
  int storage_update_row(const uchar *old_data, uchar *new_data);
  int storage_update_row_index(const uchar *old_data, uchar *new_data);
  int storage_update_row_unique_indexes(uchar *new_data, grn_id record_id);
  int wrapper_delete_row(const uchar *buf);
  int wrapper_delete_row_index(const uchar *buf);
  int storage_delete_row(const uchar *buf);
  int storage_delete_row_index(const uchar *buf);
  int storage_delete_row_unique_index(grn_obj *index_table, grn_id del_key_id);
  int storage_delete_row_unique_indexes();
  int storage_prepare_delete_row_unique_index(const uchar *buf,
                                              grn_id record_id,
                                              KEY *key_info,
                                              grn_obj *index_table,
                                              grn_obj *index_column,
                                              grn_id *del_key_id);
  int storage_prepare_delete_row_unique_indexes(const uchar *buf,
                                                grn_id record_id);
  uint wrapper_max_supported_record_length() const;
  uint storage_max_supported_record_length() const;
  uint wrapper_max_supported_keys() const;
  uint storage_max_supported_keys() const;
  uint wrapper_max_supported_key_parts();
  uint storage_max_supported_key_parts();
  uint wrapper_max_supported_key_length() const;
  uint storage_max_supported_key_length() const;
  uint wrapper_max_supported_key_part_length() const;
  uint storage_max_supported_key_part_length() const;
  ulonglong wrapper_table_flags() const;
  ulonglong storage_table_flags() const;
  ulong wrapper_index_flags(uint idx, uint part, bool all_parts) const;
  ulong storage_index_flags(uint idx, uint part, bool all_parts) const;
  int wrapper_info(uint flag);
  int storage_info(uint flag);
  int wrapper_rnd_init(bool scan);
  int storage_rnd_init(bool scan);
  int wrapper_rnd_end();
  int storage_rnd_end();
  int wrapper_rnd_next(uchar *buf);
  int storage_rnd_next(uchar *buf);
  int wrapper_rnd_pos(uchar *buf, uchar *pos);
  int storage_rnd_pos(uchar *buf, uchar *pos);
  void wrapper_position(const uchar *record);
  void storage_position(const uchar *record);
  ha_rows wrapper_records_in_range(uint key_nr, key_range *range_min,
                                   key_range *range_max);
  ha_rows storage_records_in_range(uint key_nr, key_range *range_min,
                                   key_range *range_max);
  ha_rows generic_records_in_range_geo(uint key_nr, key_range *range_min,
                                       key_range *range_max);
  int wrapper_index_init(uint idx, bool sorted);
  int storage_index_init(uint idx, bool sorted);
  int wrapper_index_end();
  int storage_index_end();
  int wrapper_index_read_map(uchar *buf, const uchar *key,
                             key_part_map keypart_map,
                             enum ha_rkey_function find_flag);
  int storage_index_read_map(uchar *buf, const uchar *key,
                             key_part_map keypart_map,
                             enum ha_rkey_function find_flag);
#ifdef MRN_HANDLER_HAVE_INDEX_READ_LAST_MAP
  int wrapper_index_read_last_map(uchar *buf, const uchar *key,
                                  key_part_map keypart_map);
  int storage_index_read_last_map(uchar *buf, const uchar *key,
                                  key_part_map keypart_map);
#endif
  int wrapper_index_next(uchar *buf);
  int storage_index_next(uchar *buf);
  int wrapper_index_prev(uchar *buf);
  int storage_index_prev(uchar *buf);
  int wrapper_index_first(uchar *buf);
  int storage_index_first(uchar *buf);
  int wrapper_index_last(uchar *buf);
  int storage_index_last(uchar *buf);
  int wrapper_index_next_same(uchar *buf, const uchar *key, uint keylen);
  int storage_index_next_same(uchar *buf, const uchar *key, uint keylen);
  int wrapper_read_range_first(const key_range *start_key,
                               const key_range *end_key,
                               bool eq_range, bool sorted);
  int storage_read_range_first(const key_range *start_key,
                               const key_range *end_key,
                               bool eq_range, bool sorted);
  int wrapper_read_range_next();
  int storage_read_range_next();
  int generic_ft_init();
  int wrapper_ft_init();
  int storage_ft_init();
  void wrapper_ft_end();
  void storage_ft_end();
  FT_INFO *wrapper_ft_init_ext(uint flags, uint key_nr, String *key);
  FT_INFO *storage_ft_init_ext(uint flags, uint key_nr, String *key);
  void generic_ft_init_ext_add_conditions_fast_order_limit(
      struct st_mrn_ft_info *info, grn_obj *expression);
  struct st_mrn_ft_info *generic_ft_init_ext_select(uint flags,
                                                    uint key_nr,
                                                    String *key);
  FT_INFO *generic_ft_init_ext(uint flags, uint key_nr, String *key);
  int wrapper_ft_read(uchar *buf);
  int storage_ft_read(uchar *buf);
  const Item *wrapper_cond_push(const Item *cond);
  const Item *storage_cond_push(const Item *cond);
  void wrapper_cond_pop();
  void storage_cond_pop();
  bool wrapper_get_error_message(int error, String *buf);
  bool storage_get_error_message(int error, String *buf);
#ifdef MRN_HANDLER_CLONE_NEED_NAME
  handler *wrapper_clone(const char *name, MEM_ROOT *mem_root);
  handler *storage_clone(const char *name, MEM_ROOT *mem_root);
#else
  handler *wrapper_clone(MEM_ROOT *mem_root);
  handler *storage_clone(MEM_ROOT *mem_root);
#endif
  uint8 wrapper_table_cache_type();
  uint8 storage_table_cache_type();
#ifdef MRN_HANDLER_HAVE_MULTI_RANGE_READ
  ha_rows wrapper_multi_range_read_info_const(uint keyno,
                                              RANGE_SEQ_IF *seq,
                                              void *seq_init_param,
                                              uint n_ranges,
                                              uint *bufsz,
                                              uint *flags,
                                              Cost_estimate *cost);
  ha_rows storage_multi_range_read_info_const(uint keyno,
                                              RANGE_SEQ_IF *seq,
                                              void *seq_init_param,
                                              uint n_ranges,
                                              uint *bufsz,
                                              uint *flags,
                                              Cost_estimate *cost);
  ha_rows wrapper_multi_range_read_info(uint keyno, uint n_ranges, uint keys,
#ifdef MRN_HANDLER_HAVE_MULTI_RANGE_READ_INFO_KEY_PARTS
                                        uint key_parts,
#endif
                                        uint *bufsz, uint *flags,
                                        Cost_estimate *cost);
  ha_rows storage_multi_range_read_info(uint keyno, uint n_ranges, uint keys,
#ifdef MRN_HANDLER_HAVE_MULTI_RANGE_READ_INFO_KEY_PARTS
                                        uint key_parts,
#endif
                                        uint *bufsz, uint *flags,
                                        Cost_estimate *cost);
  int wrapper_multi_range_read_init(RANGE_SEQ_IF *seq, void *seq_init_param,
                                    uint n_ranges, uint mode,
                                    HANDLER_BUFFER *buf);
  int storage_multi_range_read_init(RANGE_SEQ_IF *seq, void *seq_init_param,
                                    uint n_ranges, uint mode,
                                    HANDLER_BUFFER *buf);
  int wrapper_multi_range_read_next(range_id_t *range_info);
  int storage_multi_range_read_next(range_id_t *range_info);
#else // MRN_HANDLER_HAVE_MULTI_RANGE_READ
  int wrapper_read_multi_range_first(KEY_MULTI_RANGE **found_range_p,
                                     KEY_MULTI_RANGE *ranges,
                                     uint range_count,
                                     bool sorted,
                                     HANDLER_BUFFER *buffer);
  int storage_read_multi_range_first(KEY_MULTI_RANGE **found_range_p,
                                     KEY_MULTI_RANGE *ranges,
                                     uint range_count,
                                     bool sorted,
                                     HANDLER_BUFFER *buffer);
  int wrapper_read_multi_range_next(KEY_MULTI_RANGE **found_range_p);
  int storage_read_multi_range_next(KEY_MULTI_RANGE **found_range_p);
#endif // MRN_HANDLER_HAVE_MULTI_RANGE_READ
  int wrapper_delete_all_rows();
  int storage_delete_all_rows();
#ifdef MRN_HANDLER_HAVE_TRUNCATE
  int wrapper_truncate();
#endif // MRN_HANDLER_HAVE_TRUNCATE
  int wrapper_truncate_index();
  int storage_truncate();
  int storage_truncate_index();
  double wrapper_scan_time();
  double storage_scan_time();
  double wrapper_read_time(uint index, uint ranges, ha_rows rows);
  double storage_read_time(uint index, uint ranges, ha_rows rows);
  const key_map *wrapper_keys_to_use_for_scanning();
  const key_map *storage_keys_to_use_for_scanning();
  ha_rows wrapper_estimate_rows_upper_bound();
  ha_rows storage_estimate_rows_upper_bound();
  void wrapper_update_create_info(HA_CREATE_INFO* create_info);
  void storage_update_create_info(HA_CREATE_INFO* create_info);
  int wrapper_rename_table(const char *from, const char *to,
                           MRN_SHARE *tmp_share,
                           const char *from_table_name,
                           const char *to_table_name);
  int wrapper_rename_index(const char *from, const char *to,
                           MRN_SHARE *tmp_share,
                           const char *from_table_name,
                           const char *to_table_name);
  int storage_rename_table(const char *from, const char *to,
                           MRN_SHARE *tmp_share,
                           const char *from_table_name,
                           const char *to_table_name);
  bool wrapper_is_crashed() const;
  bool storage_is_crashed() const;
  bool wrapper_auto_repair(int error) const;
  bool storage_auto_repair(int error) const;
  int wrapper_disable_indexes(uint mode);
  int storage_disable_indexes(uint mode);
  int wrapper_enable_indexes(uint mode);
  int storage_enable_indexes(uint mode);
  int wrapper_check(THD* thd, HA_CHECK_OPT* check_opt);
  int storage_check(THD* thd, HA_CHECK_OPT* check_opt);
  int wrapper_fill_indexes(THD *thd, KEY *key_info,
                           grn_obj **index_columns, uint n_keys);
  int wrapper_recreate_indexes(THD *thd);
  int wrapper_repair(THD* thd, HA_CHECK_OPT* check_opt);
  int storage_repair(THD* thd, HA_CHECK_OPT* check_opt);
  bool wrapper_check_and_repair(THD *thd);
  bool storage_check_and_repair(THD *thd);
  int wrapper_analyze(THD* thd, HA_CHECK_OPT* check_opt);
  int storage_analyze(THD* thd, HA_CHECK_OPT* check_opt);
  int wrapper_optimize(THD* thd, HA_CHECK_OPT* check_opt);
  int storage_optimize(THD* thd, HA_CHECK_OPT* check_opt);
  bool wrapper_is_fatal_error(int error_num, uint flags);
  bool storage_is_fatal_error(int error_num, uint flags);
  bool wrapper_check_if_incompatible_data(HA_CREATE_INFO *create_info,
                                          uint table_changes);
  bool storage_check_if_incompatible_data(HA_CREATE_INFO *create_info,
                                          uint table_changes);
  uint wrapper_alter_table_flags(uint flags);
  uint storage_alter_table_flags(uint flags);
#ifdef MRN_HANDLER_HAVE_FINAL_ADD_INDEX
  int wrapper_add_index(TABLE *table_arg, KEY *key_info, uint num_of_keys,
                        handler_add_index **add);
  int storage_add_index(TABLE *table_arg, KEY *key_info, uint num_of_keys,
                        handler_add_index **add);
#else
  int wrapper_add_index(TABLE *table_arg, KEY *key_info, uint num_of_keys);
  int storage_add_index(TABLE *table_arg, KEY *key_info, uint num_of_keys);
#endif
  int storage_add_index_multiple_columns(KEY *key_info, uint num_of_keys,
                                         grn_obj **index_tables,
                                         grn_obj **index_columns,
                                         bool skip_unique_key);
#ifdef MRN_HANDLER_HAVE_FINAL_ADD_INDEX
  int wrapper_final_add_index(handler_add_index *add, bool commit);
  int storage_final_add_index(handler_add_index *add, bool commit);
#endif
  int wrapper_prepare_drop_index(TABLE *table_arg, uint *key_num,
                                 uint num_of_keys);
  int storage_prepare_drop_index(TABLE *table_arg, uint *key_num,
                                 uint num_of_keys);
  int wrapper_final_drop_index(TABLE *table_arg);
  int storage_final_drop_index(TABLE *table_arg);
  int wrapper_update_auto_increment();
  int storage_update_auto_increment();
  void wrapper_set_next_insert_id(ulonglong id);
  void storage_set_next_insert_id(ulonglong id);
  void wrapper_get_auto_increment(ulonglong offset, ulonglong increment,
                                  ulonglong nb_desired_values,
                                  ulonglong *first_value,
                                  ulonglong *nb_reserved_values);
  void storage_get_auto_increment(ulonglong offset, ulonglong increment,
                                  ulonglong nb_desired_values,
                                  ulonglong *first_value,
                                  ulonglong *nb_reserved_values);
  void wrapper_restore_auto_increment(ulonglong prev_insert_id);
  void storage_restore_auto_increment(ulonglong prev_insert_id);
  void wrapper_release_auto_increment();
  void storage_release_auto_increment();
  int wrapper_check_for_upgrade(HA_CHECK_OPT *check_opt);
  int storage_check_for_upgrade(HA_CHECK_OPT *check_opt);
  int wrapper_reset_auto_increment(ulonglong value);
  int storage_reset_auto_increment(ulonglong value);
  bool wrapper_was_semi_consistent_read();
  bool storage_was_semi_consistent_read();
  void wrapper_try_semi_consistent_read(bool yes);
  void storage_try_semi_consistent_read(bool yes);
  void wrapper_unlock_row();
  void storage_unlock_row();
  int wrapper_start_stmt(THD *thd, thr_lock_type lock_type);
  int storage_start_stmt(THD *thd, thr_lock_type lock_type);
  void wrapper_change_table_ptr(TABLE *table_arg, TABLE_SHARE *share_arg);
  void storage_change_table_ptr(TABLE *table_arg, TABLE_SHARE *share_arg);
  bool wrapper_primary_key_is_clustered();
  bool storage_primary_key_is_clustered();
  bool wrapper_is_fk_defined_on_table_or_index(uint index);
  bool storage_is_fk_defined_on_table_or_index(uint index);
  char *wrapper_get_foreign_key_create_info();
  char *storage_get_foreign_key_create_info();
#ifdef MRN_HANDLER_HAVE_GET_TABLESPACE_NAME
  char *wrapper_get_tablespace_name(THD *thd, char *name, uint name_len);
  char *storage_get_tablespace_name(THD *thd, char *name, uint name_len);
#endif
  bool wrapper_can_switch_engines();
  bool storage_can_switch_engines();
  int wrapper_get_foreign_key_list(THD *thd, List<FOREIGN_KEY_INFO> *f_key_list);
  int storage_get_foreign_key_list(THD *thd, List<FOREIGN_KEY_INFO> *f_key_list);
#ifdef MRN_HANDLER_HAVE_GET_PARENT_FOREIGN_KEY_LIST
  int wrapper_get_parent_foreign_key_list(THD *thd, List<FOREIGN_KEY_INFO> *f_key_list);
  int storage_get_parent_foreign_key_list(THD *thd, List<FOREIGN_KEY_INFO> *f_key_list);
#endif
  uint wrapper_referenced_by_foreign_key();
  uint storage_referenced_by_foreign_key();
  void wrapper_init_table_handle_for_HANDLER();
  void storage_init_table_handle_for_HANDLER();
  void wrapper_free_foreign_key_create_info(char* str);
  void storage_free_foreign_key_create_info(char* str);
  void wrapper_set_keys_in_use();
  void storage_set_keys_in_use();
};

#ifdef __cplusplus
}
#endif

#endif /* _ha_mroonga_hpp */
