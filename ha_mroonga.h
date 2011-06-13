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

#ifndef _ha_mroonga_h
#define _ha_mroonga_h

#ifdef USE_PRAGMA_INTERFACE
#pragma interface
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <groonga.h>
#include "mrn_sys.h"

/* structs */
struct st_mrn_statuses
{
  long count_skip;
  long fast_order_limit;
};

struct st_mrn_ft_info
{
  struct _ft_vft *please;
  grn_ctx *ctx;
  grn_obj *res;
  grn_id rid;
};

struct st_mrn_slot_data
{
  grn_id last_insert_rid;
};

/* handler class */
class ha_mroonga: public handler
{
  THR_LOCK thr_lock;
  THR_LOCK_DATA thr_lock_data;

  MRN_SHARE *share;
  KEY       *wrap_key_info;
  KEY       *base_key_info;
  MEM_ROOT  mem_root;
public:
  handler   *wrap_handler;
  bool      is_clone;
  ha_mroonga *parent_for_clone;
  MEM_ROOT  *mem_root_for_clone;

private:
  grn_ctx *ctx;

  grn_obj *db;
  grn_obj *tbl;
  grn_obj **col;

  grn_obj **idx_tbl;
  grn_obj **idx_col;

  grn_obj *res;
  grn_obj *res0;
  grn_table_cursor *cur;
  grn_table_cursor *cur0;
  grn_id row_id;
  grn_obj *_score;

  st_mrn_ft_info mrn_ft_info;

  char **key_min;
  char **key_max;
  int *key_min_len;
  int *key_max_len;
  uint dup_key;

  longlong limit;
  grn_table_sort_key *sort_keys;
  int n_sort_keys;

  bool count_skip;
  bool fast_order_limit;
  bool fast_order_limit_with_index;

  bool ignoring_duplicated_key;

public:
  ha_mroonga(handlerton *hton, TABLE_SHARE *share);
  ~ha_mroonga();
  const char *table_type() const;           // required
  const char *index_type(uint inx);
  const char **bas_ext() const;                                    // required

  ulonglong table_flags() const;                                   // required
  ulong index_flags(uint idx, uint part, bool all_parts) const;    // required

  int create(const char *name, TABLE *form, HA_CREATE_INFO *info); // required
  int open(const char *name, int mode, uint test_if_locked);       // required
  int close();                                                     // required
  int info(uint flag);                                             // required

  uint lock_count() const;
  THR_LOCK_DATA **store_lock(THD *thd,                             // required
                             THR_LOCK_DATA **to,
                             enum thr_lock_type lock_type);
  int external_lock(THD *thd, int lock_type);

  int rnd_init(bool scan);                                         // required
  int rnd_end();
  int rnd_next(uchar *buf);                                        // required
  int rnd_pos(uchar *buf, uchar *pos);                             // required
  void position(const uchar *record);                              // required
  int extra(enum ha_extra_function operation);
  int extra_opt(enum ha_extra_function operation, ulong cache_size);

  int delete_table(const char *name);
  int write_row(uchar *buf);
  int update_row(const uchar *old_data, uchar *new_data);
  int delete_row(const uchar *buf);

  uint max_supported_record_length() const { return HA_MAX_REC_LENGTH; }
  uint max_supported_keys()          const { return 100; }
  uint max_supported_key_parts()     const { return 1; }
  uint max_supported_key_length()    const { return MAX_KEY_LENGTH; }

  ha_rows records_in_range(uint inx, key_range *min_key, key_range *max_key);
  int index_init(uint idx, bool sorted);
  int index_end();
  int index_read_map(uchar * buf, const uchar * key,
                     key_part_map keypart_map,
                     enum ha_rkey_function find_flag);
  int index_read_last_map(uchar *buf, const uchar *key,
                          key_part_map keypart_map);
  int index_next(uchar * buf);
  int index_prev(uchar * buf);
  int index_first(uchar * buf);
  int index_last(uchar * buf);
  int index_next_same(uchar *buf, const uchar *key, uint keylen);

  int read_range_first(const key_range *start_key,
                       const key_range *end_key,
                       bool eq_range, bool sorted);
  int read_range_next();

  int ft_init();
  FT_INFO *ft_init_ext(uint flags, uint inx,String *key);
  int ft_read(uchar *buf);

  const COND *cond_push(const COND *cond);
  void cond_pop();

  bool get_error_message(int error, String *buf);

  int reset();

#if MYSQL_VERSION_ID >= 50513
  handler *clone(const char *name, MEM_ROOT *mem_root);
#else
  handler *clone(MEM_ROOT *mem_root);
#endif
  uint8 table_cache_type();
  int read_multi_range_first(KEY_MULTI_RANGE **found_range_p,
                             KEY_MULTI_RANGE *ranges,
                             uint range_count,
                             bool sorted,
                             HANDLER_BUFFER *buffer);
  int read_multi_range_next(KEY_MULTI_RANGE **found_range_p);
  void start_bulk_insert(ha_rows rows);
  int end_bulk_insert();
  int delete_all_rows();
  int truncate();
  double scan_time();
  double read_time(uint index, uint ranges, ha_rows rows);
  const key_map *keys_to_use_for_scanning();
  ha_rows estimate_rows_upper_bound();
  void update_create_info(HA_CREATE_INFO* create_info);
  int rename_table(const char *from, const char *to);
  bool is_crashed() const;
  bool auto_repair() const;
  int disable_indexes(uint mode);
  int enable_indexes(uint mode);
  int check(THD* thd, HA_CHECK_OPT* check_opt);
  int repair(THD* thd, HA_CHECK_OPT* check_opt);
  bool check_and_repair(THD *thd);
  int analyze(THD* thd, HA_CHECK_OPT* check_opt);
  int optimize(THD* thd, HA_CHECK_OPT* check_opt);
  bool is_fatal_error(int error_num, uint flags);

private:
  void check_count_skip(key_part_map start_key_part_map,
                        key_part_map end_key_part_map, bool fulltext);
  void check_fast_order_limit();
  void store_fields_from_primary_table(uchar *buf, grn_id rid);
  int wrapper_create(const char *name, TABLE *table,
                     HA_CREATE_INFO *info, MRN_SHARE *tmp_share);
  int default_create(const char *name, TABLE *table,
                     HA_CREATE_INFO *info, MRN_SHARE *tmp_share);
  int wrapper_create_index(const char *name, TABLE *table,
                           HA_CREATE_INFO *info, MRN_SHARE *tmp_share);
  int default_create_validate_pseudo_column(TABLE *table);
  int default_create_validate_index(TABLE *table);
  int ensure_database_create(const char *name);
  int ensure_database_open(const char *name);
  int wrapper_delete_table(const char *name, MRN_SHARE *tmp_share);
  int default_delete_table(const char *name, MRN_SHARE *tmp_share,
                           const char *tbl_name);
  int wrapper_open(const char *name, int mode, uint test_if_locked);
  int default_open(const char *name, int mode, uint test_if_locked);
  int wrapper_close();
  int default_close();
  int mrn_extra(enum ha_extra_function operation);
  int wrapper_extra(enum ha_extra_function operation);
  int default_extra(enum ha_extra_function operation);
  int wrapper_extra_opt(enum ha_extra_function operation, ulong cache_size);
  int default_extra_opt(enum ha_extra_function operation, ulong cache_size);
  int wrapper_reset();
  int default_reset();
  uint wrapper_lock_count() const;
  uint default_lock_count() const;
  THR_LOCK_DATA **wrapper_store_lock(THD *thd, THR_LOCK_DATA **to,
                                     enum thr_lock_type lock_type);
  THR_LOCK_DATA **default_store_lock(THD *thd, THR_LOCK_DATA **to,
                                     enum thr_lock_type lock_type);
  int wrapper_external_lock(THD *thd, int lock_type);
  int default_external_lock(THD *thd, int lock_type);
  void wrapper_start_bulk_insert(ha_rows rows);
  void default_start_bulk_insert(ha_rows rows);
  int wrapper_end_bulk_insert();
  int default_end_bulk_insert();
  int wrapper_write_row(uchar *buf);
  int default_write_row(uchar *buf);
  int wrapper_update_row(const uchar *old_data, uchar *new_data);
  int default_update_row(const uchar *old_data, uchar *new_data);
  int wrapper_delete_row(const uchar *buf);
  int default_delete_row(const uchar *buf);
  ulonglong wrapper_table_flags() const;
  ulonglong default_table_flags() const;
  ulong wrapper_index_flags(uint idx, uint part, bool all_parts) const;
  ulong default_index_flags(uint idx, uint part, bool all_parts) const;
  int wrapper_info(uint flag);
  int default_info(uint flag);
  int wrapper_rnd_init(bool scan);
  int default_rnd_init(bool scan);
  int wrapper_rnd_end();
  int default_rnd_end();
  int wrapper_rnd_next(uchar *buf);
  int default_rnd_next(uchar *buf);
  int wrapper_rnd_pos(uchar *buf, uchar *pos);
  int default_rnd_pos(uchar *buf, uchar *pos);
  void wrapper_position(const uchar *record);
  void default_position(const uchar *record);
  ha_rows wrapper_records_in_range(uint key_nr, key_range *range_min,
                                   key_range *range_max);
  ha_rows default_records_in_range(uint key_nr, key_range *range_min,
                                   key_range *range_max);
  int wrapper_index_init(uint idx, bool sorted);
  int default_index_init(uint idx, bool sorted);
  int wrapper_index_end();
  int default_index_end();
  int wrapper_index_read_map(uchar * buf, const uchar * key,
                             key_part_map keypart_map,
                             enum ha_rkey_function find_flag);
  int default_index_read_map(uchar * buf, const uchar * key,
                             key_part_map keypart_map,
                             enum ha_rkey_function find_flag);
  int wrapper_index_read_last_map(uchar *buf, const uchar *key,
                                  key_part_map keypart_map);
  int default_index_read_last_map(uchar *buf, const uchar *key,
                                  key_part_map keypart_map);
  int wrapper_index_next(uchar *buf);
  int default_index_next(uchar *buf);
  int wrapper_index_prev(uchar *buf);
  int default_index_prev(uchar *buf);
  int wrapper_index_first(uchar *buf);
  int default_index_first(uchar *buf);
  int wrapper_index_last(uchar *buf);
  int default_index_last(uchar *buf);
  int wrapper_index_next_same(uchar *buf, const uchar *key, uint keylen);
  int default_index_next_same(uchar *buf, const uchar *key, uint keylen);
  int wrapper_read_range_first(const key_range *start_key,
                               const key_range *end_key,
                               bool eq_range, bool sorted);
  int default_read_range_first(const key_range *start_key,
                               const key_range *end_key,
                               bool eq_range, bool sorted);
  int wrapper_read_range_next();
  int default_read_range_next();
  int wrapper_ft_init();
  int default_ft_init();
  FT_INFO *wrapper_ft_init_ext(uint flags, uint key_nr, String *key);
  FT_INFO *default_ft_init_ext(uint flags, uint key_nr, String *key);
  int wrapper_ft_read(uchar *buf);
  int default_ft_read(uchar *buf);
  const COND *wrapper_cond_push(const COND *cond);
  const COND *default_cond_push(const COND *cond);
  void wrapper_cond_pop();
  void default_cond_pop();
  bool wrapper_get_error_message(int error, String *buf);
  bool default_get_error_message(int error, String *buf);
#if MYSQL_VERSION_ID >= 50513
  handler *wrapper_clone(const char *name, MEM_ROOT *mem_root);
  handler *default_clone(const char *name, MEM_ROOT *mem_root);
#else
  handler *wrapper_clone(MEM_ROOT *mem_root);
  handler *default_clone(MEM_ROOT *mem_root);
#endif
  uint8 wrapper_table_cache_type();
  uint8 default_table_cache_type();
  int wrapper_read_multi_range_first(KEY_MULTI_RANGE **found_range_p,
                                     KEY_MULTI_RANGE *ranges,
                                     uint range_count,
                                     bool sorted,
                                     HANDLER_BUFFER *buffer);
  int default_read_multi_range_first(KEY_MULTI_RANGE **found_range_p,
                                     KEY_MULTI_RANGE *ranges,
                                     uint range_count,
                                     bool sorted,
                                     HANDLER_BUFFER *buffer);
  int wrapper_read_multi_range_next(KEY_MULTI_RANGE **found_range_p);
  int default_read_multi_range_next(KEY_MULTI_RANGE **found_range_p);
  int wrapper_delete_all_rows();
  int default_delete_all_rows();
  int wrapper_truncate();
  int default_truncate();
  double wrapper_scan_time();
  double default_scan_time();
  double wrapper_read_time(uint index, uint ranges, ha_rows rows);
  double default_read_time(uint index, uint ranges, ha_rows rows);
  const key_map *wrapper_keys_to_use_for_scanning();
  const key_map *default_keys_to_use_for_scanning();
  ha_rows wrapper_estimate_rows_upper_bound();
  ha_rows default_estimate_rows_upper_bound();
  void wrapper_update_create_info(HA_CREATE_INFO* create_info);
  void default_update_create_info(HA_CREATE_INFO* create_info);
  int wrapper_rename_table(const char *from, const char *to);
  int default_rename_table(const char *from, const char *to);
  bool wrapper_is_crashed() const;
  bool default_is_crashed() const;
  bool wrapper_auto_repair() const;
  bool default_auto_repair() const;
  int wrapper_disable_indexes(uint mode);
  int default_disable_indexes(uint mode);
  int wrapper_enable_indexes(uint mode);
  int default_enable_indexes(uint mode);
  int wrapper_check(THD* thd, HA_CHECK_OPT* check_opt);
  int default_check(THD* thd, HA_CHECK_OPT* check_opt);
  int wrapper_repair(THD* thd, HA_CHECK_OPT* check_opt);
  int default_repair(THD* thd, HA_CHECK_OPT* check_opt);
  bool wrapper_check_and_repair(THD *thd);
  bool default_check_and_repair(THD *thd);
  int wrapper_analyze(THD* thd, HA_CHECK_OPT* check_opt);
  int default_analyze(THD* thd, HA_CHECK_OPT* check_opt);
  int wrapper_optimize(THD* thd, HA_CHECK_OPT* check_opt);
  int default_optimize(THD* thd, HA_CHECK_OPT* check_opt);
  bool wrapper_is_fatal_error(int error_num, uint flags);
  bool default_is_fatal_error(int error_num, uint flags);
};

#ifdef __cplusplus
}
#endif

#endif /* _ha_mroonga_h */
