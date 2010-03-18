#include <string.h>
#include <gcutter.h>
#include <glib/gstdio.h>
#include "driver.h"

#define TEST_ENTER do { GRN_LOG(ctx, GRN_LOG_DEBUG,             \
                                "<%s>", __FUNCTION__);} while(0)

static gchar *base_directory;
static gchar *tmp_directory;
static grn_ctx *ctx;
static mrn_object *obj;

void cut_startup()
{
  base_directory = g_get_current_dir();
  tmp_directory = g_build_filename(base_directory, "/tmp", NULL);
  cut_remove_path(tmp_directory, NULL);
  g_mkdir_with_parents(tmp_directory, 0700);
  g_chdir(tmp_directory);
  ctx = (grn_ctx*) g_malloc(sizeof(grn_ctx));
  obj = (mrn_object *) g_malloc(sizeof(mrn_object));
}

void cut_shutdown()
{
  g_chdir(base_directory);
  g_free(tmp_directory);
  g_free(ctx);
}

void cut_setup()
{
  mrn_init(0);
  grn_ctx_init(ctx,0);
  grn_ctx_use(ctx, mrn_system_db);
}

void cut_teardown()
{
  grn_ctx_fin(ctx);
  mrn_deinit();
}

mrn_info *gen_mrn_info(grn_ctx *ctx, const char *keyname, const char *dbname, const char *tblname)
{
  mrn_info *info = mrn_init_obj_info(ctx, 2);
  uchar bitmap[1];

  info->name = keyname;

  info->db->name = dbname;
  info->db->name_size = strlen(dbname);
  info->db->path = (char*) dbname;

  info->table->name = tblname;
  info->table->name_size = strlen(tblname);
  info->table->flags |= GRN_OBJ_TABLE_NO_KEY;

  info->columns[0]->name = "c1";
  info->columns[0]->name_size = strlen("c1");
  info->columns[0]->flags |= GRN_OBJ_COLUMN_SCALAR;
  info->columns[0]->type = grn_ctx_at(ctx, GRN_DB_INT32);
  info->columns[0]->gtype = GRN_DB_INT32;
  MRN_SET_BIT(bitmap,0);

  info->columns[1]->name = "c2";
  info->columns[1]->name_size = strlen("c2");
  info->columns[1]->flags |= GRN_OBJ_COLUMN_SCALAR;
  info->columns[1]->type = grn_ctx_at(ctx, GRN_DB_TEXT);
  info->columns[1]->gtype = GRN_DB_TEXT;
  MRN_SET_BIT(bitmap,1);

  return info;
}

void test_mrn_hash_put()
{
  TEST_ENTER;
  const char *key1 = "aa";
  const char *key2 = "bbb";
  const char *key3 = "cccc";
  const char *key4 = "primitive";
  char *value1 = "11ddbb";
  char *value2 = "22fefe2";
  char *value3 = "3333r";
  int value4 = 777;

  cut_assert_equal_int(0,mrn_hash_put(ctx, key1, (void*) value1));
  cut_assert_equal_int(0,mrn_hash_put(ctx, key2, (void*) value2));
  cut_assert_equal_int(0,mrn_hash_put(ctx, key3, (void*) value3));
  cut_assert_equal_int(0,mrn_hash_put(ctx, key4, (void*) &value4));
  cut_assert_equal_int(-1,mrn_hash_put(ctx, key1, (void*) value2));
  cut_assert_equal_int(-1,mrn_hash_put(ctx, key2, (void*) value3));
  cut_assert_equal_int(-1,mrn_hash_put(ctx, key3, (void*) value1));
}

void test_mrn_hash_get()
{
  TEST_ENTER;
  const char *key1 = "aa";
  const char *key2 = "bbb";
  const char *key3 = "cccc";
  const char *key4 = "not found";
  const char *key5 = "primitive";
  char *value1 = "abcdefg";
  char *value2 = "hijklmnopq";
  char *value3 = "rxyz012";
  int value5 = 777;
  char *res;
  int *res_int;

  mrn_hash_put(ctx, key1, (void*) value1);
  mrn_hash_put(ctx, key2, (void*) value2);
  mrn_hash_put(ctx, key3, (void*) value3);
  mrn_hash_put(ctx, key5, (void*) &value5);

  cut_assert_equal_int(0, mrn_hash_get(ctx, key1, (void**) &res));
  cut_assert_equal_string(value1, res);
  cut_assert_equal_int(0, mrn_hash_get(ctx, key2, (void**) &res));
  cut_assert_equal_string(value2, res);
  cut_assert_equal_int(0, mrn_hash_get(ctx, key3, (void**) &res));
  cut_assert_equal_string(value3, res);

  cut_assert_equal_int(-1, mrn_hash_get(ctx, key4, (void**) &res));

  cut_assert_equal_int(0, mrn_hash_get(ctx, key5, (void**) &res_int));
  cut_assert_equal_int(value5, *res_int);
}

void test_mrn_hash_remove()
{
  TEST_ENTER;
  const char *key1 = "aa";
  const char *key2 = "bbb";
  const char *key3 = "cccc";
  const char *key4 = "not found";
  const char *key5 = "primitive";
  char *value1 = "112233";
  char *value2 = "2221115";
  char *value3 = "333344";
  int value5 = 777;

  mrn_hash_put(ctx, key1, (void*) value1);
  mrn_hash_put(ctx, key2, (void*) value2);
  mrn_hash_put(ctx, key3, (void*) value3);
  mrn_hash_put(ctx, key5, (void*) &value5);

  cut_assert_equal_int(-1, mrn_hash_remove(ctx, key4));
  cut_assert_equal_int(0, mrn_hash_remove(ctx, key1));
  cut_assert_equal_int(0, mrn_hash_remove(ctx, key2));
  cut_assert_equal_int(0, mrn_hash_remove(ctx, key3));
  cut_assert_equal_int(0, mrn_hash_remove(ctx, key5));

  cut_assert_equal_int(-1, mrn_hash_remove(ctx, key1));
  cut_assert_equal_int(-1, mrn_hash_remove(ctx, key2));
  cut_assert_equal_int(-1, mrn_hash_remove(ctx, key3));
  cut_assert_equal_int(-1, mrn_hash_remove(ctx, key5));
}

void test_mrn_init_obj_info()
{
  TEST_ENTER;
  uint n_columns = 8, i;
  mrn_info *info;
  info = mrn_init_obj_info(ctx, n_columns);
  cut_assert_not_null(info);
  info->table->name = "mrn_init_obj_info";
  info->table->name_size = strlen(info->table->name);
  cut_assert_true(info->table->flags == GRN_OBJ_PERSISTENT);
  cut_assert_equal_int(n_columns, info->n_columns);
  for (i=0; i < n_columns; i++)
  {
    info->columns[i]->name = "fuga";
    info->columns[i]->name_size = strlen("fuga");
    cut_assert_true(info->columns[i]->flags == GRN_OBJ_PERSISTENT);
  }
  mrn_deinit_obj_info(ctx, info);
}

void test_mrn_deinit_obj_info()
{
  TEST_ENTER;
  mrn_info *info = mrn_init_obj_info(ctx, 8);
  cut_assert_not_null(info);
  cut_assert_equal_int(0, mrn_deinit_obj_info(ctx, info));
}

void test_mrn_create()
{
  TEST_ENTER;
  mrn_info *info =
    gen_mrn_info(ctx, "./mroonga/mrn_create", "mroonga", "mrn_create");

  cut_assert_equal_int(0, mrn_db_open_or_create(ctx, info, obj));

  obj->table = NULL;
  obj->columns = NULL;

  cut_assert_equal_int(0, mrn_create(ctx, info, obj));
  cut_assert_not_null(obj->table);
  cut_assert_not_null(obj->columns[0]);
  cut_assert_not_null(obj->columns[1]);

  cut_assert_equal_int(0, mrn_drop(ctx, "mroonga", "mrn_create"));
  mrn_deinit_obj_info(ctx, info);
}

void test_mrn_open()
{
  TEST_ENTER;
  mrn_info *info =
    gen_mrn_info(ctx, "./mroonga/mrn_open", "mroonga", "mrn_open");

  cut_assert_equal_int(0, mrn_db_open_or_create(ctx, info, obj));
  cut_assert_equal_int(0, mrn_create(ctx, info, obj));
  cut_assert_equal_int(0, mrn_close(ctx, info, obj));

  cut_assert_null(obj->table);
  cut_assert_equal_int(0, mrn_open(ctx, info, obj));
  cut_assert_not_null(obj->table);
  cut_assert_not_null(obj->columns[0]);
  cut_assert_not_null(obj->columns[1]);

  cut_assert_equal_int(0, mrn_drop(ctx, "mroonga", "mrn_open"));
  mrn_deinit_obj_info(ctx, info);
}

void test_mrn_close()
{
  TEST_ENTER;
  mrn_info *info =
    gen_mrn_info(ctx, "./mroonga/mrn_close", "mroonga", "mrn_close");

  cut_assert_equal_int(0, mrn_db_open_or_create(ctx, info, obj));
  cut_assert_equal_int(0, mrn_create(ctx, info, obj));

  cut_assert_not_null(obj->table);
  cut_assert_not_null(obj->columns[0]);
  cut_assert_not_null(obj->columns[1]);
  cut_assert_equal_int(0, mrn_close(ctx, info, obj));
  cut_assert_null(obj->table);

  cut_assert_equal_int(0, mrn_drop(ctx, "mroonga", "mrn_close"));
  mrn_deinit_obj_info(ctx, info);
}

void test_mrn_drop()
{
  TEST_ENTER;
  mrn_info *info =
    gen_mrn_info(ctx, "./mroonga/mrn_drop", "mroonga", "mrn_drop");

  cut_assert_equal_int(0, mrn_db_open_or_create(ctx, info, obj));
  cut_assert_equal_int(0, mrn_create(ctx, info, obj));
  cut_assert_equal_int(0, mrn_drop(ctx, "mroonga", "mrn_drop"));
  cut_assert_equal_int(-1, mrn_open(ctx, info, obj));
  mrn_deinit_obj_info(ctx, info);
}

void test_mrn_bitmap_macro()
{
  TEST_ENTER;
  uchar *a;
  a = g_malloc(128);
  memset(a,0,128);
  int i;
  for (i=0; i < 128*8; i++)
  {
    cut_assert_false(MRN_IS_BIT(a,i));
    MRN_SET_BIT(a,i);
    cut_assert_true(MRN_IS_BIT(a,i));
    MRN_CLEAR_BIT(a,i);
    cut_assert_false(MRN_IS_BIT(a,i));
  }
  g_free(a);
}

void test_get_data_type()
{
  cut_assert_null(grn_ctx_at(ctx, GRN_DB_VOID));
  cut_assert_not_null(grn_ctx_at(ctx, GRN_DB_OBJECT));
  cut_assert_not_null(grn_ctx_at(ctx, GRN_DB_BOOL));
  cut_assert_not_null(grn_ctx_at(ctx, GRN_DB_INT8));
  cut_assert_not_null(grn_ctx_at(ctx, GRN_DB_UINT8));
  cut_assert_not_null(grn_ctx_at(ctx, GRN_DB_INT16));
  cut_assert_not_null(grn_ctx_at(ctx, GRN_DB_UINT16));
  cut_assert_not_null(grn_ctx_at(ctx, GRN_DB_INT32));
  cut_assert_not_null(grn_ctx_at(ctx, GRN_DB_UINT32));
  cut_assert_not_null(grn_ctx_at(ctx, GRN_DB_INT64));
  cut_assert_not_null(grn_ctx_at(ctx, GRN_DB_UINT64));
  cut_assert_not_null(grn_ctx_at(ctx, GRN_DB_FLOAT));
  cut_assert_not_null(grn_ctx_at(ctx, GRN_DB_TIME));
  cut_assert_not_null(grn_ctx_at(ctx, GRN_DB_SHORT_TEXT));
  cut_assert_not_null(grn_ctx_at(ctx, GRN_DB_TEXT));
  cut_assert_not_null(grn_ctx_at(ctx, GRN_DB_LONG_TEXT));
  cut_assert_not_null(grn_ctx_at(ctx, GRN_DB_DELIMIT));
  cut_assert_not_null(grn_ctx_at(ctx, GRN_DB_UNIGRAM));
  cut_assert_not_null(grn_ctx_at(ctx, GRN_DB_BIGRAM));
  cut_assert_not_null(grn_ctx_at(ctx, GRN_DB_TRIGRAM));
  // cut_assert_not_null(grn_ctx_at(ctx, GRN_DB_MECAB));
}
