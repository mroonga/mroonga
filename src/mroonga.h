#ifndef _mroonga_h
#define _mroonga_h

#include <stdlib.h>
#include <stdio.h>

/* max 64 chars at utf8 */
#define MRN_MAX_IDENTIFIER_LEN 192

/* type definition */
typedef struct _mrn_charset_map {
  const char* csname_mysql;
  grn_encoding csname_groonga;
} MRN_CHARSET_MAP;

/* macro */
#define MRN_MALLOC(size) malloc(size)
#define MRN_FREE(ptr) free(ptr)

#define MRN_LOG(level, ...) do {\
  grn_ctx ctx = GRN_CTX_INITIALIZER;\
  GRN_LOG(&ctx, level, __VA_ARGS__);\
  } while(0)

#define MRN_TRACE do { \
  char buf[64]; \
  snprintf(buf,63,"%s:%d %s", __FILE__, __LINE__, __FUNCTION__);	\
  MRN_LOG(GRN_LOG_DUMP, buf); \
  } while(0)

#endif /* _mroonga_h */
