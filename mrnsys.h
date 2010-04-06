#ifndef _mrn_util_h
#define _mrn_util_h

#include <groonga.h>

/* constants */
#define MRN_PACKAGE_STRING "mroonga 0.1"
#define MRN_MAX_KEY_SIZE 1024
#define MRN_MAX_PATH_SIZE 256
#define MRN_MAX_EXPRS 32
#define MRN_DB_FILE_SUFFIX ".mrn"
#define MRN_LOG_FILE_NAME "mroonga.log"
#define MRN_LEX_SUFFIX "_lex"
#define MRN_HASH_SUFFIX "_hash"
#define MRN_PAT_SUFFIX "_pat"

/* functions */
int mrn_hash_put(grn_ctx *ctx, grn_hash *hash, const char *key, void *value);
int mrn_hash_get(grn_ctx *ctx, grn_hash *hash, const char *key, void **value);
int mrn_hash_remove(grn_ctx *ctx, grn_hash *hash, const char *key);

char *mrn_db_path_gen(const char *arg, char *dest);
char *mrn_db_name_gen(const char *arg, char *dest);
char *mrn_table_name_gen(const char *arg, char *dest);
char *mrn_lex_name_gen(const char *arg, char *dest);
char *mrn_hash_name_gen(const char *arg, char *dest);
char *mrn_pat_name_gen(const char *arg, char *dest);

int mrn_check_table_name(const char *table_name);

#endif /* _mrn_util_h */
