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

#ifndef _ha_mroonga_h
#define _ha_mroonga_h

#ifdef USE_PRAGMA_INTERFACE
#pragma interface
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <groonga.h>
#include "mrnsys.h"

/* structs */
struct st_mrn_statuses
{
  long count_skip;
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

/* functions */
int mrn_init(void *hton);
int mrn_deinit(void *hton);
handler *mrn_handler_create(handlerton *hton, TABLE_SHARE *share, MEM_ROOT *root);
void mrn_drop_db(handlerton *hton, char *path);
int mrn_close_connection(handlerton *hton, THD *thd);
float mrn_ft_find_relevance(FT_INFO *handler, uchar *record, uint length);
float mrn_ft_get_relevance(FT_INFO *handler);
void mrn_ft_close_search(FT_INFO *handler);

/* handler class */
class ha_mroonga: public handler
{
  THR_LOCK thr_lock;
  THR_LOCK_DATA thr_lock_data;

  grn_ctx *ctx;

  grn_obj *db;
  grn_obj *tbl;
  grn_obj **col;

  grn_obj **idx_tbl;
  grn_obj **idx_col;

  grn_obj *res;
  grn_table_cursor *cur;
  grn_id row_id;
  grn_obj *_score;

  st_mrn_ft_info mrn_ft_info;

  char **key_min;
  char **key_max;
  int *key_min_len;
  int *key_max_len;

  bool count_skip;

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

  THR_LOCK_DATA **store_lock(THD *thd,                             // required
			     THR_LOCK_DATA **to,
			     enum thr_lock_type lock_type);

  int rnd_init(bool scan);                                         // required
  int rnd_end();
  int rnd_next(uchar *buf);                                        // required
  int rnd_pos(uchar *buf, uchar *pos);                             // required
  void position(const uchar *record);                              // required

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

private:
  void check_count_skip(key_part_map start_key_part_map,
                        key_part_map end_key_part_map, bool fulltext);
  void store_fields_from_primary_table(uchar *buf, grn_id rid);
};

#ifdef __cplusplus
}
#endif

#endif /* _ha_mroonga_h */
