/* -*- c-basic-offset: 2 -*- */
/*
  Copyright(C) 2010 Tetsuro IKEDA
  Copyright(C) 2011-2012 Kouhei Sutou <kou@clear-code.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <string.h>
#include <gcutter.h>
#include <glib/gstdio.h>
#include "mrn_sys.h"

static grn_ctx *ctx;
static grn_obj *db;
static grn_hash *hash;
static grn_obj buffer;

void cut_startup()
{
  ctx = (grn_ctx *)malloc(sizeof(grn_ctx));
  grn_init();
  grn_ctx_init(ctx, 0);
  db = grn_db_create(ctx, NULL, NULL);
  grn_ctx_use(ctx, db);
}

void cut_shutdown()
{
  grn_obj_unlink(ctx, db);
  grn_ctx_fin(ctx);
  grn_fin();
  free(ctx);
}

void cut_setup()
{
  hash = grn_hash_create(ctx, NULL, 1024, sizeof(grn_obj *),
                         GRN_OBJ_KEY_VAR_SIZE);
  GRN_TEXT_INIT(&buffer, 0);
}

void cut_teardown()
{
  grn_hash_close(ctx, hash);
  grn_obj_unlink(ctx, &buffer);
}

void test_mrn_hash_put()
{
  const char *key = "mroonga";

  cut_assert_equal_int(0, mrn_hash_put(ctx, hash, key, &buffer));
  cut_assert_equal_int(-1, mrn_hash_put(ctx, hash, key, &buffer));
}

void test_mrn_hash_get()
{
  const char *key = "mroonga";
  const char *value = "A storage engine based on groonga.";
  grn_obj *result;

  GRN_TEXT_SETS(ctx, &buffer, value);
  GRN_TEXT_PUT(ctx, &buffer, "\0", 1);

  mrn_hash_put(ctx, hash, key, &buffer);
  cut_assert_equal_int(0, mrn_hash_get(ctx, hash, key, &result));
  cut_assert_equal_string(value, GRN_TEXT_VALUE(&buffer));
}

void test_mrn_hash_remove()
{
  const char *key = "mroonga";

  mrn_hash_put(ctx, hash, key, &buffer);

  cut_assert_equal_int(-1, mrn_hash_remove(ctx, hash, "nonexistent"));
  cut_assert_equal_int(0, mrn_hash_remove(ctx, hash, key));
  cut_assert_equal_int(-1, mrn_hash_remove(ctx, hash, key));
}

void test_mrn_db_path_gen()
{
  char buf[64];
  const char *arg1 = "./hoge/fuga";
  const char *arg2 = "./foobar/mysql";
  const char *arg3 = "./d/b";
  cut_assert_equal_string("hoge.mrn", mrn_db_path_gen(arg1, buf));
  cut_assert_equal_string("foobar.mrn", mrn_db_path_gen(arg2, buf));
  cut_assert_equal_string("d.mrn", mrn_db_path_gen(arg3, buf));

  const char *arg4 = "./hoge/";
  const char *arg5 = "./foobar/";
  const char *arg6 = "./d/";
  cut_assert_equal_string("hoge.mrn", mrn_db_path_gen(arg4, buf));
  cut_assert_equal_string("foobar.mrn", mrn_db_path_gen(arg5, buf));
  cut_assert_equal_string("d.mrn", mrn_db_path_gen(arg6, buf));
}

void test_mrn_db_name_gen()
{
  char buf[64];
  const char *arg1 = "./hoge/fuga";
  const char *arg2 = "./foobar/mysql";
  const char *arg3 = "./d/b";
  cut_assert_equal_string("hoge", mrn_db_name_gen(arg1, buf));
  cut_assert_equal_string("foobar", mrn_db_name_gen(arg2, buf));
  cut_assert_equal_string("d", mrn_db_name_gen(arg3, buf));

  const char *arg4 = "./hoge/";
  const char *arg5 = "./foobar/";
  const char *arg6 = "./d/";
  cut_assert_equal_string("hoge", mrn_db_name_gen(arg4, buf));
  cut_assert_equal_string("foobar", mrn_db_name_gen(arg5, buf));
  cut_assert_equal_string("d", mrn_db_name_gen(arg6, buf));
}

void test_mrn_table_name_gen()
{
  char buf[64];
  const char *arg1 = "./hoge/fuga";
  const char *arg2 = "./foobar/mysql";
  const char *arg3 = "./d/b";
  cut_assert_equal_string("fuga", mrn_table_name_gen(arg1, buf));
  cut_assert_equal_string("mysql", mrn_table_name_gen(arg2, buf));
  cut_assert_equal_string("b", mrn_table_name_gen(arg3, buf));
}

void test_mrn_index_table_name_gen()
{
  char buf[64], buf2[64];
  const char *arg = "./db/users";
  mrn_table_name_gen(arg, buf);
  cut_assert_equal_string("users-name",
                          mrn_index_table_name_gen(buf, "name", buf2));
}
