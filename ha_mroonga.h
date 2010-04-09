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

/* functions */
int mrn_init(void *hton);
int mrn_deinit(void *hton);
handler *mrn_handler_create(handlerton *hton, TABLE_SHARE *share, MEM_ROOT *root);
void mrn_drop_db(handlerton *hton, char *path);

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
  int rnd_next(uchar *buf);                                        // required
  int rnd_pos(uchar *buf, uchar *pos);                             // required
  void position(const uchar *record);                              // required

  int delete_table(const char *name);
  int write_row(uchar *buf);
  int update_row(const uchar *old_data, uchar *new_data);
  int delete_row(const uchar *buf);

  uint max_supported_record_length() const { return HA_MAX_REC_LENGTH; }
  uint max_supported_keys()          const { return 2; }
  uint max_supported_key_parts()     const { return 1; }
  uint max_supported_key_length()    const { return MAX_KEY_LENGTH; }

  ha_rows records_in_range(uint inx, key_range *min_key, key_range *max_key);

  int index_read(uchar *buf, const uchar *key, uint key_len, enum ha_rkey_function find_flag);
  int index_next(uchar *buf);

  int ft_init();
  FT_INFO *ft_init_ext(uint flags, uint inx,String *key);
  int ft_read(uchar *buf);

  const COND *cond_push(const COND *cond);
  void cond_pop();
};

#ifdef __cplusplus
}
#endif

#endif /* _ha_mroonga_h */
