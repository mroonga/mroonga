#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <groonga.h>
#include "mroonga.h"
#include <sys/stat.h>

grn_ctx *mrn_ctx_sys;
grn_hash *mrn_hash_sys;

typedef struct st_mrn_tbl {
  char *name;
  int name_len;
  grn_obj *obj;
} mrn_tbl;

typedef struct st_mrn_db {
  char *name;
  int name_len;
  grn_hash *hash;
  grn_obj *obj;
} mrn_db;

void sample_hash()
{
  /* libgroonga init */
  grn_init();

  /* ctx init */
  mrn_ctx_sys = (grn_ctx*) MRN_MALLOC(sizeof(grn_ctx));
  grn_ctx_init(mrn_ctx_sys, 0, GRN_ENC_UTF8);

  /* hash init */
  mrn_hash_sys = grn_hash_create(mrn_ctx_sys,NULL,MRN_MAX_KEY_LEN,sizeof(size_t),
				 GRN_OBJ_KEY_VAR_SIZE, GRN_ENC_UTF8);

  grn_search_flags flags = GRN_TABLE_ADD;
  mrn_db *mdb1,*mdb2;
  mrn_tbl *mtbl1, *mtbl2;
  grn_obj db1,db2,tbl1,tbl2;
  GRN_OBJ_INIT(&db1,GRN_BULK,0);
  GRN_OBJ_INIT(&db2,GRN_BULK,0);
  GRN_OBJ_INIT(&tbl1,GRN_BULK,0);
  GRN_OBJ_INIT(&tbl2,GRN_BULK,0);

  mdb1 = malloc(sizeof(mrn_db));
  mdb2 = malloc(sizeof(mrn_db));
  mtbl1 = malloc(sizeof(mrn_tbl));
  mtbl2 = malloc(sizeof(mrn_tbl));

  /* scenario 1 : create table db1.tbl1 */

  /* search by "db1" but not opened */
  void *dummy;
  grn_search_flags _flags = 0;
  grn_id rid = grn_hash_lookup(mrn_ctx_sys,mrn_hash_sys,"db1",3,&dummy,&_flags);
  printf("rid = %d\n", rid);

  /* init mdb1 */
  mdb1->name = "db1";
  mdb1->obj = &db1;
  mdb1->hash = grn_hash_create(mrn_ctx_sys,NULL,MRN_MAX_KEY_LEN,sizeof(size_t),
			     GRN_OBJ_KEY_VAR_SIZE,GRN_ENC_UTF8);
  /* put mdb1  */
  void *val1;
  grn_hash_lookup(mrn_ctx_sys,mrn_hash_sys,mdb1->name,strlen(mdb1->name),&val1,&flags);
  memcpy(val1,mdb1,sizeof(mdb1));

  /* init mdb2 */
  mdb2->name = "db2";
  mdb2->obj = &db2;
  mdb2->hash = grn_hash_create(mrn_ctx_sys,NULL,MRN_MAX_KEY_LEN,sizeof(size_t),
			     GRN_OBJ_KEY_VAR_SIZE,GRN_ENC_UTF8);

  /* put mdb2 */
  void *val2;
  grn_hash_lookup(mrn_ctx_sys,mrn_hash_sys,mdb2->name,strlen(mdb2->name),&val2,&flags);
  memcpy(val2,mdb2,sizeof(mdb2));

  /* get mdb1 */
  void *res1;
  grn_hash_lookup(mrn_ctx_sys,mrn_hash_sys,mdb1->name,strlen(mdb1->name),&res1,&flags);
  mrn_db *foo1 = (mrn_db*) res1;
  printf("lookup name = %s\n", foo1->name);

  /* get mdb2 */
  void *res2;
  grn_hash_lookup(mrn_ctx_sys,mrn_hash_sys,mdb2->name,strlen(mdb2->name),&res2,&flags);
  mrn_db *foo2 = (mrn_db*) res2;
  printf("lookup name = %s\n", foo2->name);

}


void sample_table_create_drop()
{
  const char *t1 = "_t1";
  const char *t2 = "_t2";

  unlink(t1);
  unlink(t2);

 /* libgroonga init */
  grn_init();

  /* ctx init */
  mrn_ctx_sys = (grn_ctx*) MRN_MALLOC(sizeof(grn_ctx));
  grn_ctx_init(mrn_ctx_sys, 0, GRN_ENC_UTF8);
  
  grn_obj *gdb = grn_db_create(mrn_ctx_sys, NULL, NULL);
  grn_ctx_use(mrn_ctx_sys,gdb);

  grn_obj *key_type = grn_ctx_get(mrn_ctx_sys,GRN_DB_SHORTTEXT);
  grn_obj *gtbl = grn_table_create(mrn_ctx_sys, t1, 3,
				   t1, GRN_OBJ_PERSISTENT|GRN_OBJ_TABLE_HASH_KEY,
				   key_type,1000,GRN_ENC_UTF8);


  const char *rpath = grn_obj_path(mrn_ctx_sys, gtbl);
  grn_obj_remove(mrn_ctx_sys,gtbl);

  
  grn_obj *key_type2 = grn_ctx_get(mrn_ctx_sys,GRN_DB_SHORTTEXT);
  grn_obj *gtbl2 = grn_table_create(mrn_ctx_sys, t1, 3,
				   t2, GRN_OBJ_PERSISTENT|GRN_OBJ_TABLE_HASH_KEY,
				   key_type2,1000,GRN_ENC_UTF8);

}
/*
void sample_macro_name_split()
{
  char db[128];
  char table[128];
  MRN_NAME_SPLIT("./hoge/fuga",db,table);
  printf("[%s] , [%s]\n",db,table);
  MRN_NAME_SPLIT("./baaaaaab/ceeeeeeeec",db,table);
  printf("[%s] , [%s]\n",db,table);
  MRN_NAME_SPLIT("./v/x",db,table);
  printf("[%s] , [%s]\n",db,table);
  MRN_NAME_SPLIT("./b77777b/c00000000c",db,table);
  printf("[%s] , [%s]\n",db,table);
}
*/
void sample_string()
{
  const char *name = "./test/t1";
  const char *obj_name = name + 2;
  const char *rt = obj_name - 2;
  printf("name=%s, obj_name=%s, rt=%s\n",name,obj_name,rt);
}
/*
void sample_dump()
{
  const char *db_path = "hoge.grn";
  const char *tbl_path = "fuga.grn";
  const char *tbl_name = "hoge/fuga";

  unlink(db_path);
  unlink(tbl_path);

  grn_init();
  grn_ctx ctx = GRN_CTX_INITIALIZER;

  grn_obj *db = grn_db_create(&ctx, db_path, NULL);
  //grn_obj *db = grn_db_open(&ctx, db_path);
  grn_ctx_use(&ctx, db);

  grn_obj *key_type = grn_ctx_get(&ctx, GRN_DB_SHORTTEXT);
  grn_obj *obj = grn_table_create(&ctx, tbl_name, strlen(tbl_name), tbl_path,
				    GRN_OBJ_PERSISTENT|GRN_OBJ_TABLE_HASH_KEY,
				    key_type,1000,GRN_ENC_UTF8);

  grn_id i;
  for (i=0; i < 20; i++) {
    grn_obj *obj = grn_ctx_get(&ctx, i);
    printf("grn_id=%d, obj=%p",i, obj);
    if (obj) {
      char name[1024];
      int len;
      memset(name,0,1024);
      len = grn_obj_name(&ctx, obj, name, 1024);
      printf(" ,name=%s, len=%d, ret_len=%d",name, strlen(name),len);
    }
    printf("\n");
  }
  grn_fin();
}
*/

int
sample_open_or_create(int argc, char **argv)
{
  const char *db_path = "hoge.grn";
  const char *tbl_path = "fuga.grn";
  const char *tbl_name = "hoge/fuga";
  grn_obj *db;
  grn_ctx ctx;
  grn_obj *obj;

  unlink(db_path);
  unlink(tbl_path);

  grn_init();


  int flg=0;

  struct stat dummy;

 foo:
  grn_ctx_init(&ctx, 0, 0);

  if ((stat(db_path, &dummy))) { // check if file not exists
    printf("creating objects...\n");
    db = grn_db_create(&ctx, db_path, NULL);
    grn_ctx_use(&ctx, db);

    grn_obj *key_type = grn_ctx_get(&ctx, GRN_DB_SHORTTEXT);
    obj = grn_table_create(&ctx, tbl_name, strlen(tbl_name), tbl_path,
				  GRN_OBJ_PERSISTENT|GRN_OBJ_TABLE_HASH_KEY,
				  key_type,1000,GRN_ENC_UTF8);
  } else {
    printf("opening objects...\n");
    db = grn_db_open(&ctx, db_path);
    grn_ctx_use(&ctx, db);
    obj = grn_table_open(&ctx, tbl_name, strlen(tbl_name), tbl_path);
  }

  grn_id i;
  for (i=0; i < 20; i++) {
    grn_obj *obj = grn_ctx_get(&ctx, i);
    printf("grn_id=%d, obj=%p",i, obj);
    if (obj) {
      char name[1024];
      int len;
      memset(name,0,1024);
      len = grn_obj_name(&ctx, obj, name, 1024);
      printf(" ,name=%s, len=%d, ret_len=%d",name, strlen(name),len);
    }
    printf("\n");
  }
  grn_obj_close(&ctx,obj);
  grn_obj_close(&ctx,db);

  if (flg==0) {
    flg++;
    goto foo;
  }

  grn_fin();
  return 0;
}

int main(int argc, char **argv)
{
  sample_open_or_create(argc, argv);
  return 0;
}
