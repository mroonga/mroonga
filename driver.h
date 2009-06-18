#ifndef _driver_h
#define _driver_h

#include <stdlib.h>
#include <stdio.h>
#include <groonga.h>

#define MRN_MAX_KEY_LEN 1024
#define MRN_DB_FILE_PATH "groonga.db"
#define MRN_LOG_FILE_NAME "groonga.log"


/* TLS variables */
extern __thread grn_ctx *mrn_ctx_tls;


/* type definition */

typedef struct _mrn_charset_map {
  const char *csname_mysql;   
  grn_encoding csname_groonga;
} MRN_CHARSET_MAP;

typedef struct _mrn_share_field {
  const char *name;
  uint name_len;
  grn_obj *obj;
  grn_obj *index;
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


/* macro */
#define MRN_MALLOC(size) malloc(size)
#define MRN_FREE(ptr) free(ptr)

#define MRN_LOG(level, ...) GRN_LOG(mrn_ctx_tls, level, __VA_ARGS__)

#define MRN_TRACE do { \
  char buf[64]; \
  snprintf(buf,63,"%s", __FUNCTION__);	\
  MRN_LOG(GRN_LOG_DEBUG, buf); \
  } while(0)

/* name, obj_name, path */
#define MRN_HANDLER_NAME(obj_name) (obj_name - 2)
#define MRN_TABLE_NAME(name) (name + 2)

/* functions */
int mrn_init();
int mrn_deinit();
void mrn_logger_func(int level, const char *time, const char *title,
		     const char *msg, const char *location, void *func_arg);
int mrn_flush_logs();
grn_encoding mrn_charset_mysql_groonga(const char *csname);
const char *mrn_charset_groonga_mysql(grn_encoding encoding);
void mrn_ctx_init();
grn_obj *mrn_db_open_or_create();
mrn_share *mrn_share_get(const char *name);
void mrn_share_put(mrn_share *share);
void mrn_share_remove(mrn_share *share);
grn_obj *mrn_get_type(int type);


/* static variables */
extern grn_hash *mrn_hash_sys;
extern grn_obj *mrn_db_sys, *mrn_lexicon_sys;
extern pthread_mutex_t *mrn_mutex_sys;
extern const char *mrn_logfile_name;
extern FILE *mrn_logfile;

extern grn_logger_info mrn_logger_info;

#endif /* _driver_h */
