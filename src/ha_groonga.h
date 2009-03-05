#ifndef _ha_groonga_h
#define _ha_groonga_h

#include "mroonga.h"

typedef struct _mrn_share_field {
  const char *name;
  const char *path;
  uint name_len;
  uint path_len;
  grn_obj *obj;
  grn_id gid;
} mrn_field;

typedef struct _mrn_share {
  const char *name;
  uint name_len;
  uint use_count;
  grn_obj *obj;
  THR_LOCK lock;
  grn_id gid;
  mrn_field **field;
  uint fields;
} mrn_share;

/* handler class */
class ha_groonga: public handler
{
  mrn_share *share;
  THR_LOCK_DATA lock;

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
};

#endif /* _ha_groonga_h */
