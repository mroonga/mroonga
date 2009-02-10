#ifndef _ha_groonga_h
#define _ha_groonga_h

/* max 64 chars at utf8 */
#define MRN_MAX_IDENTIFIER_LEN 192

extern grn_ctx *mrn_ctx_sys;
extern grn_hash *mrn_hash_sys;
extern pthread_mutex_t *mrn_mutex_sys;

class ha_groonga: public handler
{
public:
  ha_groonga(handlerton *hton, TABLE_SHARE *share);
  ~ha_groonga();

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

#define MROONGA_DEBUG
#ifdef MROONGA_DEBUG
#define MRN_ENTER \
  do {GRN_LOG(mrn_ctx_sys, GRN_LOG_DEBUG, "enter"); } while(0)
#define MRN_RETURN_VOID \
  do {GRN_LOG(mrn_ctx_sys, GRN_LOG_DEBUG, "return void"); return; } while(0)
#define MRN_RETURN(res) \
  do {GRN_LOG(mrn_ctx_sys, GRN_LOG_DEBUG, "return %d", res); return res; } while(0)
#define MRN_RETURN_P(res) \
  do {GRN_LOG(mrn_ctx_sys, GRN_LOG_DEBUG, "return %p", res); return res; } while(0)
#define MRN_RETURN_S(res) \
  do {GRN_LOG(mrn_ctx_sys, GRN_LOG_DEBUG, "return %s", res); return res; } while(0)
#define MRN_RETURN_F(res) \
  do {GRN_LOG(mrn_ctx_sys, GRN_LOG_DEBUG, "return %f", res); return res; } while(0)
#define MRN_LOG(level, ...) \
  GRN_LOG(mrn_ctx_sys, level, __VA_ARGS__)
#endif /* MROONGA_DEBUG */

#endif /* _ha_groonga_h */
