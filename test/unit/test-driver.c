#include <gcutter.h>
#include <glib/gstdio.h>
#include "driver.h"

static gchar *base_directory;
static gchar *tmp_directory;
static grn_ctx *ctx;

void cut_startup()
{
  base_directory = g_get_current_dir();
  tmp_directory = g_build_filename(base_directory, "/tmp", NULL);
  cut_remove_path(tmp_directory, NULL);
  g_mkdir_with_parents(tmp_directory, 0700);
  g_chdir(tmp_directory);
  ctx = (grn_ctx*) g_malloc(sizeof(grn_ctx));
}

void cut_shutdown()
{
  g_chdir(base_directory);
  g_free(tmp_directory);
  g_free(ctx);
}

void cut_setup()
{
  mrn_init();
  grn_ctx_init(ctx,0);
  grn_ctx_use(ctx, mrn_db);
}

void cut_teardown()
{
  grn_ctx_fin(ctx);
  mrn_deinit();
}

void test_mrn_flush_logs()
{
  cut_assert_equal_int(0, mrn_flush_logs(ctx));
}

void test_mrn_hash_put()
{
  const char *key1 = "aa";
  const char *key2 = "bbb";
  const char *key3 = "cccc";
  char *value1 = "11";
  char *value2 = "222";
  char *value3 = "3333";

  cut_assert_equal_int(0,mrn_hash_put(ctx, key1, (void*) value1));
  cut_assert_equal_int(0,mrn_hash_put(ctx, key2, (void*) value2));
  cut_assert_equal_int(0,mrn_hash_put(ctx, key3, (void*) value3));
  cut_assert_equal_int(-1,mrn_hash_put(ctx, key1, (void*) value2));
  cut_assert_equal_int(-1,mrn_hash_put(ctx, key2, (void*) value3));
  cut_assert_equal_int(-1,mrn_hash_put(ctx, key3, (void*) value1));
}

void test_mrn_hash_get()
{
  const char *key1 = "aa";
  const char *key2 = "bbb";
  const char *key3 = "cccc";
  const char *key4 = "not found";
  char *value1 = "11";
  char *value2 = "222";
  char *value3 = "3333";
  void *res;

  mrn_hash_put(ctx, key1, (void*) value1);
  mrn_hash_put(ctx, key2, (void*) value2);
  mrn_hash_put(ctx, key3, (void*) value3);

  cut_assert_equal_int(0, mrn_hash_get(ctx, key1, &res));
  cut_assert_equal_string(value1, (char*) res);
  cut_assert_equal_int(0, mrn_hash_get(ctx, key2, &res));
  cut_assert_equal_string(value2, (char*) res);
  cut_assert_equal_int(0, mrn_hash_get(ctx, key3, &res));
  cut_assert_equal_string(value3, (char*) res);
  cut_assert_equal_int(-1, mrn_hash_get(ctx, key4, &res));

  mrn_hash_remove(ctx,key2);
}

void test_mrn_hash_remove()
{
  const char *key1 = "aa";
  const char *key2 = "bbb";
  const char *key3 = "cccc";
  const char *key4 = "not found";
  char *value1 = "11";
  char *value2 = "222";
  char *value3 = "3333";

  mrn_hash_put(ctx, key1, (void*) value1);
  mrn_hash_put(ctx, key2, (void*) value2);
  mrn_hash_put(ctx, key3, (void*) value3);

  cut_assert_equal_int(-1, mrn_hash_remove(ctx, key4));
  cut_assert_equal_int(0, mrn_hash_remove(ctx, key1));
  cut_assert_equal_int(0, mrn_hash_remove(ctx, key2));
  cut_assert_equal_int(0, mrn_hash_remove(ctx, key3));
  cut_assert_equal_int(-1, mrn_hash_remove(ctx, key1));
  cut_assert_equal_int(-1, mrn_hash_remove(ctx, key2));
  cut_assert_equal_int(-1, mrn_hash_remove(ctx, key3));
}
