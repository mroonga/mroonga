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
  handler   *wrap_handler;

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

  uint lock_count();
  THR_LOCK_DATA **store_lock(THD *thd,                             // required
			     THR_LOCK_DATA **to,
			     enum thr_lock_type lock_type);

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
  int default_create_validate_pseudo_column(TABLE *table);
  int default_create_validate_index(TABLE *table);
  int default_create_ensure_database_open(const char *name);
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
  uint wrapper_lock_count();
  uint default_lock_count();
};

#ifdef __cplusplus
}
#endif

#endif /* _ha_mroonga_h */
