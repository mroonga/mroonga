#include <string.h>
#include <gcutter.h>
#include <glib/gstdio.h>
#include "driver.h"

#define TEST_ENTER do { GRN_LOG(ctx, GRN_LOG_DEBUG,             \
                                "<%s>", __FUNCTION__);} while(0)

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
  TEST_ENTER;
  cut_assert_equal_int(0, mrn_flush_logs(ctx));
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
  cut_assert_null(info->table->obj);
  cut_assert_equal_int(n_columns, info->n_columns);
  for (i=0; i < n_columns; i++)
  {
    info->columns[i]->name = "fuga";
    info->columns[i]->name_size = 4;
    cut_assert_true(info->columns[i]->flags == GRN_OBJ_PERSISTENT);
    cut_assert_null(info->columns[i]->obj);
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
  grn_obj *obj, *obj2;
  mrn_info *info = mrn_init_obj_info(ctx, 2);

  info->table->name = "test/mrn_create";
  info->table->name_size = strlen("test/mrn_create");
  info->table->flags |= GRN_OBJ_TABLE_NO_KEY;

  info->columns[0]->name = "c1";
  info->columns[0]->name_size = strlen("c1");
  info->columns[0]->flags |= GRN_OBJ_COLUMN_SCALAR;
  info->columns[0]->type = grn_ctx_at(ctx, GRN_DB_INT32);

  info->columns[1]->name = "c2";
  info->columns[1]->name_size = strlen("c2");
  info->columns[1]->flags |= GRN_OBJ_COLUMN_SCALAR;
  info->columns[1]->type = grn_ctx_at(ctx, GRN_DB_TEXT);

  cut_assert_equal_int(0, mrn_create(ctx, info));

  cut_assert_not_null((obj = grn_ctx_get(ctx, "test/mrn_create", strlen("test/mrn_create"))));
  cut_assert_not_null((obj2 = grn_obj_column(ctx, obj, "c1", strlen("c1"))));
  grn_obj_remove(ctx, obj2);
  cut_assert_not_null((obj2 = grn_obj_column(ctx, obj, "c2", strlen("c2"))));
  grn_obj_remove(ctx, obj2);
  cut_assert_null((obj2 = grn_obj_column(ctx, obj, "c3", strlen("c3"))));
  grn_obj_remove(ctx, obj);

  mrn_deinit_obj_info(ctx, info);
}

void test_mrn_open()
{
  TEST_ENTER;
  grn_obj *obj,*obj2;

  mrn_info *info = mrn_init_obj_info(ctx, 2);

  info->table->name = "test/mrn_open";
  info->table->name_size = strlen("test/mrn_open");
  info->table->flags |= GRN_OBJ_TABLE_NO_KEY;

  info->columns[0]->name = "c1";
  info->columns[0]->name_size = strlen("c1");
  info->columns[0]->flags |= GRN_OBJ_COLUMN_SCALAR;
  info->columns[0]->type = grn_ctx_at(ctx, GRN_DB_INT32);

  info->columns[1]->name = "c2";
  info->columns[1]->name_size = strlen("c2");
  info->columns[1]->flags |= GRN_OBJ_COLUMN_SCALAR;
  info->columns[1]->type = grn_ctx_at(ctx, GRN_DB_TEXT);

  cut_assert_equal_int(0, mrn_create(ctx, info));
  cut_assert_null(info->table->obj);
  cut_assert_null(info->columns[0]->obj);
  cut_assert_null(info->columns[1]->obj);
  cut_assert_equal_int(0, mrn_open(ctx, info));
  cut_assert_not_null(info->table->obj);
  cut_assert_not_null(info->columns[0]->obj);
  cut_assert_not_null(info->columns[1]->obj);

  grn_obj_remove(ctx, info->columns[1]->obj);
  grn_obj_remove(ctx, info->columns[0]->obj);
  grn_obj_remove(ctx, info->table->obj);
  mrn_deinit_obj_info(ctx, info);
}

void test_mrn_close()
{
  TEST_ENTER;
  grn_obj *obj,*obj2;

  mrn_info *info = mrn_init_obj_info(ctx, 2);

  info->table->name = "test/mrn_close";
  info->table->name_size = strlen("test/mrn_close");
  info->table->flags |= GRN_OBJ_TABLE_NO_KEY;

  info->columns[0]->name = "c1";
  info->columns[0]->name_size = strlen("c1");
  info->columns[0]->flags |= GRN_OBJ_COLUMN_SCALAR;
  info->columns[0]->type = grn_ctx_at(ctx, GRN_DB_INT32);

  info->columns[1]->name = "c2";
  info->columns[1]->name_size = strlen("c2");
  info->columns[1]->flags |= GRN_OBJ_COLUMN_SCALAR;
  info->columns[1]->type = grn_ctx_at(ctx, GRN_DB_TEXT);

  cut_assert_equal_int(0, mrn_create(ctx, info));
  cut_assert_equal_int(0, mrn_open(ctx, info));
  cut_assert_not_null(info->table->obj);
  cut_assert_not_null(info->columns[0]->obj);
  cut_assert_not_null(info->columns[1]->obj);

  cut_assert_equal_int(0, mrn_close(ctx, info));
  cut_assert_null(info->table->obj);
  cut_assert_null(info->columns[0]->obj);
  cut_assert_null(info->columns[1]->obj);

  cut_assert_equal_int(0, mrn_open(ctx, info));
  grn_obj_remove(ctx, info->columns[1]->obj);
  grn_obj_remove(ctx, info->columns[0]->obj);
  grn_obj_remove(ctx, info->table->obj);
  mrn_deinit_obj_info(ctx, info);
}

void test_mrn_drop()
{
  TEST_ENTER;
  grn_obj *obj, *obj2;
  mrn_info *info = mrn_init_obj_info(ctx, 2);

  info->table->name = "test/mrn_drop";
  info->table->name_size = strlen("test/mrn_drop");
  info->table->flags |= GRN_OBJ_TABLE_NO_KEY;

  info->columns[0]->name = "c1";
  info->columns[0]->name_size = strlen("c1");
  info->columns[0]->flags |= GRN_OBJ_COLUMN_SCALAR;
  info->columns[0]->type = grn_ctx_at(ctx, GRN_DB_INT32);

  info->columns[1]->name = "c2";
  info->columns[1]->name_size = strlen("c2");
  info->columns[1]->flags |= GRN_OBJ_COLUMN_SCALAR;
  info->columns[1]->type = grn_ctx_at(ctx, GRN_DB_TEXT);

  cut_assert_equal_int(0, mrn_create(ctx, info));
  cut_assert_equal_int(0, mrn_open(ctx, info));
  cut_assert_equal_int(0, mrn_close(ctx, info));
  cut_assert_equal_int(0, mrn_drop(ctx, "test/mrn_drop"));
  cut_assert_equal_int(-1, mrn_open(ctx, info));
  mrn_deinit_obj_info(ctx, info);
}

void test_mrn_drop_from_other_ctx()
{
  TEST_ENTER;
  grn_obj *obj, *obj2;
  mrn_info *info = mrn_init_obj_info(ctx, 2);
  grn_ctx ctx2;
  grn_ctx_init(&ctx2,0);
  grn_ctx_use(&ctx2, mrn_db);

  info->table->name = "test/mrn_drop_from_other_ctx";
  info->table->name_size = strlen("test/mrn_drop_from_other_ctx");
  info->table->flags |= GRN_OBJ_TABLE_NO_KEY;

  info->columns[0]->name = "c1";
  info->columns[0]->name_size = strlen("c1");
  info->columns[0]->flags |= GRN_OBJ_COLUMN_SCALAR;
  info->columns[0]->gtype = GRN_DB_INT32;
  info->columns[0]->type = grn_ctx_at(ctx, GRN_DB_INT32);

  info->columns[1]->name = "c2";
  info->columns[1]->name_size = strlen("c2");
  info->columns[1]->flags |= GRN_OBJ_COLUMN_SCALAR;
  info->columns[1]->gtype = GRN_DB_TEXT;
  info->columns[1]->type = grn_ctx_at(ctx, GRN_DB_TEXT);

  cut_assert_equal_int(0, mrn_create(ctx, info));
  cut_assert_equal_int(0, mrn_open(ctx, info));
  cut_assert_equal_int(0, mrn_close(ctx, info));
  cut_assert_equal_int(0, mrn_drop(&ctx2, "test/mrn_drop_from_other_ctx"));
  cut_assert_equal_int(-1, mrn_open(&ctx2, info));
  mrn_deinit_obj_info(ctx, info);
  grn_ctx_fin(&ctx2);
}

void test_mrn_write_row()
{
  TEST_ENTER;
  grn_obj *obj, *obj2;
  mrn_info *info = mrn_init_obj_info(ctx, 2);
  mrn_record *record;

  info->table->name = "test/mrn_write_row";
  info->table->name_size = strlen("test/mrn_write_row");
  info->table->flags |= GRN_OBJ_TABLE_NO_KEY;

  info->columns[0]->name = "c1";
  info->columns[0]->name_size = strlen("c1");
  info->columns[0]->flags |= GRN_OBJ_COLUMN_SCALAR;
  info->columns[0]->type = grn_ctx_at(ctx, GRN_DB_INT32);

  info->columns[1]->name = "c2";
  info->columns[1]->name_size = strlen("c2");
  info->columns[1]->flags |= GRN_OBJ_COLUMN_SCALAR;
  info->columns[1]->type = grn_ctx_at(ctx, GRN_DB_TEXT);

  cut_assert_equal_int(0, mrn_create(ctx, info));
  cut_assert_equal_int(0, mrn_open(ctx, info));

  record = mrn_init_record(ctx, info, NULL);
  cut_assert_not_null(record);
  {
    int val1=100;
    char *val2 = "record value";

    GRN_TEXT_SET(ctx, record->value[0], (char*) &val1, sizeof(val1));
    GRN_TEXT_SET(ctx, record->value[1], val2, strlen(val2));
    cut_assert_equal_int(0, mrn_write_row(ctx, record));
  }
  cut_assert_equal_int(0, mrn_deinit_record(ctx, record));

  cut_assert_equal_int(0, mrn_close(ctx, info));
  cut_assert_equal_int(0, mrn_drop(ctx, "test/mrn_write_row"));
  mrn_deinit_obj_info(ctx, info);
}

void test_mrn_init_record()
{
  TEST_ENTER;
  grn_obj *obj, *obj2;
  mrn_info *info = mrn_init_obj_info(ctx, 2);
  mrn_record *record;

  info->table->name = "test/mrn_init_record";
  info->table->name_size = strlen("test/mrn_init_record");
  info->table->flags |= GRN_OBJ_TABLE_NO_KEY;

  info->columns[0]->name = "c1";
  info->columns[0]->name_size = strlen("c1");
  info->columns[0]->flags |= GRN_OBJ_COLUMN_SCALAR;
  info->columns[0]->gtype = GRN_DB_INT32;
  info->columns[0]->type = grn_ctx_at(ctx, GRN_DB_INT32);

  info->columns[1]->name = "c2";
  info->columns[1]->name_size = strlen("c2");
  info->columns[1]->flags |= GRN_OBJ_COLUMN_SCALAR;
  info->columns[1]->gtype = GRN_DB_TEXT;
  info->columns[1]->type = grn_ctx_at(ctx, GRN_DB_TEXT);

  cut_assert_equal_int(0, mrn_create(ctx, info));
  cut_assert_equal_int(0, mrn_open(ctx, info));

  {
    record = mrn_init_record(ctx, info, NULL);
    cut_assert_not_null(record);
    cut_assert_not_null(record->info);
    cut_assert_not_null(record->value);
    cut_assert_not_null(record->value[0]);
    cut_assert_not_null(record->value[1]);
    cut_assert_equal_int(2, record->n_columns);
    mrn_deinit_record(ctx, record);
  }

  cut_assert_equal_int(0, mrn_close(ctx, info));
  cut_assert_equal_int(0, mrn_drop(ctx, "test/mrn_init_record"));
  mrn_deinit_obj_info(ctx, info);
}

void test_mrn_deinit_record()
{
  TEST_ENTER;
  grn_obj *obj, *obj2;
  mrn_info *info = mrn_init_obj_info(ctx, 2);
  mrn_record *record;

  info->table->name = "test/mrn_deinit_record";
  info->table->name_size = strlen("test/mrn_deinit_record");
  info->table->flags |= GRN_OBJ_TABLE_NO_KEY;

  info->columns[0]->name = "c1";
  info->columns[0]->name_size = strlen("c1");
  info->columns[0]->flags |= GRN_OBJ_COLUMN_SCALAR;
  info->columns[0]->type = grn_ctx_at(ctx, GRN_DB_INT32);

  info->columns[1]->name = "c2";
  info->columns[1]->name_size = strlen("c2");
  info->columns[1]->flags |= GRN_OBJ_COLUMN_SCALAR;
  info->columns[1]->type = grn_ctx_at(ctx, GRN_DB_TEXT);

  cut_assert_equal_int(0, mrn_create(ctx, info));
  cut_assert_equal_int(0, mrn_open(ctx, info));

  record = mrn_init_record(ctx, info, NULL);

  {
    cut_assert_equal_int(0, mrn_deinit_record(ctx, record));
  }

  cut_assert_equal_int(0, mrn_close(ctx, info));
  cut_assert_equal_int(0, mrn_drop(ctx, "test/mrn_deinit_record"));
  mrn_deinit_obj_info(ctx, info);
}

void test_mrn_rnd_init()
{
  TEST_ENTER;
  grn_obj *obj, *obj2;
  mrn_info *info = mrn_init_obj_info(ctx, 2);
  mrn_record *record;

  info->table->name = "test/mrn_rnd_init";
  info->table->name_size = strlen("test/mrn_rnd_init");
  info->table->flags |= GRN_OBJ_TABLE_NO_KEY;

  info->columns[0]->name = "c1";
  info->columns[0]->name_size = strlen("c1");
  info->columns[0]->flags |= GRN_OBJ_COLUMN_SCALAR;
  info->columns[0]->gtype = GRN_DB_INT32;
  info->columns[0]->type = grn_ctx_at(ctx, GRN_DB_INT32);

  info->columns[1]->name = "c2";
  info->columns[1]->name_size = strlen("c2");
  info->columns[1]->flags |= GRN_OBJ_COLUMN_SCALAR;
  info->columns[1]->gtype = GRN_DB_TEXT;
  info->columns[1]->type = grn_ctx_at(ctx, GRN_DB_TEXT);

  cut_assert_equal_int(0, mrn_create(ctx, info));
  cut_assert_equal_int(0, mrn_open(ctx, info));

  record = mrn_init_record(ctx, info, NULL);
  cut_assert_not_null(record);
  int val1=100;
  char *val2 = "record value";
  GRN_TEXT_SET(ctx, record->value[0], (char*) &val1, sizeof(val1));
  GRN_TEXT_SET(ctx, record->value[1], val2, strlen(val2));
  cut_assert_equal_int(0, mrn_write_row(ctx, record));
  cut_assert_equal_int(0, mrn_deinit_record(ctx, record));

  {
    cut_assert_equal_int(0, mrn_rnd_init(ctx, info));
    cut_assert_not_null(info->cursor);
    grn_table_cursor_close(ctx, info->cursor);
  }

  cut_assert_equal_int(0, mrn_close(ctx, info));
  cut_assert_equal_int(0, mrn_drop(ctx, "test/mrn_rnd_init"));
  mrn_deinit_obj_info(ctx, info);
}

void test_mrn_rnd_next()
{
  TEST_ENTER;
  grn_obj *obj, *obj2;
  mrn_info *info = mrn_init_obj_info(ctx, 2);
  mrn_record *record;

  info->table->name = "test/mrn_rnd_next";
  info->table->name_size = strlen("test/mrn_rnd_next");
  info->table->flags |= GRN_OBJ_TABLE_NO_KEY;

  info->columns[0]->name = "c1";
  info->columns[0]->name_size = strlen("c1");
  info->columns[0]->flags |= GRN_OBJ_COLUMN_SCALAR;
  info->columns[0]->gtype = GRN_DB_INT32;
  info->columns[0]->type = grn_ctx_at(ctx, GRN_DB_INT32);

  info->columns[1]->name = "c2";
  info->columns[1]->name_size = strlen("c2");
  info->columns[1]->flags |= GRN_OBJ_COLUMN_SCALAR;
  info->columns[1]->gtype = GRN_DB_TEXT;
  info->columns[1]->type = grn_ctx_at(ctx, GRN_DB_TEXT);

  cut_assert_equal_int(0, mrn_create(ctx, info));
  cut_assert_equal_int(0, mrn_open(ctx, info));

  record = mrn_init_record(ctx, info, NULL);
  cut_assert_not_null(record);
  int val1=100;
  char *val2 = "record value";
  GRN_INT32_SET(ctx, record->value[0], val1);
  cut_assert_equal_int(12, strlen(val2));
  GRN_TEXT_SET(ctx, record->value[1], val2, strlen(val2));
  cut_assert_equal_int(0, mrn_write_row(ctx, record));
  cut_assert_equal_int(0, mrn_deinit_record(ctx, record));

  cut_assert_equal_int(0, mrn_rnd_init(ctx, info));
  cut_assert_not_null(info->cursor);

  {
    mrn_record *record;
    int res1;
    char *res2;
    record = mrn_init_record(ctx, info, NULL);
    cut_assert_equal_int(0, mrn_rnd_next(ctx, record, NULL));
    res1 = GRN_INT32_VALUE(record->value[0]);
    res2 = GRN_TEXT_VALUE(record->value[1]);
    cut_assert_equal_int(100, res1);
    cut_assert_equal_int(12, GRN_BULK_WSIZE(record->value[1]));
    cut_assert_equal_int(0, strncmp(val2, res2, 12));

    cut_assert_equal_int(0, mrn_rewind_record(ctx, record));
    cut_assert_equal_int(1, mrn_rnd_next(ctx, record, NULL));
    cut_assert_null(record->info->cursor);
    cut_assert_equal_int(0, mrn_deinit_record(ctx, record));
  }

  cut_assert_equal_int(0, mrn_close(ctx, info));
  cut_assert_equal_int(0, mrn_drop(ctx, "test/mrn_rnd_next"));
  mrn_deinit_obj_info(ctx, info);
}

void test_mrn_table_size()
{
  TEST_ENTER;
  grn_obj *obj, *obj2;
  mrn_info *info = mrn_init_obj_info(ctx, 2);
  mrn_record *record;

  info->table->name = "test/mrn_table_size";
  info->table->name_size = strlen("test/mrn_table_size");
  info->table->flags |= GRN_OBJ_TABLE_NO_KEY;

  info->columns[0]->name = "c1";
  info->columns[0]->name_size = strlen("c1");
  info->columns[0]->flags |= GRN_OBJ_COLUMN_SCALAR;
  info->columns[0]->gtype = GRN_DB_INT32;
  info->columns[0]->type = grn_ctx_at(ctx, GRN_DB_INT32);

  info->columns[1]->name = "c2";
  info->columns[1]->name_size = strlen("c2");
  info->columns[1]->flags |= GRN_OBJ_COLUMN_SCALAR;
  info->columns[1]->gtype = GRN_DB_TEXT;
  info->columns[1]->type = grn_ctx_at(ctx, GRN_DB_TEXT);

  cut_assert_equal_int(0, mrn_create(ctx, info));
  cut_assert_equal_int(0, mrn_open(ctx, info));

  record = mrn_init_record(ctx, info, NULL);
  cut_assert_not_null(record);
  int val1=100;
  char *val2 = "record value";
  GRN_TEXT_SET(ctx, record->value[0], (char*) &val1, sizeof(val1));
  cut_assert_equal_int(12, strlen(val2));
  GRN_TEXT_SET(ctx, record->value[1], val2, strlen(val2));
  cut_assert_equal_int(0, mrn_write_row(ctx, record));
  cut_assert_equal_int(0, mrn_deinit_record(ctx, record));

  cut_assert_equal_int(0, mrn_rnd_init(ctx, info));
  cut_assert_not_null(info->cursor);

  {
    cut_assert_equal_int(1, mrn_table_size(ctx, info));
  }

  cut_assert_equal_int(0, mrn_close(ctx, info));
  cut_assert_equal_int(0, mrn_drop(ctx, "test/mrn_table_size"));
  mrn_deinit_obj_info(ctx, info);
}

void test_mrn_init_column_list()
{
  TEST_ENTER;
  grn_obj *obj, *obj2;
  mrn_info *info = mrn_init_obj_info(ctx, 4);

  info->table->name = "test/mrn_init_column_list";
  info->table->name_size = strlen("test/mrn_init_column_list");
  info->table->flags |= GRN_OBJ_TABLE_NO_KEY;

  info->columns[0]->name = "c1";
  info->columns[0]->name_size = strlen("c1");
  info->columns[0]->flags |= GRN_OBJ_COLUMN_SCALAR;
  info->columns[0]->gtype = GRN_DB_INT32;
  info->columns[0]->type = grn_ctx_at(ctx, GRN_DB_INT32);

  info->columns[1]->name = "c2";
  info->columns[1]->name_size = strlen("c2");
  info->columns[1]->flags |= GRN_OBJ_COLUMN_SCALAR;
  info->columns[1]->gtype = GRN_DB_TEXT;
  info->columns[1]->type = grn_ctx_at(ctx, GRN_DB_TEXT);

  info->columns[2]->name = "c3";
  info->columns[2]->name_size = strlen("c3");
  info->columns[2]->flags |= GRN_OBJ_COLUMN_SCALAR;
  info->columns[2]->gtype = GRN_DB_INT32;
  info->columns[2]->type = grn_ctx_at(ctx, GRN_DB_INT32);

  info->columns[3]->name = "c4";
  info->columns[3]->name_size = strlen("c4");
  info->columns[3]->flags |= GRN_OBJ_COLUMN_SCALAR;
  info->columns[3]->gtype = GRN_DB_TEXT;
  info->columns[3]->type = grn_ctx_at(ctx, GRN_DB_TEXT);

  {
    int src[] = {1,3,1,3};
    mrn_column_list *list = mrn_init_column_list(ctx, info, src, 4);
    cut_assert_not_null(list);
    cut_assert_equal_int(0, memcmp(info, list->info, sizeof(info)));
    cut_assert_not_null(list->columns);
    cut_assert_equal_int(2, list->actual_size);
    cut_assert_null(list->columns[0]);
    cut_assert_not_null(list->columns[1]);
    cut_assert_equal_string("c2", list->columns[1]->name);
    cut_assert_null(list->columns[2]);
    cut_assert_not_null(list->columns[3]);
    cut_assert_equal_string("c4", list->columns[3]->name);
    mrn_deinit_column_list(ctx, list);

  }

  mrn_deinit_obj_info(ctx, info);
}

void test_mrn_init_record_cond()
{
  TEST_ENTER;
  grn_obj *obj, *obj2;
  mrn_info *info = mrn_init_obj_info(ctx, 4);
  mrn_record *record;

  info->table->name = "test/mrn_init_record_cond";
  info->table->name_size = strlen("test/mrn_init_record_cond");
  info->table->flags |= GRN_OBJ_TABLE_NO_KEY;

  info->columns[0]->name = "c1";
  info->columns[0]->name_size = strlen("c1");
  info->columns[0]->flags |= GRN_OBJ_COLUMN_SCALAR;
  info->columns[0]->gtype = GRN_DB_INT32;
  info->columns[0]->type = grn_ctx_at(ctx, GRN_DB_INT32);

  info->columns[1]->name = "c2";
  info->columns[1]->name_size = strlen("c2");
  info->columns[1]->flags |= GRN_OBJ_COLUMN_SCALAR;
  info->columns[1]->gtype = GRN_DB_INT32;
  info->columns[1]->type = grn_ctx_at(ctx, GRN_DB_INT32);

  info->columns[2]->name = "c3";
  info->columns[2]->name_size = strlen("c3");
  info->columns[2]->flags |= GRN_OBJ_COLUMN_SCALAR;
  info->columns[2]->gtype = GRN_DB_INT32;
  info->columns[2]->type = grn_ctx_at(ctx, GRN_DB_INT32);

  info->columns[3]->name = "c4";
  info->columns[3]->name_size = strlen("c4");
  info->columns[3]->flags |= GRN_OBJ_COLUMN_SCALAR;
  info->columns[3]->gtype = GRN_DB_INT32;
  info->columns[3]->type = grn_ctx_at(ctx, GRN_DB_INT32);

  int src[] = {1,3,3,1,3};
  mrn_column_list *list = mrn_init_column_list(ctx, info, src, 5);

  {
    cut_assert_equal_int(2, list->actual_size);
    record = mrn_init_record(ctx, info, list);
    cut_assert_not_null(record);
    cut_assert_not_null(record->info);
    cut_assert_equal_int(2, record->actual_size);
    cut_assert_equal_int(4, record->n_columns);
    cut_assert_not_null(record->value);
    cut_assert_not_null(record->value[0]);
    cut_assert_not_null(record->value[1]);
    mrn_deinit_record(ctx, record);
  }

  mrn_deinit_column_list(ctx, list);
  mrn_deinit_obj_info(ctx, info);
}

void test_mrn_rnd_next_cond()
{
  TEST_ENTER;
  grn_obj *obj, *obj2;
  mrn_info *info = mrn_init_obj_info(ctx, 4);
  mrn_record *record;

  info->table->name = "test/mrn_rnd_next_cond";
  info->table->name_size = strlen("test/mrn_rnd_next_cond");
  info->table->flags |= GRN_OBJ_TABLE_NO_KEY;

  info->columns[0]->name = "c1";
  info->columns[0]->name_size = strlen("c1");
  info->columns[0]->flags |= GRN_OBJ_COLUMN_SCALAR;
  info->columns[0]->gtype = GRN_DB_INT32;
  info->columns[0]->type = grn_ctx_at(ctx, GRN_DB_INT32);

  info->columns[1]->name = "c2";
  info->columns[1]->name_size = strlen("c2");
  info->columns[1]->flags |= GRN_OBJ_COLUMN_SCALAR;
  info->columns[1]->gtype = GRN_DB_INT32;
  info->columns[1]->type = grn_ctx_at(ctx, GRN_DB_INT32);

  info->columns[2]->name = "c3";
  info->columns[2]->name_size = strlen("c3");
  info->columns[2]->flags |= GRN_OBJ_COLUMN_SCALAR;
  info->columns[2]->gtype = GRN_DB_INT32;
  info->columns[2]->type = grn_ctx_at(ctx, GRN_DB_INT32);

  info->columns[3]->name = "c4";
  info->columns[3]->name_size = strlen("c4");
  info->columns[3]->flags |= GRN_OBJ_COLUMN_SCALAR;
  info->columns[3]->gtype = GRN_DB_INT32;
  info->columns[3]->type = grn_ctx_at(ctx, GRN_DB_INT32);

  cut_assert_equal_int(0, mrn_create(ctx, info));
  cut_assert_equal_int(0, mrn_open(ctx, info));

  record = mrn_init_record(ctx, info, NULL);
  cut_assert_not_null(record);
  GRN_INT32_SET(ctx, record->value[0], 100);
  GRN_INT32_SET(ctx, record->value[1], 101);
  GRN_INT32_SET(ctx, record->value[2], 102);
  GRN_INT32_SET(ctx, record->value[3], 103);
  cut_assert_equal_int(0, mrn_write_row(ctx, record));

  cut_assert_equal_int(0, mrn_rewind_record(ctx, record));

  GRN_INT32_SET(ctx, record->value[0], 200);
  GRN_INT32_SET(ctx, record->value[1], 201);
  GRN_INT32_SET(ctx, record->value[2], 202);
  GRN_INT32_SET(ctx, record->value[3], 203);
  cut_assert_equal_int(0, mrn_write_row(ctx, record));

  mrn_deinit_record(ctx, record);


  int src[] = {1,3,3,1,3};
  mrn_column_list *list = mrn_init_column_list(ctx, info, src, 5);

  cut_assert_equal_int(0, mrn_rnd_init(ctx, info));
  cut_assert_not_null(info->cursor);

  {
    mrn_record *record;
    record = mrn_init_record(ctx, info, list);
    cut_assert_equal_int(0, mrn_rnd_next(ctx, record, list));
    cut_assert_equal_int(101, GRN_INT32_VALUE(record->value[0]));
    cut_assert_equal_int(103, GRN_INT32_VALUE(record->value[1]));

    cut_assert_equal_int(0, mrn_rewind_record(ctx, record));
    cut_assert_equal_int(0, mrn_rnd_next(ctx, record, list));
    cut_assert_equal_int(201, GRN_INT32_VALUE(record->value[0]));
    cut_assert_equal_int(203, GRN_INT32_VALUE(record->value[1]));

    cut_assert_equal_int(0, mrn_rewind_record(ctx, record));
    cut_assert_equal_int(1, mrn_rnd_next(ctx, record, list));
    cut_assert_null(record->info->cursor);
    cut_assert_equal_int(0, mrn_deinit_record(ctx, record));
  }

  cut_assert_equal_int(0, mrn_close(ctx, info));
  cut_assert_equal_int(0, mrn_drop(ctx, "test/mrn_rnd_next_cond"));
  mrn_deinit_obj_info(ctx, info);
}
