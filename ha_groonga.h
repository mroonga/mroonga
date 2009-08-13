#ifndef _ha_groonga_h
#define _ha_groonga_h

#ifdef USE_PRAGMA_INTERFACE
#pragma interface
#endif

#include "driver.h"

#ifdef MRN_HTRACE
#undef MRN_HTRACE
#define MRN_HTRACE {\
    GRN_LOG(ctx, GRN_LOG_DEBUG, "[handler=%p, ctx=%p, minfo=%p] ha_groonga::%s (%s)", \
            this,ctx,minfo, __FUNCTION__,                               \
            (minfo) ? ((minfo->table) ? (minfo->table->name) : NULL) : NULL); \
  } while(0)
#else
#define MRN_HTRACE
#endif

typedef struct _mrn_cond
{
  COND *cond;
  _mrn_cond *next;
} mrn_cond;

/* handler class */
class ha_groonga: public handler
{
  THR_LOCK thr_lock;
  THR_LOCK_DATA thr_lock_data;

  mrn_table *share;
  grn_table_cursor *cursor;
  grn_id record_id;
  grn_obj *res;

  grn_ctx *ctx;

  mrn_info *minfo;
  mrn_cond *mcond;
  mrn_record *cur;
  uchar *column_map;

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
  uint max_supported_keys()          const { return 2; }
  uint max_supported_key_parts()     const { return 1; }
  uint max_supported_key_length()    const { return MAX_KEY_LENGTH; }

  int index_read(uchar *buf, const uchar *key, uint key_len,
		  enum ha_rkey_function find_flag);
  int index_next(uchar *buf);

  int ft_init();
  FT_INFO *ft_init_ext(uint flags, uint inx,String *key);
  int ft_read(uchar *buf);

  const COND *cond_push(const COND *cond);
  void cond_pop();

  // additional functions
  int convert_info(const char *name, TABLE_SHARE *share, mrn_info **minfo);
  int set_bitmap(uchar **bitmap);
};

const char *mrn_item_type_string[] = {
  "FIELD_ITEM", "FUNC_ITEM", "SUM_FUNC_ITEM", "STRING_ITEM",
  "INT_ITEM", "REAL_ITEM", "NULL_ITEM", "VARBIN_ITEM",
  "COPY_STR_ITEM", "FIELD_AVG_ITEM", "DEFAULT_VALUE_ITEM",
  "PROC_ITEM", "COND_ITEM", "REF_ITEM", "FIELD_STD_ITEM",
  "FIELD_VARIANCE_ITEM", "INSERT_VALUE_ITEM",
  "SUBSELECT_ITEM", "ROW_ITEM", "CACHE_ITEM", "TYPE_HOLDER",
  "PARAM_ITEM", "TRIGGER_FIELD_ITEM", "DECIMAL_ITEM",
  "XPATH_NODESET", "XPATH_NODESET_CMP",
  "VIEW_FIXER_ITEM"};

const char *mrn_functype_string[] = {
  "UNKNOWN_FUNC","EQ_FUNC","EQUAL_FUNC","NE_FUNC","LT_FUNC","LE_FUNC",
  "GE_FUNC","GT_FUNC","FT_FUNC",
  "LIKE_FUNC","ISNULL_FUNC","ISNOTNULL_FUNC",
  "COND_AND_FUNC", "COND_OR_FUNC", "COND_XOR_FUNC",
  "BETWEEN", "IN_FUNC", "MULT_EQUAL_FUNC",
  "INTERVAL_FUNC", "ISNOTNULLTEST_FUNC",
  "SP_EQUALS_FUNC", "SP_DISJOINT_FUNC","SP_INTERSECTS_FUNC",
  "SP_TOUCHES_FUNC","SP_CROSSES_FUNC","SP_WITHIN_FUNC",
  "SP_CONTAINS_FUNC","SP_OVERLAPS_FUNC",
  "SP_STARTPOINT","SP_ENDPOINT","SP_EXTERIORRING",
  "SP_POINTN","SP_GEOMETRYN","SP_INTERIORRINGN",
  "NOT_FUNC", "NOT_ALL_FUNC",
  "NOW_FUNC", "TRIG_COND_FUNC",
  "SUSERVAR_FUNC", "GUSERVAR_FUNC", "COLLATE_FUNC",
  "EXTRACT_FUNC", "CHAR_TYPECAST_FUNC", "FUNC_SP", "UDF_FUNC",
  "NEG_FUNC", "GSYSVAR_FUNC"};

#endif /* _ha_groonga_h */
