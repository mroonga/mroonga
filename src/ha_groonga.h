#ifndef _ha_groonga_h
#define _ha_groonga_h

extern grn_ctx grn_gctx;

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
#define MRN_ENTER        do {GRN_LOG(grn_log_debug, "enter"); } while(0)
#define MRN_RETURN_VOID  do {GRN_LOG(grn_log_debug, "return void"); return; } while(0)
#define MRN_RETURN(x)  do {GRN_LOG(grn_log_debug, "return %d", x); return x; } while(0)
#define MRN_RETURN_P(x)  do {GRN_LOG(grn_log_debug, "return %p", x); return x; } while(0)
#define MRN_RETURN_S(x)  do {GRN_LOG(grn_log_debug, "return %s", x); return x; } while(0)
#define MRN_RETURN_F(x)  do {GRN_LOG(grn_log_debug, "return %f", x); return x; } while(0)
#define MRN_DEBUG(...)   GRN_LOG(grn_log_debug,__VA_ARGS__)
#endif /* MROONGA_DEBUG */

#endif
