#include <string.h>
#include <gcutter.h>
#include <glib/gstdio.h>
#include <groonga.h>

static gchar *base_directory;
static gchar *tmp_directory;
static grn_ctx *ctx;
static grn_obj *db, *table, *col_int, *col_text;

void cut_startup()
{
  base_directory = g_get_current_dir();
  tmp_directory = g_build_filename(base_directory, "/tmp", NULL);
  cut_remove_path(tmp_directory, NULL);
  g_mkdir_with_parents(tmp_directory, 0700);
  g_chdir(tmp_directory);
  grn_init();
  ctx = (grn_ctx*) malloc(sizeof(grn_ctx));
  grn_ctx_init(ctx,0);
  db = grn_db_create(ctx,"groonga.db", NULL);
  grn_ctx_use(ctx, db);
}

void cut_shutdown()
{
  grn_obj_remove(ctx, db);
  free(ctx);
  grn_fin();
  g_chdir(base_directory);
  g_free(tmp_directory);
}

void cut_setup()
{
  table = grn_table_create(ctx, "test/t1", 7, NULL,
                           GRN_OBJ_PERSISTENT | GRN_OBJ_TABLE_NO_KEY,
                           NULL, 0);
  col_int = grn_column_create(ctx, table, "col_int", 7, NULL,
                              GRN_OBJ_PERSISTENT | GRN_OBJ_COLUMN_SCALAR,
                              grn_ctx_at(ctx, GRN_DB_INT32));
  col_text = grn_column_create(ctx, table, "col_text", 8, NULL,
                               GRN_OBJ_PERSISTENT | GRN_OBJ_COLUMN_SCALAR,
                               grn_ctx_at(ctx, GRN_DB_TEXT));
  int i,added;
  grn_obj obj_int, obj_text;
  GRN_INT32_INIT(&obj_int,0);
  GRN_TEXT_INIT(&obj_text,0);
  for (i=0; i < 10; i++)
  {
    char buf[32];
    grn_id id = grn_table_add(ctx, table, NULL, 0, &added);
    GRN_BULK_REWIND(&obj_int);
    GRN_BULK_REWIND(&obj_text);
    GRN_INT32_SET(ctx, &obj_int, i*100);
    grn_obj_set_value(ctx, col_int, id, &obj_int, GRN_OBJ_SET);
    snprintf(buf, 32, "text:%d", i*100);
    GRN_TEXT_SETS(ctx, &obj_text, buf);
    grn_obj_set_value(ctx, col_text, id, &obj_text, GRN_OBJ_SET);
  }
  grn_obj_close(ctx, &obj_int);
  grn_obj_close(ctx, &obj_text);
}

void cut_teardown()
{
  grn_obj_remove(ctx, table);
}


void test_sample_expression()
{
  grn_obj *expr, buf, *res;
  grn_obj *v, *r;
  expr = grn_expr_create(ctx, NULL, 0);
  cut_assert_not_null(expr);
  v = grn_expr_add_var(ctx, expr, NULL, 0);
  GRN_RECORD_INIT(v, 0, grn_obj_id(ctx, table));
  grn_expr_append_obj(ctx, expr, v);

  GRN_TEXT_INIT(&buf,0);
  GRN_TEXT_SETS(ctx, &buf, "col_int");
  grn_expr_append_const(ctx, expr, &buf);
  grn_expr_append_op(ctx, expr, GRN_OP_OBJ_GET_VALUE, 2);

  grn_expr_compile(ctx, expr);
  res = grn_expr_exec(ctx, expr);
  grn_expr_close(ctx, expr);
}

