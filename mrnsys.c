#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mrnsys.h"
#include "config.h"

int mrn_hash_put(grn_ctx *ctx, grn_hash *hash, const char *key, void *value)
{
  int added, res=0;
  void *buf;
  grn_hash_add(ctx, hash, (const char*) key, strlen(key), &buf, &added);
  // duplicate check
  if (added == 0) {
    GRN_LOG(ctx, GRN_LOG_WARNING, "hash put duplicated (key=%s)", key);
    res = -1;
  } else {
    // store address of value
    memcpy(buf, &value, sizeof(buf));
    GRN_LOG(ctx, GRN_LOG_DEBUG, "hash put (key=%s)", key);
  }
  return res;
}

int mrn_hash_get(grn_ctx *ctx, grn_hash *hash, const char *key, void **value)
{
  int res = 0;
  grn_id id;
  void *buf;
  id = grn_hash_get(ctx, hash, (const char*) key, strlen(key), &buf);
  // key not found
  if (id == GRN_ID_NIL) {
    GRN_LOG(ctx, GRN_LOG_DEBUG, "hash get not found (key=%s)", key);
    res = -1;
  } else {
    // restore address of value
    memcpy(value, buf, sizeof(buf));
  }
  return res;
}

int mrn_hash_remove(grn_ctx *ctx, grn_hash *hash, const char *key)
{
  int res = 0;
  grn_rc rc;
  grn_id id;
  id = grn_hash_get(ctx, hash, (const char*) key, strlen(key), NULL);
  if (id == GRN_ID_NIL) {
    GRN_LOG(ctx, GRN_LOG_WARNING, "hash remove not found (key=%s)", key);
    res = -1;
  } else {
    rc = grn_hash_delete_by_id(ctx, hash, id, NULL);
    if (rc != GRN_SUCCESS) {
      GRN_LOG(ctx, GRN_LOG_ERROR, "hash remove error (key=%s)", key);
      res = -1;
    } else {
      GRN_LOG(ctx, GRN_LOG_DEBUG, "hash remove (key=%s)", key);
    }
  }
  return res;
}

/**
 * "./${db}/${tbl}" ==> "${db}.mrn"
 * "./${db}/"       ==> "${db}.mrn"
 */
char *mrn_db_path_gen(const char *arg, char *dest)
{
  int i=2, j=0, len = strlen(arg);
  while (arg[i] != '/' && i < len) {
    dest[j++] = arg[i++];
  }
  dest[j] = '\0';
  strcat(dest, MRN_DB_FILE_SUFFIX);
  return dest;
}

/**
 * "./${db}/${tbl}" ==> "${db}"
 * "./${db}/"       ==> "${db}"
 */
char *mrn_db_name_gen(const char *arg, char *dest)
{
  int i=2, j=0, len = strlen(arg);
  while (arg[i] != '/' && i < len) {
    dest[j++] = arg[i++];
  }
  dest[j] = '\0';
  return dest;
}

/**
 * "./${db}/${tbl}" ==> "${tbl}"
 */
char *mrn_table_name_gen(const char *arg, char *dest)
{
  int len = strlen(arg);
  int i=len, j=0;
  for (; arg[--i] != '/' ;) {}
  for (; i <= len ;) {
    dest[j++] = arg[++i];
  }
  dest[j] = '\0';
  return dest;
}

/**
 * "${tbl}" ==> "${tbl}_0000"
 * "${tbl}" ==> "${tbl}_0001"
 * "${tbl}" ==> "${tbl}_0002"
 * "${tbl}" ==> "${tbl}_0003"
 * ...
 */
char *mrn_index_name_gen(const char *table_name, int idx, char *dest)
{
  snprintf(dest, MRN_MAX_PATH_SIZE, "%s_%04d", table_name, idx);
  return dest;
}
