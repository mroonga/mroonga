#ifndef _ha_yase_h
#define _ha_yase_h
#ifdef __cplusplus
extern "C" {
#endif

class ha_yase: public handler
{
public:
  ha_yase(handlerton *hton, TABLE_SHARE *share);
  ~ha_yase();

  const char *table_type() const;                                  // required
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
};

#ifdef __cplusplus
}
#endif
#endif
