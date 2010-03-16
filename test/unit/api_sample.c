#include <string.h>
#include <gcutter.h>
#include <glib/gstdio.h>
#include <groonga.h>

static gchar *base_directory;
static gchar *tmp_directory;
static grn_ctx *ctx;
static grn_obj *db;
static grn_obj *table, *col_int, *col_text;
static grn_obj *table2, *col_int2, *col_text2;
FILE *sample_logfile = NULL;

void sample_logger_func(int level, const char *time, const char *title,
			const char *msg, const char *location, void *func_arg)
{
  const char slev[] = " EACewnid-";
  if ((sample_logfile)) {
    fprintf(sample_logfile, "%s|%c|%u|%s\n", time,
            *(slev + level), (uint)pthread_self(), msg);
    fflush(sample_logfile);
  }
}

grn_logger_info sample_logger_info = {
  GRN_LOG_DUMP,
  GRN_LOG_TIME|GRN_LOG_MESSAGE,
  sample_logger_func,
  NULL
};


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
  sample_logfile = fopen("sample.log", "a");
  grn_logger_info_set(ctx, &sample_logger_info);
  db = grn_db_create(ctx,"groonga.db", NULL);
  grn_ctx_use(ctx, db);
}

void cut_shutdown()
{
  grn_obj_remove(ctx, db);
  free(ctx);
  grn_fin();
  fclose(sample_logfile);
  g_chdir(base_directory);
  g_free(tmp_directory);
}

void create_table()
{
  table = grn_table_create(ctx, "t1", strlen("t1"), NULL,
                           GRN_OBJ_PERSISTENT | GRN_OBJ_TABLE_NO_KEY,
                           NULL, 0);
  col_int = grn_column_create(ctx, table, "col_int", strlen("col_int"), NULL,
                              GRN_OBJ_PERSISTENT | GRN_OBJ_COLUMN_SCALAR,
                              grn_ctx_at(ctx, GRN_DB_INT32));
  col_text = grn_column_create(ctx, table, "col_text", strlen("col_text"), NULL,
                               GRN_OBJ_PERSISTENT | GRN_OBJ_COLUMN_SCALAR,
                               grn_ctx_at(ctx, GRN_DB_TEXT));
}

void create_table2()
{
  table2 = grn_table_create(ctx, "t2", strlen("t2"), NULL,
                            GRN_OBJ_PERSISTENT | GRN_OBJ_TABLE_NO_KEY,
                            NULL, 0);
  col_int2 = grn_column_create(ctx, table2, "col_int2", strlen("col_int2"), NULL,
                               GRN_OBJ_PERSISTENT | GRN_OBJ_COLUMN_SCALAR,
                               grn_ctx_at(ctx, GRN_DB_INT32));
  col_text2 = grn_column_create(ctx, table2, "col_text2", strlen("col_text2"), NULL,
                               GRN_OBJ_PERSISTENT | GRN_OBJ_COLUMN_SCALAR,
                                grn_ctx_at(ctx, GRN_DB_TEXT));
}

void insert_data()
{
  int i,added;
  grn_obj obj_int, obj_text;
  GRN_INT32_INIT(&obj_int,0);
  GRN_TEXT_INIT(&obj_text,0);
  for (i=1; i <= 100; i++)
  {
    char buf[32];
    grn_id id = grn_table_add(ctx, table, NULL, 0, &added);
    GRN_BULK_REWIND(&obj_int);
    GRN_BULK_REWIND(&obj_text);
    GRN_INT32_SET(ctx, &obj_int, i*10);
    grn_obj_set_value(ctx, col_int, id, &obj_int, GRN_OBJ_SET);
    snprintf(buf, 32, "text:%d", i);
    GRN_TEXT_SETS(ctx, &obj_text, buf);
    grn_obj_set_value(ctx, col_text, id, &obj_text, GRN_OBJ_SET);
  }
  grn_obj_close(ctx, &obj_int);
  grn_obj_close(ctx, &obj_text);
}

void insert_data2()
{
  int i,added;
  grn_obj obj_int, obj_text;
  GRN_INT32_INIT(&obj_int,0);
  GRN_TEXT_INIT(&obj_text,0);
  for (i=1; i <= 100; i++)
  {
    char buf[32];
    grn_id id = grn_table_add(ctx, table2, NULL, 0, &added);
    GRN_BULK_REWIND(&obj_int);
    GRN_BULK_REWIND(&obj_text);
    GRN_INT32_SET(ctx, &obj_int, i*10);
    grn_obj_set_value(ctx, col_int2, id, &obj_int, GRN_OBJ_SET);
    snprintf(buf, 32, "text:%d", i);
    GRN_TEXT_SETS(ctx, &obj_text, buf);
    grn_obj_set_value(ctx, col_text2, id, &obj_text, GRN_OBJ_SET);
  }
  grn_obj_close(ctx, &obj_int);
  grn_obj_close(ctx, &obj_text);
}

void drop_table()
{
  grn_obj_remove(ctx, col_int);
  grn_obj_remove(ctx, col_text);
  grn_obj_remove(ctx, table);
}

void drop_table2()
{
  grn_obj_remove(ctx, col_int2);
  grn_obj_remove(ctx, col_text2);
  grn_obj_remove(ctx, table2);
}

void test_sample_expression()
{
  grn_obj *expr, intbuf, textbuf, *res;
  grn_obj *v, *r;
  grn_table_cursor *tc;
  grn_id res_id, *rec_id;

  create_table();
  insert_data();

  GRN_INT32_INIT(&intbuf,0);
  GRN_TEXT_INIT(&textbuf,0);

  expr = grn_expr_create(ctx, NULL, 0);
  cut_assert_not_null(expr);
  v = grn_expr_add_var(ctx, expr, NULL, 0);
  GRN_RECORD_INIT(v, 0, grn_obj_id(ctx, table));

  {
    {
      grn_expr_append_obj(ctx, expr, v, GRN_OP_PUSH, 1);
      GRN_BULK_REWIND(&textbuf);
      GRN_TEXT_SETS(ctx, &textbuf, "col_int");
      grn_expr_append_const(ctx, expr, &textbuf, GRN_OP_PUSH, 1);
      grn_expr_append_op(ctx, expr, GRN_OP_GET_VALUE, 2);
      GRN_BULK_REWIND(&intbuf);
      GRN_INT32_SET(ctx, &intbuf, 40);
      grn_expr_append_const(ctx, expr, &intbuf, GRN_OP_PUSH, 1);
      grn_expr_append_op(ctx, expr, GRN_OP_GREATER, 2);
    }
    {
      grn_expr_append_obj(ctx, expr, v, GRN_OP_PUSH, 1);
      GRN_BULK_REWIND(&textbuf);
      GRN_TEXT_SETS(ctx, &textbuf, "col_int");
      grn_expr_append_const(ctx, expr, &textbuf, GRN_OP_PUSH, 1);
      grn_expr_append_op(ctx, expr, GRN_OP_GET_VALUE, 2);
      GRN_BULK_REWIND(&intbuf);
      GRN_INT32_SET(ctx, &intbuf, 80);
      grn_expr_append_const(ctx, expr, &intbuf, GRN_OP_PUSH, 1);
      grn_expr_append_op(ctx, expr, GRN_OP_LESS, 2);
    }
    grn_expr_append_op(ctx, expr, GRN_OP_AND, 2);
  }

  res = grn_table_create(ctx, NULL, 0, NULL,
                         GRN_TABLE_HASH_KEY|GRN_OBJ_WITH_SUBREC, table, NULL);

  grn_obj *res2 = grn_table_select(ctx, table, expr, res, GRN_OP_OR);
  cut_assert_not_null(res);
  cut_assert_equal_int(3, grn_table_size(ctx, res));

  tc = grn_table_cursor_open(ctx, res, NULL, 0, NULL, 0, 0, -1, 0);
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
    cut_assert_equal_int(i*10, GRN_INT32_VALUE(&intbuf));
    cut_assert_not_null(grn_obj_get_value(ctx, col_text, *rec_id, &textbuf));
    snprintf(buf, 32, "text:%d", i);
    cut_assert_true(memcmp(buf, GRN_TEXT_VALUE(&textbuf), sizeof(buf)));
  }
  grn_expr_close(ctx, expr);
  grn_table_cursor_close(ctx, tc);
  grn_obj_close(ctx, res2);
  drop_table();
}

void test_secondary_index_pat_int()
{
  grn_obj *index_table, *index_col, buf, buf2, *res;
  grn_table_cursor *tc;
  grn_id id, docid;

  create_table();

  GRN_INT32_INIT(&buf,0);
  GRN_TEXT_INIT(&buf2,0);

  index_table = grn_table_create(ctx, "pat",  strlen("pat"), NULL,
                               GRN_OBJ_TABLE_PAT_KEY|GRN_OBJ_PERSISTENT,
                               grn_ctx_at(ctx, GRN_DB_INT32), 0);
  index_col = grn_column_create(ctx, index_table, "col", strlen("col"), NULL,
                                GRN_OBJ_COLUMN_INDEX|GRN_OBJ_PERSISTENT,
                                grn_ctx_at(ctx, GRN_DB_INT32));


  GRN_INT32_SET(ctx, &buf, grn_obj_id(ctx, col_int));
  grn_obj_set_info(ctx, index_col, GRN_INFO_SOURCE, &buf);

  insert_data();

  res = grn_table_create(ctx, NULL, 0, NULL,
                         GRN_TABLE_HASH_KEY|GRN_OBJ_WITH_SUBREC, table, 0);

  GRN_BULK_REWIND(&buf);
  GRN_INT32_SET(ctx, &buf, 500);
  grn_obj_search(ctx, index_col, &buf, res, GRN_OP_OR, NULL);
  cut_assert_equal_int(1, grn_table_size(ctx, res));

  tc = grn_table_cursor_open(ctx, res, NULL, 0, NULL, 0, 0, 0, 0);
  while ((id = grn_table_cursor_next(ctx, tc)))
  {
    GRN_BULK_REWIND(&buf);
    GRN_BULK_REWIND(&buf2);
    grn_table_get_key(ctx, res, id, &docid, sizeof(docid));
    cut_assert_equal_int(50, docid);
    cut_assert_not_null(grn_obj_get_value(ctx, col_int, docid, &buf));
    cut_assert_not_null(grn_obj_get_value(ctx, col_text, docid, &buf2));
    cut_assert_equal_int(50*10, GRN_INT32_VALUE(&buf));
    cut_assert_equal_int(7, GRN_TEXT_LEN(&buf2));
    cut_assert_equal_substring("text:50", GRN_TEXT_VALUE(&buf2),
                               GRN_TEXT_LEN(&buf2));
  }
  grn_obj_remove(ctx, index_table);
  drop_table();
}

void test_secondary_index_hash_int()
{
  grn_obj *index_table, *index_col, buf, buf2, *res;
  grn_table_cursor *tc;
  grn_id id, docid;

  create_table();

  GRN_INT32_INIT(&buf,0);
  GRN_TEXT_INIT(&buf2,0);

  index_table = grn_table_create(ctx, "hash",  4, NULL,
                               GRN_OBJ_TABLE_HASH_KEY|GRN_OBJ_PERSISTENT,
                               grn_ctx_at(ctx, GRN_DB_INT32), 0);
  index_col = grn_column_create(ctx, index_table, "col", 3, NULL,
                                GRN_OBJ_COLUMN_INDEX|GRN_OBJ_PERSISTENT,
                                grn_ctx_at(ctx, GRN_DB_INT32));


  GRN_INT32_SET(ctx, &buf, grn_obj_id(ctx, col_int));
  grn_obj_set_info(ctx, index_col, GRN_INFO_SOURCE, &buf);

  insert_data();

  res = grn_table_create(ctx, NULL, 0, NULL,
                         GRN_TABLE_HASH_KEY|GRN_OBJ_WITH_SUBREC, table, 0);

  GRN_BULK_REWIND(&buf);
  GRN_INT32_SET(ctx, &buf, 500);
  grn_obj_search(ctx, index_col, &buf, res, GRN_OP_OR, NULL);
  cut_assert_equal_int(1, grn_table_size(ctx, res));

  tc = grn_table_cursor_open(ctx, res, NULL, 0, NULL, 0, 0, 0, 0);
  while ((id = grn_table_cursor_next(ctx, tc)))
  {
    GRN_BULK_REWIND(&buf);
    GRN_BULK_REWIND(&buf2);
    grn_table_get_key(ctx, res, id, &docid, sizeof(docid));
    cut_assert_equal_int(50, docid);
    cut_assert_not_null(grn_obj_get_value(ctx, col_int, docid, &buf));
    cut_assert_not_null(grn_obj_get_value(ctx, col_text, docid, &buf2));
    cut_assert_equal_int(50*10, GRN_INT32_VALUE(&buf));
    cut_assert_equal_int(7, GRN_TEXT_LEN(&buf2));
    cut_assert_equal_substring("text:50", GRN_TEXT_VALUE(&buf2),
                               GRN_TEXT_LEN(&buf2));
  }

  grn_obj_remove(ctx, index_table);
  drop_table();
}
 
void test_secondary_index_pat_text()
{
  grn_obj *index_table, *index_col, buf, buf2, *res;
  grn_table_cursor *tc;
  grn_id id, docid;

  create_table();

  GRN_INT32_INIT(&buf,0);
  GRN_TEXT_INIT(&buf2,0);

  index_table = grn_table_create(ctx, "pat",  3, NULL,
                               GRN_OBJ_TABLE_PAT_KEY|GRN_OBJ_PERSISTENT,
                               grn_ctx_at(ctx, GRN_DB_INT32), 0);
  index_col = grn_column_create(ctx, index_table, "col", 3, NULL,
                                GRN_OBJ_COLUMN_INDEX|GRN_OBJ_PERSISTENT,
                                grn_ctx_at(ctx, GRN_DB_TEXT));


  GRN_INT32_SET(ctx, &buf, grn_obj_id(ctx, col_text));
  grn_obj_set_info(ctx, index_col, GRN_INFO_SOURCE, &buf);

  insert_data();

  res = grn_table_create(ctx, NULL, 0, NULL,
                         GRN_TABLE_HASH_KEY|GRN_OBJ_WITH_SUBREC, table, 0);

  GRN_BULK_REWIND(&buf2);
  GRN_TEXT_SETS(ctx, &buf2, "text:20");
  grn_obj_search(ctx, index_col, &buf2, res, GRN_OP_OR, NULL);
  cut_assert_equal_int(1, grn_table_size(ctx, res));

  tc = grn_table_cursor_open(ctx, res, NULL, 0, NULL, 0, 0, 0, 0);
  while ((id = grn_table_cursor_next(ctx, tc)))
  {
    GRN_BULK_REWIND(&buf);
    GRN_BULK_REWIND(&buf2);
    grn_table_get_key(ctx, res, id, &docid, sizeof(docid));
    cut_assert_equal_int(20, docid);
    cut_assert_not_null(grn_obj_get_value(ctx, col_int, docid, &buf));
    cut_assert_not_null(grn_obj_get_value(ctx, col_text, docid, &buf2));
    cut_assert_equal_int(20*10, GRN_INT32_VALUE(&buf));
    cut_assert_equal_int(7, GRN_TEXT_LEN(&buf2));
    cut_assert_equal_substring("text:20", GRN_TEXT_VALUE(&buf2),
                               GRN_TEXT_LEN(&buf2));
  }

  grn_obj_remove(ctx, index_table);
  drop_table();
}

void test_secondary_index_hash_text()
{
  grn_obj *index_table, *index_col, buf, buf2, *res;
  grn_table_cursor *tc;
  grn_id id, docid;

  create_table();

  GRN_INT32_INIT(&buf,0);
  GRN_TEXT_INIT(&buf2,0);

  index_table = grn_table_create(ctx, "hash",  4, NULL,
                               GRN_OBJ_TABLE_HASH_KEY|GRN_OBJ_PERSISTENT,
                               grn_ctx_at(ctx, GRN_DB_INT32), 0);
  index_col = grn_column_create(ctx, index_table, "col", 3, NULL,
                                GRN_OBJ_COLUMN_INDEX|GRN_OBJ_PERSISTENT,
                                grn_ctx_at(ctx, GRN_DB_TEXT));


  GRN_INT32_SET(ctx, &buf, grn_obj_id(ctx, col_text));
  grn_obj_set_info(ctx, index_col, GRN_INFO_SOURCE, &buf);

  insert_data();

  res = grn_table_create(ctx, NULL, 0, NULL,
                         GRN_TABLE_HASH_KEY|GRN_OBJ_WITH_SUBREC, table, 0);

  GRN_BULK_REWIND(&buf2);
  GRN_TEXT_SETS(ctx, &buf2, "text:20");
  grn_obj_search(ctx, index_col, &buf2, res, GRN_OP_OR, NULL);
  cut_assert_equal_int(1, grn_table_size(ctx, res));

  tc = grn_table_cursor_open(ctx, res, NULL, 0, NULL, 0, 0, 0, 0);
  while ((id = grn_table_cursor_next(ctx, tc)))
  {
    GRN_BULK_REWIND(&buf);
    GRN_BULK_REWIND(&buf2);
    grn_table_get_key(ctx, res, id, &docid, sizeof(docid));
    cut_assert_equal_int(20, docid);
    cut_assert_not_null(grn_obj_get_value(ctx, col_int, docid, &buf));
    cut_assert_not_null(grn_obj_get_value(ctx, col_text, docid, &buf2));
    cut_assert_equal_int(20*10, GRN_INT32_VALUE(&buf));
    cut_assert_equal_int(7, GRN_TEXT_LEN(&buf2));
    cut_assert_equal_substring("text:20", GRN_TEXT_VALUE(&buf2),
                               GRN_TEXT_LEN(&buf2));
  }

  grn_obj_remove(ctx, index_table);
  drop_table();
}


void test_secondary_index_pat_multi()
{
  grn_obj *index_table, buf, buf2, *res;
  grn_obj *index_int, *index_int2, *index_text, *index_text2;
  grn_table_cursor *tc;
  grn_id id, docid;

  create_table();
  create_table2();

  GRN_INT32_INIT(&buf,0);
  GRN_TEXT_INIT(&buf2,0);

  index_table = grn_table_create(ctx, "pat",  3, NULL,
                               GRN_OBJ_TABLE_PAT_KEY|GRN_OBJ_PERSISTENT,
                               grn_ctx_at(ctx, GRN_DB_INT32), 0);
  index_int = grn_column_create(ctx, index_table, "col_int", 7, NULL,
                                GRN_OBJ_COLUMN_INDEX|GRN_OBJ_PERSISTENT,
                                grn_ctx_at(ctx, GRN_DB_INT32));
  index_text = grn_column_create(ctx, index_table, "col_text", 8, NULL,
                                 GRN_OBJ_COLUMN_INDEX|GRN_OBJ_PERSISTENT,
                                 grn_ctx_at(ctx, GRN_DB_TEXT));
  index_int2 = grn_column_create(ctx, index_table, "col_int2", 8, NULL,
                                 GRN_OBJ_COLUMN_INDEX|GRN_OBJ_PERSISTENT,
                                 grn_ctx_at(ctx, GRN_DB_INT32));
  index_text2 = grn_column_create(ctx, index_table, "col_text2", 9, NULL,
                                  GRN_OBJ_COLUMN_INDEX|GRN_OBJ_PERSISTENT,
                                  grn_ctx_at(ctx, GRN_DB_TEXT));

  GRN_INT32_SET(ctx, &buf, grn_obj_id(ctx, col_int));
  grn_obj_set_info(ctx, index_int, GRN_INFO_SOURCE, &buf);
  GRN_BULK_REWIND(&buf);

  GRN_INT32_SET(ctx, &buf, grn_obj_id(ctx, col_text));
  grn_obj_set_info(ctx, index_text, GRN_INFO_SOURCE, &buf);
  GRN_BULK_REWIND(&buf);

  GRN_INT32_SET(ctx, &buf, grn_obj_id(ctx, col_int2));
  grn_obj_set_info(ctx, index_int2, GRN_INFO_SOURCE, &buf);
  GRN_BULK_REWIND(&buf);

  GRN_INT32_SET(ctx, &buf, grn_obj_id(ctx, col_text2));
  grn_obj_set_info(ctx, index_text2, GRN_INFO_SOURCE, &buf);
  GRN_BULK_REWIND(&buf);

  insert_data();
  insert_data2();

  {
    res = grn_table_create(ctx, NULL, 0, NULL,
                           GRN_TABLE_HASH_KEY|GRN_OBJ_WITH_SUBREC, table, 0);

    GRN_BULK_REWIND(&buf);
    GRN_INT32_SET(ctx, &buf, 500);
    grn_obj_search(ctx, index_int, &buf, res, GRN_OP_OR, NULL);
    cut_assert_equal_int(1, grn_table_size(ctx, res));

    tc = grn_table_cursor_open(ctx, res, NULL, 0, NULL, 0, 0, 0, 0);
    while ((id = grn_table_cursor_next(ctx, tc)))
    {
      GRN_BULK_REWIND(&buf);
      GRN_BULK_REWIND(&buf2);
      grn_table_get_key(ctx, res, id, &docid, sizeof(docid));
      cut_assert_equal_int(50, docid);
      cut_assert_not_null(grn_obj_get_value(ctx, col_int, docid, &buf));
      cut_assert_not_null(grn_obj_get_value(ctx, col_text, docid, &buf2));
      cut_assert_equal_int(50*10, GRN_INT32_VALUE(&buf));
      cut_assert_equal_int(7, GRN_TEXT_LEN(&buf2));
      cut_assert_equal_substring("text:50", GRN_TEXT_VALUE(&buf2),
                                 GRN_TEXT_LEN(&buf2));
    }
    grn_table_cursor_close(ctx, tc);
    grn_obj_close(ctx, res);
  }

  {
    res = grn_table_create(ctx, NULL, 0, NULL,
                           GRN_TABLE_HASH_KEY|GRN_OBJ_WITH_SUBREC, table, 0);

    GRN_BULK_REWIND(&buf);
    GRN_TEXT_SETS(ctx, &buf, "text:30");
    grn_obj_search(ctx, index_text, &buf, res, GRN_OP_OR, NULL);
    cut_assert_equal_int(1, grn_table_size(ctx, res));

    tc = grn_table_cursor_open(ctx, res, NULL, 0, NULL, 0, 0, 0, 0);
    while ((id = grn_table_cursor_next(ctx, tc)))
    {
      GRN_BULK_REWIND(&buf);
      GRN_BULK_REWIND(&buf2);
      grn_table_get_key(ctx, res, id, &docid, sizeof(docid));
      cut_assert_equal_int(30, docid);
      cut_assert_not_null(grn_obj_get_value(ctx, col_int, docid, &buf));
      cut_assert_not_null(grn_obj_get_value(ctx, col_text, docid, &buf2));
      cut_assert_equal_int(30*10, GRN_INT32_VALUE(&buf));
      cut_assert_equal_int(7, GRN_TEXT_LEN(&buf2));
      cut_assert_equal_substring("text:30", GRN_TEXT_VALUE(&buf2),
                                 GRN_TEXT_LEN(&buf2));
    }
    grn_table_cursor_close(ctx, tc);
    grn_obj_close(ctx, res);
  }

  {
    res = grn_table_create(ctx, NULL, 0, NULL,
                           GRN_TABLE_HASH_KEY|GRN_OBJ_WITH_SUBREC, table2, 0);

    GRN_BULK_REWIND(&buf);
    GRN_INT32_SET(ctx, &buf, 700);
    grn_obj_search(ctx, index_int2, &buf, res, GRN_OP_OR, NULL);
    cut_assert_equal_int(1, grn_table_size(ctx, res));

    tc = grn_table_cursor_open(ctx, res, NULL, 0, NULL, 0, 0, 0, 0);
    while ((id = grn_table_cursor_next(ctx, tc)))
    {
      GRN_BULK_REWIND(&buf);
      GRN_BULK_REWIND(&buf2);
      grn_table_get_key(ctx, res, id, &docid, sizeof(docid));
      cut_assert_equal_int(70, docid);
      cut_assert_not_null(grn_obj_get_value(ctx, col_int2, docid, &buf));
      cut_assert_not_null(grn_obj_get_value(ctx, col_text2, docid, &buf2));
      cut_assert_equal_int(70*10, GRN_INT32_VALUE(&buf));
      cut_assert_equal_int(7, GRN_TEXT_LEN(&buf2));
      cut_assert_equal_substring("text:70", GRN_TEXT_VALUE(&buf2),
                                 GRN_TEXT_LEN(&buf2));
    }
    grn_table_cursor_close(ctx, tc);
    grn_obj_close(ctx, res);
  }

  {
    res = grn_table_create(ctx, NULL, 0, NULL,
                           GRN_TABLE_HASH_KEY|GRN_OBJ_WITH_SUBREC, table2, 0);

    GRN_BULK_REWIND(&buf);
    GRN_TEXT_SETS(ctx, &buf, "text:10");
    grn_obj_search(ctx, index_text2, &buf, res, GRN_OP_OR, NULL);
    cut_assert_equal_int(1, grn_table_size(ctx, res));

    tc = grn_table_cursor_open(ctx, res, NULL, 0, NULL, 0, 0, 0, 0);
    while ((id = grn_table_cursor_next(ctx, tc)))
    {
      GRN_BULK_REWIND(&buf);
      GRN_BULK_REWIND(&buf2);
      grn_table_get_key(ctx, res, id, &docid, sizeof(docid));
      cut_assert_equal_int(10, docid);
      cut_assert_not_null(grn_obj_get_value(ctx, col_int, docid, &buf));
      cut_assert_not_null(grn_obj_get_value(ctx, col_text, docid, &buf2));
      cut_assert_equal_int(10*10, GRN_INT32_VALUE(&buf));
      cut_assert_equal_int(7, GRN_TEXT_LEN(&buf2));
      cut_assert_equal_substring("text:10", GRN_TEXT_VALUE(&buf2),
                                 GRN_TEXT_LEN(&buf2));
    }
    grn_table_cursor_close(ctx, tc);
    grn_obj_close(ctx, res);
  }

  grn_obj_remove(ctx, index_int);
  grn_obj_remove(ctx, index_text);
  grn_obj_remove(ctx, index_int2);
  grn_obj_remove(ctx, index_text2);
  grn_obj_remove(ctx, index_table);
  drop_table();
  drop_table2();
}
