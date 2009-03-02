#ifndef _mroonga_h
#define _mroonga_h

#include <stdlib.h>
#include <stdio.h>

#define MRN_MAX_KEY_LEN 1024
#define MRN_DB_FILE_PATH "mroonga.grn"
#define MRN_TABLE_FILE_EXT ".grn"
#define MRN_LOG_FILE_NAME "mroonga.log"

/* type definition */
typedef struct _mrn_charset_map {
  const char *csname_mysql;    /* encoding name of mysql */
  grn_encoding csname_groonga; /* encoding name of groonga */
} MRN_CHARSET_MAP;

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
#define MRN_NAME(obj_name) (obj_name - 2)
#define MRN_OBJ_NAME(name) (name + 2)
#define MRN_OBJ_PATH(buf,name) snprintf(buf,MRN_MAX_KEY_LEN-1,"%s%s",name, MRN_TABLE_FILE_EXT)

#endif /* _mroonga_h */
