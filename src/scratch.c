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

void sample_dump()
{
  grn_init();
  const char *db_path = "/usr/local/mysql/data/mroonga.grn";
  grn_ctx ctx;
  grn_ctx_init(&ctx,0,GRN_ENC_UTF8);

  grn_obj *db = grn_db_open(&ctx, db_path);
  grn_ctx_use(&ctx, db);

  grn_obj *obj=NULL;
  grn_id i=0;
  do {
    obj = grn_ctx_get(&ctx, ++i);
    printf("grn_id=%d, obj=%p",i, obj);
    char name[1024];
    int len;
    memset(name,0,1024);
    len = grn_obj_name(&ctx, obj, name, 1024);
    printf(" ,name=%s, ret_len=%d",name ,len);
    printf("\n");
  } while (obj != NULL);
  grn_fin();
}


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

void sample_write_row()
{
  const char *dbname="sample_db";
  const char *tblname="sample_tbl";
  const char *colname="sample_col";


  grn_init();

  grn_ctx ctx;
  grn_ctx_init(&ctx,0,0);

  grn_obj *db = grn_db_create(&ctx,dbname,NULL);
  grn_ctx_use(&ctx,db);
  grn_obj_flags flags = GRN_OBJ_PERSISTENT|GRN_OBJ_TABLE_PAT_KEY;
  grn_obj *grn_int = grn_ctx_get(&ctx, GRN_DB_INT);
  uint val_size = sizeof(int);
  grn_obj *tbl = grn_table_create(&ctx, tblname, strlen(tblname),
				  tblname, flags, grn_int, val_size, GRN_ENC_UTF8);
  grn_obj *col = grn_column_create(&ctx, tbl, colname, strlen(colname), colname,
		    GRN_OBJ_PERSISTENT, grn_int);

  int key1=10;
  int key2=20;
  int key3=30;
  int val1=100;
  int val2=200;
  int val3=300;
  grn_search_flags sflags = GRN_TABLE_ADD;
  grn_id gid;
  grn_rc rc;
  grn_obj new;


  gid = grn_table_lookup(&ctx,tbl,&key1,sizeof(key1), &sflags);
  GRN_OBJ_INIT(&new, GRN_BULK, GRN_OBJ_DO_SHALLOW_COPY);
  GRN_BULK_SET(&ctx, &new, (char*) &val1, sizeof(val1));
  rc = grn_obj_set_value(&ctx,tbl,gid,&new,GRN_OBJ_SET);
  printf("gid=%d, rc=%d, val=%d\n",gid,rc,val1);

  gid = grn_table_lookup(&ctx,tbl,&key2,sizeof(key2), &sflags);
  GRN_OBJ_INIT(&new, GRN_BULK, GRN_OBJ_DO_SHALLOW_COPY);
  GRN_BULK_SET(&ctx, &new, (char*) &val2, sizeof(val2));
  rc = grn_obj_set_value(&ctx,tbl,gid,&new,GRN_OBJ_SET);
  printf("gid=%d, rc=%d, val=%d\n",gid,rc,val2);

  gid = grn_table_lookup(&ctx,tbl,&key3,sizeof(key3), &sflags);
  GRN_OBJ_INIT(&new, GRN_BULK, GRN_OBJ_DO_SHALLOW_COPY);
  GRN_BULK_SET(&ctx, &new, (char*) &val3, sizeof(val3));
  rc = grn_obj_set_value(&ctx,tbl,gid,&new,GRN_OBJ_SET);
  printf("gid=%d, rc=%d, val=%d\n",gid,rc,val2);

  grn_table_cursor *cursor = grn_table_cursor_open(&ctx,tbl,NULL,0,NULL,0,0);

  grn_obj buf;
  GRN_OBJ_INIT(&buf, GRN_BULK, 0);
  while((gid = grn_table_cursor_next(&ctx,cursor)) != GRN_ID_NIL) {
    int *key;
    GRN_BULK_REWIND(&buf);
    int key_len = grn_table_cursor_get_key(&ctx, cursor, (void**) &key);
    int *r;
    int val_len = grn_table_cursor_get_value(&ctx, cursor, (void**) &r);
    grn_obj_get_value(&ctx, tbl, gid, &buf);

    int *p = (int*) GRN_BULK_HEAD(&buf);

    printf("cursor gid=%d, *key=%d,key_len=%d, r=%d, p=%d\n",
	   gid,*key,key_len, *r,*p);
  }

  grn_fin();
}

void sample_no_key()
{
  grn_init();
  grn_ctx *ctx = (grn_ctx*) malloc(sizeof(grn_ctx));

  grn_obj *db = grn_db_create(ctx,"sample_db",NULL);

  grn_obj *grn_int = grn_ctx_get(ctx, GRN_DB_INT);

  /*
  {
    grn_obj *tbl = grn_table_create(ctx, "sample_tbl1", 11, "sample_tbl1",
				    GRN_OBJ_PERSISTENT|GRN_OBJ_TABLE_PAT_KEY,
				    grn_int, 4, GRN_ENC_UTF8);
    printf("tbl is %p\n",tbl);
    grn_obj *col = grn_column_create(ctx, tbl, "sample_col1", 11, "sample_col1",
				     GRN_OBJ_PERSISTENT, grn_int);

    int key1=100;
    int key2=200;
    grn_search_flags col_flags = GRN_TABLE_ADD;
    grn_id id;
    id = grn_table_lookup(ctx, tbl, (void*) &key1, sizeof(int), &col_flags);
    printf("id=%d\n",id);
    id = grn_table_lookup(ctx, tbl, (void*) &key2, sizeof(int), &col_flags);
    printf("id=%d\n",id);

  }
  {
    grn_obj *tbl = grn_table_create(ctx, "sample_tbl2", 11, "sample_tbl2",
				    GRN_OBJ_PERSISTENT|GRN_OBJ_TABLE_HASH_KEY,
				    grn_int, 4, GRN_ENC_UTF8);
    printf("tbl is %p\n",tbl);
    grn_obj *col = grn_column_create(ctx, tbl, "sample_col2", 11, "sample_col2",
				     GRN_OBJ_PERSISTENT, grn_int);


    int key1=100;
    int key2=200;
    grn_search_flags col_flags = GRN_TABLE_ADD;
    grn_id id;
    id = grn_table_lookup(ctx, tbl, (void*) &key1, sizeof(int), &col_flags);
    printf("id=%d\n",id);
    id = grn_table_lookup(ctx, tbl, (void*) &key2, sizeof(int), &col_flags);
    printf("id=%d\n",id);
  }
  */
  {
    grn_obj *tbl = grn_table_create(ctx, "sample_tbl3", 11, "sample_tbl3",
				    GRN_OBJ_PERSISTENT|GRN_OBJ_TABLE_NO_KEY,
				    grn_int, 4, GRN_ENC_UTF8);
    printf("tbl is %p\n",tbl);
    grn_obj *col = grn_column_create(ctx, tbl, "sample_col3", 11, "sample_col3",
				     GRN_OBJ_PERSISTENT, grn_int);

    grn_id id;
    id = grn_table_add(ctx, tbl);
    printf("id=%d\n",id);
    id = grn_table_add(ctx, tbl);
    printf("id=%d\n",id);

    grn_obj_close(ctx, tbl);

    tbl = grn_table_open(ctx, "sample_tbl3", 11, "sample_tbl3");
    printf("reopen =%p\n", tbl);
  }
  grn_fin();
  free(ctx);
}

int main(int argc, char **argv)
{
  sample_no_key();
  return 0;
}
