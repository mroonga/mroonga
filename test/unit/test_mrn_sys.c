/*
  Copyright(C) 2010 Tetsuro IKEDA
  Copyright(C) 2011 Kouhei Sutou <kou@clear-code.com>

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

grn_ctx *ctx;
grn_obj *db;

void cut_startup()
{
  ctx = (grn_ctx*) malloc(sizeof(grn_ctx));
  grn_init();
  grn_ctx_init(ctx,0);
  db = grn_db_create(ctx, NULL,NULL);
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
}

void cut_teardown()
{
}

void test_mrn_hash_put()
{
  grn_hash *hash = grn_hash_create(ctx,NULL,1024,sizeof(size_t),
                                   GRN_OBJ_KEY_VAR_SIZE);

  const char *key1 = "aa";
  const char *key2 = "bbb";
  const char *key3 = "cccc";
  const char *key4 = "primitive";
  char *value1 = "11ddbb";
  char *value2 = "22fefe2";
  char *value3 = "3333r";
  int value4 = 777;

  cut_assert_equal_int(0,mrn_hash_put(ctx, hash, key1, (void*) value1));
  cut_assert_equal_int(0,mrn_hash_put(ctx, hash, key2, (void*) value2));
  cut_assert_equal_int(0,mrn_hash_put(ctx, hash, key3, (void*) value3));
  cut_assert_equal_int(0,mrn_hash_put(ctx, hash, key4, (void*) &value4));
  cut_assert_equal_int(-1,mrn_hash_put(ctx, hash, key1, (void*) value2));
  cut_assert_equal_int(-1,mrn_hash_put(ctx, hash, key2, (void*) value3));
  cut_assert_equal_int(-1,mrn_hash_put(ctx, hash, key3, (void*) value1));

  grn_hash_close(ctx, hash);
}

void test_mrn_hash_get()
{ 
  grn_hash *hash = grn_hash_create(ctx,NULL,1024,sizeof(size_t),
                                   GRN_OBJ_KEY_VAR_SIZE);
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

  mrn_hash_put(ctx, hash, key1, (void*) value1);
  mrn_hash_put(ctx, hash, key2, (void*) value2);
  mrn_hash_put(ctx, hash, key3, (void*) value3);
  mrn_hash_put(ctx, hash, key5, (void*) &value5);

  cut_assert_equal_int(0, mrn_hash_get(ctx, hash, key1, (void**) &res));
  cut_assert_equal_string(value1, res);
  cut_assert_equal_int(0, mrn_hash_get(ctx, hash, key2, (void**) &res));
  cut_assert_equal_string(value2, res);
  cut_assert_equal_int(0, mrn_hash_get(ctx, hash, key3, (void**) &res));
  cut_assert_equal_string(value3, res);

  cut_assert_equal_int(-1, mrn_hash_get(ctx, hash, key4, (void**) &res));

  cut_assert_equal_int(0, mrn_hash_get(ctx, hash, key5, (void**) &res_int));
  cut_assert_equal_int(value5, *res_int);

  grn_hash_close(ctx, hash);
}

void test_mrn_hash_remove()
{
  grn_hash *hash = grn_hash_create(ctx,NULL,1024,sizeof(size_t),
                                   GRN_OBJ_KEY_VAR_SIZE);
  const char *key1 = "aa";
  const char *key2 = "bbb";
  const char *key3 = "cccc";
  const char *key4 = "not found";
  const char *key5 = "primitive";
  char *value1 = "112233";
  char *value2 = "2221115";
  char *value3 = "333344";
  int value5 = 777;

  mrn_hash_put(ctx, hash, key1, (void*) value1);
  mrn_hash_put(ctx, hash, key2, (void*) value2);
  mrn_hash_put(ctx, hash, key3, (void*) value3);
  mrn_hash_put(ctx, hash, key5, (void*) &value5);

  cut_assert_equal_int(-1, mrn_hash_remove(ctx, hash, key4));
  cut_assert_equal_int(0, mrn_hash_remove(ctx, hash, key1));
  cut_assert_equal_int(0, mrn_hash_remove(ctx, hash, key2));
  cut_assert_equal_int(0, mrn_hash_remove(ctx, hash, key3));
  cut_assert_equal_int(0, mrn_hash_remove(ctx, hash, key5));

  cut_assert_equal_int(-1, mrn_hash_remove(ctx, hash, key1));
  cut_assert_equal_int(-1, mrn_hash_remove(ctx, hash, key2));
  cut_assert_equal_int(-1, mrn_hash_remove(ctx, hash, key3));
  cut_assert_equal_int(-1, mrn_hash_remove(ctx, hash, key5));

  grn_hash_close(ctx, hash);
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
  cut_assert_equal_string("users_name",
                          mrn_index_table_name_gen(buf, "name", buf2));
}
