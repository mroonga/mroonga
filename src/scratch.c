#include <stdio.h>
#include <string.h>
#include <groonga.h>
#include "mroonga.h"

grn_ctx *mrn_ctx_sys;
grn_hash *mrn_hash_sys;

typedef struct st_mrn_db {
  char *name;
  int name_len;
  grn_obj *obj;
  grn_hash *hash;
} mrn_db;

typedef struct st_mrn_tbl {
  char *name;
  int name_len;
  grn_obj *obj;
} mrn_tbl;

void sample_hash()
{
  /* libgroonga init */
  grn_init();

  /* ctx init */
  mrn_ctx_sys = (grn_ctx*) MRN_MALLOC(sizeof(grn_ctx));
  grn_ctx_init(mrn_ctx_sys, GRN_CTX_USE_DB, GRN_ENC_UTF8);

  /* hash init */
  mrn_hash_sys = grn_hash_create(mrn_ctx_sys,NULL,MRN_MAX_IDENTIFIER_LEN,sizeof(size_t),
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
  mdb1->name_len = 3;
  mdb1->obj = &db1;
  mdb1->hash = grn_hash_create(mrn_ctx_sys,NULL,MRN_MAX_IDENTIFIER_LEN,sizeof(size_t),
			     GRN_OBJ_KEY_VAR_SIZE,GRN_ENC_UTF8);
  /* put mdb1  */
  void *val1;
  grn_hash_lookup(mrn_ctx_sys,mrn_hash_sys,mdb1->name,mdb1->name_len,&val1,&flags);
  memcpy(val1,mdb1,sizeof(mdb1));

  /* init mdb2 */
  mdb2->name = "db2";
  mdb2->name_len = 3;
  mdb2->obj = &db2;
  mdb2->hash = grn_hash_create(mrn_ctx_sys,NULL,MRN_MAX_IDENTIFIER_LEN,sizeof(size_t),
			     GRN_OBJ_KEY_VAR_SIZE,GRN_ENC_UTF8);

  /* put mdb2 */
  void *val2;
  grn_hash_lookup(mrn_ctx_sys,mrn_hash_sys,mdb2->name,mdb2->name_len,&val2,&flags);
  memcpy(val2,mdb2,sizeof(mdb2));

  /* get mdb1 */
  void *res1;
  grn_hash_lookup(mrn_ctx_sys,mrn_hash_sys,mdb1->name,mdb1->name_len,&res1,&flags);
  mrn_db *foo1 = (mrn_db*) res1;
  printf("lookup name = %s\n", foo1->name);

  /* get mdb2 */
  void *res2;
  grn_hash_lookup(mrn_ctx_sys,mrn_hash_sys,mdb2->name,mdb2->name_len,&res2,&flags);
  mrn_db *foo2 = (mrn_db*) res2;
  printf("lookup name = %s\n", foo2->name);

}


int main()
{
  sample_hash();
}
