#ifndef _ha_groonga_h
#define _ha_groonga_h

#ifdef USE_PRAGMA_INTERFACE
#pragma interface
#endif

#include "mroonga.h"

typedef struct _mrn_share_field {
  const char *name;
  uint name_len;
  grn_obj *obj;
  grn_id gid;
  uint field_no;
} mrn_field;

typedef struct _mrn_share {
  const char *name;
  uint name_len;
  uint use_count;
  grn_obj *obj;
  grn_id gid;
  mrn_field **field;
  uint fields;
  uint pkey_field;
} mrn_share;

/* handler class */
class ha_groonga: public handler
{
  THR_LOCK thr_lock;
  THR_LOCK_DATA thr_lock_data;

  mrn_share *share;
  grn_table_cursor *cursor;
  grn_id record_id;

public:
  ha_groonga(handlerton *hton, TABLE_SHARE *share);
  ~ha_groonga();

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

  uint max_supported_record_length() const { return HA_MAX_REC_LENGTH; }
  uint max_supported_keys()          const { return 1; }
  uint max_supported_key_parts()     const { return 1; }
  uint max_supported_key_length()    const { return MAX_KEY_LENGTH; }

  int index_read(uchar *buf, const uchar *key, uint key_len,
		  enum ha_rkey_function find_flag);
  int index_next(uchar *buf);
};

#endif /* _ha_groonga_h */
