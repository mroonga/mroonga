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
  for (i=1; i <= 10; i++)
  {
    char buf[32];
    grn_id id = grn_table_add(ctx, table, NULL, 0, &added);
    GRN_BULK_REWIND(&obj_int);
    GRN_BULK_REWIND(&obj_text);
    GRN_INT32_SET(ctx, &obj_int, i);
    grn_obj_set_value(ctx, col_int, id, &obj_int, GRN_OBJ_SET);
    snprintf(buf, 32, "text:%d", i);
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
  grn_obj *expr, intbuf, textbuf, *res;
  grn_obj *v, *r;
  grn_table_cursor *tc;
  grn_id res_id, *rec_id;

  GRN_INT32_INIT(&intbuf,0);
  GRN_TEXT_INIT(&textbuf,0);

  expr = grn_expr_create(ctx, NULL, 0);
  cut_assert_not_null(expr);
  v = grn_expr_add_var(ctx, expr, NULL, 0);
  GRN_RECORD_INIT(v, 0, grn_obj_id(ctx, table));

  {
    {
      grn_expr_append_obj(ctx, expr, v);
      GRN_BULK_REWIND(&textbuf);
      GRN_TEXT_SETS(ctx, &textbuf, "col_int");
      grn_expr_append_const(ctx, expr, &textbuf);
      grn_expr_append_op(ctx, expr, GRN_OP_OBJ_GET_VALUE, 2);
      GRN_BULK_REWIND(&intbuf);
      GRN_INT32_SET(ctx, &intbuf, 4);
      grn_expr_append_const(ctx, expr, &intbuf);
      grn_expr_append_op(ctx, expr, GRN_OP_GREATER, 2);
    }
    {
      grn_expr_append_obj(ctx, expr, v);
      GRN_BULK_REWIND(&textbuf);
      GRN_TEXT_SETS(ctx, &textbuf, "col_int");
      grn_expr_append_const(ctx, expr, &textbuf);
      grn_expr_append_op(ctx, expr, GRN_OP_OBJ_GET_VALUE, 2);
      GRN_BULK_REWIND(&intbuf);
      GRN_INT32_SET(ctx, &intbuf, 8);
      grn_expr_append_const(ctx, expr, &intbuf);
      grn_expr_append_op(ctx, expr, GRN_OP_LESS, 2);
    }
    grn_expr_append_op(ctx, expr, GRN_OP_AND, 2);
  }

  res = grn_table_create(ctx, NULL, 0, NULL,
                         GRN_TABLE_HASH_KEY|GRN_OBJ_WITH_SUBREC, table, NULL);

  grn_rc rc = grn_table_select(ctx, table, expr, res, GRN_OP_OR);
  cut_assert_equal_int(0, rc);
  cut_assert_equal_int(3, grn_table_size(ctx, res));

  tc = grn_table_cursor_open(ctx, res, NULL, 0, NULL, 0, 0, 0, 0);
  int i;
  for (i=5; i<=7; i++)
  {
    char buf[32];
    res_id = grn_table_cursor_next(ctx, tc);
    grn_table_cursor_get_key(ctx, tc, (void**) &rec_id);

    GRN_BULK_REWIND(&intbuf);
    GRN_BULK_REWIND(&textbuf);
    cut_assert_equal_int(i, *rec_id);
    cut_assert_not_null(grn_obj_get_value(ctx, col_int, *rec_id, &intbuf));
    cut_assert_equal_int(i, GRN_INT32_VALUE(&intbuf));
    cut_assert_not_null(grn_obj_get_value(ctx, col_text, *rec_id, &textbuf));
    snprintf(buf, 32, "text:%d", i);
    cut_assert_true(memcmp(buf, GRN_TEXT_VALUE(&textbuf), sizeof(buf)));
  }
  grn_expr_close(ctx, expr);
  grn_table_cursor_close(ctx, tc);
}
