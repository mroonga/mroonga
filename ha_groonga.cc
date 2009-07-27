#ifdef USE_PRAGMA_IMPLEMENTATION
#pragma implementation
#endif

#include <mysql_priv.h>
#include <mysql/plugin.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "ha_groonga.h"

MRN_CHARSET_MAP mrn_charset_map[] = {
  {"utf8", GRN_ENC_UTF8},
  {"cp932", GRN_ENC_SJIS},
  {"sjis", GRN_ENC_SJIS},
  {"eucjpms", GRN_ENC_EUC_JP},
  {"ujis", GRN_ENC_EUC_JP},
  {"latin1", GRN_ENC_LATIN1},
  {"koi8r", GRN_ENC_KOI8R},
  {0x0, GRN_ENC_DEFAULT},
  {0x0, GRN_ENC_NONE}
};

grn_encoding mrn_charset_mysql_groonga(const char *csname)
{
  if (!csname) return GRN_ENC_NONE;
  int i;
  for (i = 0; mrn_charset_map[i].csname_mysql; i++) {
    if (!(strcasecmp(csname, mrn_charset_map[i].csname_mysql)))
      return mrn_charset_map[i].csname_groonga;
  }
  return GRN_ENC_NONE;
}

const char *mrn_charset_groonga_mysql(grn_encoding encoding)
{
  int i;
  for (i = 0; (mrn_charset_map[i].csname_groonga != GRN_ENC_DEFAULT); i++) {
    if (mrn_charset_map[i].csname_groonga == encoding)
      return mrn_charset_map[i].csname_mysql;
  }
  return NULL;
}


grn_builtin_type mrn_get_type(grn_ctx *ctx, int type)
{
  grn_builtin_type gtype;
  switch (type)
  {
  case MYSQL_TYPE_LONG:
    gtype = GRN_DB_INT32;
    break;
  case MYSQL_TYPE_VARCHAR:
    gtype = GRN_DB_TEXT;
    break;
  default:
    gtype = GRN_DB_VOID;
  }
  return gtype;
}


handler *mrn_handler_create(handlerton *hton,
			    TABLE_SHARE *share,
			    MEM_ROOT *root)
{
  return (new (root) ha_groonga(hton, share));
}

bool mrn_plugin_flush_logs(handlerton *hton)
{
  grn_ctx ctx;
  grn_ctx_init(&ctx,0);
  mrn_flush_logs(&ctx);
  grn_ctx_fin(&ctx);
  return 0;
}

int mrn_plugin_init(void *p)
{
  handlerton *hton;
  hton = (handlerton *)p;
  hton->state = SHOW_OPTION_YES;
  hton->create = mrn_handler_create;
  hton->flush_logs = mrn_plugin_flush_logs;
  hton->flags = 0;
  return mrn_init();
}

int mrn_plugin_deinit(void *p)
{
  return mrn_deinit();
}


struct st_mysql_storage_engine storage_engine_structure =
{ MYSQL_HANDLERTON_INTERFACE_VERSION };

mysql_declare_plugin(mroonga)
{
  MYSQL_STORAGE_ENGINE_PLUGIN,
  &storage_engine_structure,
  "Mroonga",
  "Tetsuro IKEDA",
  "MySQL binding for Groonga",
  PLUGIN_LICENSE_BSD,
  mrn_plugin_init,
  mrn_plugin_deinit,
  0x0001,
  NULL,
  NULL,
  NULL
}
mysql_declare_plugin_end;

/* handler implementation */
ha_groonga::ha_groonga(handlerton *hton, TABLE_SHARE *share)
  :handler(hton, share)
{
  ctx = (grn_ctx*) malloc(sizeof(grn_ctx));
  grn_ctx_init(ctx,0);
  grn_ctx_use(ctx, mrn_db);
  minfo = NULL;
  mcond = NULL;
  cur = NULL;
}

ha_groonga::~ha_groonga()
{
  grn_ctx_fin(ctx);
  free(ctx);
}

const char *ha_groonga::table_type() const
{
  return "Mroonga";
}

const char *ha_groonga::index_type(uint inx)
{
  return "NONE";
}

static const char*ha_groonga_exts[] = {
  NullS
};
const char **ha_groonga::bas_ext() const
{
  return ha_groonga_exts;
}

ulonglong ha_groonga::table_flags() const
{
  return HA_NO_TRANSACTIONS|HA_REC_NOT_IN_SEQ|HA_NULL_IN_KEY|HA_CAN_FULLTEXT;
}

ulong ha_groonga::index_flags(uint idx, uint part, bool all_parts) const
{
  return 0;
}

/*
  create a table, mrn_table and push it to db hash.
  currently only using in-memory table in groonga.
*/
#ifdef PROTOTYPE
int ha_groonga::create(const char *name, TABLE *form, HA_CREATE_INFO *info)
{
  const char *obj_name = MRN_TABLE_NAME(name);

  grn_obj_flags table_flags = GRN_OBJ_PERSISTENT;
  grn_obj *key_type;
  uint value_size;

  /* check if pkey is exists */
  if (form->s->primary_key != MAX_KEY) {
    int pkey_parts = form->key_info[form->s->primary_key].key_parts;
    if (pkey_parts > 1) {
      return HA_WRONG_CREATE_OPTION;
    }
    table_flags |= GRN_OBJ_TABLE_PAT_KEY;
    value_size = GRN_TABLE_MAX_KEY_SIZE;
    key_type = grn_ctx_at(ctx, GRN_DB_INT32);
  } else {
    if (form->s->keys > 0) {
      return HA_WRONG_CREATE_OPTION;
    }
    table_flags |= GRN_OBJ_TABLE_NO_KEY;
    value_size = 0;
    key_type = NULL;
  }

  grn_obj *table_obj = grn_table_create(ctx, obj_name, strlen(obj_name), NULL,
					table_flags, key_type, value_size);

  int i;
  grn_obj *type;
  grn_obj *column_obj, *ft_obj;
  for (i=0; i < form->s->fields; i++) {
    Field *field = form->s->field[i];
    if (field->flags & PRI_KEY_FLAG) {
      /* we embed pkey column in table so skip here */
      continue;
    }
    type = mrn_get_type(ctx, field->type());
    if (type != NULL) {
      column_obj = grn_column_create(ctx, table_obj,
				     field->field_name, strlen(field->field_name),
				     NULL, GRN_OBJ_PERSISTENT|GRN_OBJ_COLUMN_SCALAR, type);
      /* auto fulltex index */
      if (field->type() == MYSQL_TYPE_VARCHAR) {
	ft_obj = grn_column_create(ctx, mrn_lexicon,
				   field->field_name, strlen(field->field_name),
				   NULL, GRN_OBJ_COLUMN_INDEX|GRN_OBJ_PERSISTENT, table_obj);
	grn_id id = grn_obj_id(ctx, column_obj);
	grn_obj buff;
	GRN_TEXT_INIT(&buff,0);
	GRN_TEXT_SET(ctx, &buff, (char*) &id, sizeof(grn_id));
	grn_obj_set_info(ctx, ft_obj, GRN_INFO_SOURCE, &buff);
	grn_obj_close(ctx, ft_obj);
      }

      grn_obj_close(ctx, column_obj);
    } else {
      goto err;
    }
  }

  grn_obj_close(ctx, table_obj);
  grn_obj_close(ctx, key_type);
  grn_obj_close(ctx, type);
  return 0;

 err:
  grn_obj_remove(ctx, table_obj);
  return HA_WRONG_CREATE_OPTION;
}
#else
int ha_groonga::create(const char *name, TABLE *form, HA_CREATE_INFO *info)
{
  int res;
  mrn_info *minfo;
  MRN_HTRACE;
  convert_info(name, this->table_share, &minfo);
  res = mrn_create(ctx, minfo);
  mrn_deinit_obj_info(ctx, minfo);
  return res;
}
#endif

#ifdef PROTOTYPE
int ha_groonga::open(const char *name, int mode, uint test_if_locked)
{
  thr_lock_init(&thr_lock);
  thr_lock_data_init(&thr_lock, &thr_lock_data, NULL);

  mrn_table *share;
  if (!(mrn_hash_get(ctx, MRN_TABLE_NAME(name), (void**) &share))) {
    this->share = share;
  } else {
    share = (mrn_table*) MRN_MALLOC(sizeof(mrn_table));
    share->name = MRN_TABLE_NAME(name);
    share->name_len = strlen(share->name);
    grn_obj *obj = grn_table_open(ctx, share->name, strlen(share->name), NULL);
    share->obj = obj;

    if (this->table_share->primary_key == MAX_KEY) {
      share->pkey_field = -1;
    }

    share->fields = this->table_share->fields;
    share->field = (mrn_field**) MRN_MALLOC(sizeof(mrn_field*) * (share->fields + 1));
    int i;
    for (i=0; i < share->fields; i++) {
      Field *mysql_field = this->table_share->field[i];
      mrn_field *field = (mrn_field*) MRN_MALLOC(sizeof(mrn_field));
      field->name = mysql_field->field_name;
      field->name_len = strlen(field->name);
      field->field_no = i;
      if (mysql_field->flags & PRI_KEY_FLAG) {
	share->pkey_field = i;
      } else {
	/* NOTE: currently only support INT */
	grn_obj *type = mrn_get_type(ctx, mysql_field->type());
	field->obj = grn_column_open(ctx, share->obj,
				     field->name, field->name_len,
				     NULL, type);
	if (mysql_field->type() == MYSQL_TYPE_VARCHAR) {
	  field->index = grn_column_open(ctx, mrn_lexicon,
					 field->name, field->name_len, NULL, share->obj);
	}
      }
      share->field[i] = field;
    }
    share->field[i] = NULL;

    mrn_hash_put(ctx, share->name, (void*) share);
    this->share = share;
  }
  share->use_count++;
  return 0;
}
#else
int ha_groonga::open(const char *name, int mode, uint test_if_locked)
{
  MRN_HTRACE;
  thr_lock_init(&thr_lock);
  thr_lock_data_init(&thr_lock, &thr_lock_data, NULL);

  mrn_info *minfo;
  int res;

  pthread_mutex_lock(mrn_lock);

  if (mrn_hash_get(ctx, MRN_TABLE_NAME(name), (void**) &minfo) == 0)
  {
    minfo->ref_count++;
    pthread_mutex_unlock(mrn_lock);
    return 0;
  }
  else
  {
    mrn_info *minfo;
    convert_info(name, this->table_share, &minfo);
    if (mrn_open(ctx, minfo) == 0)
    {
      mrn_hash_put(ctx, minfo->table->name, minfo);
      minfo->ref_count++;
      pthread_mutex_unlock(mrn_lock);
      this->minfo = minfo;
      return 0;
    }
    else
    {
      pthread_mutex_unlock(mrn_lock);
      mrn_deinit_obj_info(ctx, minfo);
      this->minfo = NULL;
      return -1;
    }
  }
}
#endif

#ifdef PROTOTYPE
int ha_groonga::close()
{
  thr_lock_delete(&thr_lock);

  mrn_table *share = this->share;
  mrn_hash_remove(ctx, share->name);
  grn_obj_close(ctx, share->obj);

  mrn_field **field;
  for (field = share->field; *field; field++) {
    MRN_FREE(*field);
  }
  MRN_FREE(share->field);
  MRN_FREE(share);

  return 0;
}
#else
int ha_groonga::close()
{
  MRN_HTRACE;
  thr_lock_delete(&thr_lock);

  mrn_info *minfo = this->minfo;

  pthread_mutex_lock(mrn_lock);

  minfo->ref_count--;
  if (minfo->ref_count <= 0)
  {
    if (mrn_hash_remove(ctx, minfo->table->name) != 0)
    {
      GRN_LOG(ctx, GRN_LOG_ERROR, "error in mrn_hash_remove:[%p,%s]",
              ctx, minfo->table->name);
    }
    mrn_close(ctx, minfo);
    mrn_deinit_obj_info(ctx, minfo);
    this->minfo = NULL;
  }

  pthread_mutex_unlock(mrn_lock);
  return 0;
}
#endif

#ifdef PROTOTYPE
int ha_groonga::delete_table(const char *name)
{
  const char *obj_name = MRN_TABLE_NAME(name);
  grn_obj *obj = grn_table_open(ctx, obj_name, strlen(obj_name), NULL);
  grn_obj_remove(ctx, obj);

  return 0;
}
#else
int ha_groonga::delete_table(const char *name)
{
  MRN_HTRACE;
  return mrn_drop(ctx, MRN_TABLE_NAME(name));
}
#endif

#ifdef PROTOTYPE
int ha_groonga::info(uint flag)
{
  stats.records = (ha_rows) grn_table_size(ctx, share->obj);
  return 0;
}
#else
int ha_groonga::info(uint flag)
{
  MRN_HTRACE;
  stats.records = (ha_rows) mrn_table_size(ctx, this->minfo);
  return 0;
}
#endif

THR_LOCK_DATA **ha_groonga::store_lock(THD *thd,
				       THR_LOCK_DATA **to,
				       enum thr_lock_type lock_type)
{
  MRN_HTRACE;
  if (lock_type != TL_IGNORE && thr_lock_data.type == TL_UNLOCK)
    thr_lock_data.type = lock_type;
  *to++ = &thr_lock_data;
  return to;
}

#ifdef PROTOTYPE
int ha_groonga::rnd_init(bool scan)
{
  this->cursor = grn_table_cursor_open(ctx, share->obj,
				       NULL, 0, NULL, 0, 0);
  return 0;
}
#else
int ha_groonga::rnd_init(bool scan)
{
  MRN_HTRACE;
  this->cur = mrn_init_record(ctx, minfo);
  return mrn_rnd_init(ctx, minfo);
}
#endif

#ifdef PROTOTYPE
int ha_groonga::rnd_next(uchar *buf)
{
  grn_id gid = grn_table_cursor_next(ctx, this->cursor);
  if (gid != GRN_ID_NIL) {
    grn_obj obj;
    GRN_TEXT_INIT(&obj,0);

    Field **mysql_field;
    mrn_field **grn_field;
    int num;
    for (mysql_field = table->field, grn_field = share->field, num=0;
	 *mysql_field;
	 mysql_field++, grn_field++, num++) {
      if (num == share->pkey_field) {
	int *val;
	grn_table_cursor_get_key(ctx, this->cursor, (void**) &val);
	(*mysql_field)->set_notnull();
	(*mysql_field)->store(*val);
      } else {
	GRN_BULK_REWIND(&obj);
	grn_obj_get_value(ctx, (*grn_field)->obj, gid, &obj);
	/* TODO: refactoring. following can be share with index_read */
	int *tmp_int;
	char *tmp_char;
	switch((*mysql_field)->type()) {
	case (MYSQL_TYPE_LONG) :
	  tmp_int = (int*) GRN_BULK_HEAD(&obj);
	  (*mysql_field)->set_notnull();
	  (*mysql_field)->store(*tmp_int);
	  break;
	case (MYSQL_TYPE_VARCHAR) :
	  tmp_char = (char*) GRN_BULK_HEAD(&obj);
	  (*mysql_field)->set_notnull();
	  (*mysql_field)->store(tmp_char,strlen(tmp_char), system_charset_info);
	  break;
	}
      }
    }
    this->record_id = gid;
    grn_obj_close(ctx, &obj);
    return 0;
  } else {
    grn_table_cursor_close(ctx, this->cursor);
    return HA_ERR_END_OF_FILE;
  }
}
#else
int ha_groonga::rnd_next(uchar *buf)
{
  MRN_HTRACE;
  int rc;
  mrn_record *record;
  mrn_info *info = this->minfo;
  record = this->cur;
  if (mcond == NULL)
  {
    rc = mrn_rnd_next(ctx, record, NULL);
  }
  else
  {
    rc = mrn_rnd_next(ctx, record, mcond->list);
  }
  if (rc == 0)
  {
    Field **field;
    int i;
    for (i=0, field = table->field; *field; i++, field++)
    {
      if (record->value[i] == NULL)
      {
        (*field)->set_null();
      }
      else
      {
        int vint;
        char *vchar;
        switch ((*field)->type())
        {
        case (MYSQL_TYPE_LONG) :
          vint = GRN_INT32_VALUE(record->value[i]);
          (*field)->set_notnull();
          (*field)->store(vint);
          break;
        case (MYSQL_TYPE_VARCHAR) :
          vchar = GRN_TEXT_VALUE(record->value[i]);
          (*field)->set_notnull();
          (*field)->store(vchar, GRN_BULK_WSIZE(record->value[i]), system_charset_info);
          break;
        }
      }
    }
    mrn_rewind_record(ctx, record);
    return 0;
  }
  else if (rc == 1)
  {
    mcond = NULL;
    mrn_deinit_record(ctx, record);
    cur = NULL;
    return HA_ERR_END_OF_FILE;
  }
  else
  {
    mcond = NULL;
    mrn_deinit_record(ctx, record);
    cur = NULL;
    return -1;
  }
}
#endif

#ifdef PROTOTYPE
int ha_groonga::rnd_pos(uchar *buf, uchar *pos)
{
  grn_id gid = *((grn_id*) pos);

  grn_obj obj;
  int pkey_val;
  int *val;
  GRN_TEXT_INIT(&obj,0);

  Field **mysql_field;
  mrn_field **grn_field;
  int num;
  for (mysql_field = table->field, grn_field = share->field, num=0;
       *mysql_field;
       mysql_field++, grn_field++, num++) {
    if (num == share->pkey_field) {
      int ret_val = grn_table_get_key(ctx, share->obj, gid, (void*) &pkey_val, sizeof(int));
      (*mysql_field)->set_notnull();
      (*mysql_field)->store(pkey_val);
    } else {
      GRN_BULK_REWIND(&obj);
      grn_obj_get_value(ctx, (*grn_field)->obj, gid, &obj);
      val = (int*) GRN_BULK_HEAD(&obj);
      (*mysql_field)->set_notnull();
      (*mysql_field)->store(*val);
    }
  }
  return 0;
}
#else
int ha_groonga::rnd_pos(uchar *buf, uchar *pos)
{
  MRN_HTRACE;
  return 0;
}
#endif

#ifdef PROTOTYPE
void ha_groonga::position(const uchar *record)
{
  memcpy(this->ref, &this->record_id, sizeof(grn_id));
}
#else
void ha_groonga::position(const uchar *record)
{
  MRN_HTRACE;
}
#endif

#ifdef PROTOTYPE
int ha_groonga::write_row(uchar *buf)
{
  /*
  int primary_key_no = table->s->primary_key;
  KEY pkey = table->s->key_info[primary_key_no];
  */

  /* NOTE: currently we use 1st column value as primary key value */
  grn_search_flags flags = GRN_TABLE_ADD;
  grn_id gid;
  grn_obj wrapper;
  Field **mysql_field;
  mrn_field **grn_field;
  int num;

  if (share->pkey_field != -1) {
    Field *pkey_field = table->field[share->pkey_field];
    int pkey_value = pkey_field->val_int();
    gid = grn_table_lookup(ctx, share->obj,
			   (const void*) &pkey_value, sizeof(pkey_value), &flags);
  } else {
    int emu_key = 0;
    gid = grn_table_add(ctx, share->obj, (const void*) &emu_key, 4, NULL);
  }

  GRN_TEXT_INIT(&wrapper,0);
  for (mysql_field = table->field, grn_field = share->field, num=0;
       *mysql_field;
       mysql_field++, grn_field++, num++) {
    if (num == share->pkey_field) {
      continue;
    }
    GRN_BULK_REWIND(&wrapper);
    /* TODO: replace if-else into swtich-case */
    if ((*mysql_field)->type() == MYSQL_TYPE_LONG) {
      int val = (*mysql_field)->val_int();
      GRN_TEXT_SET(ctx, &wrapper, (char*)&val, sizeof(val));
    } else if ((*mysql_field)->type() == MYSQL_TYPE_VARCHAR) {
      String tmp;
      const char *val = (*mysql_field)->val_str(&tmp)->ptr();
      GRN_TEXT_SET(ctx, &wrapper, val, strlen(val));
    } else {
      return HA_ERR_UNSUPPORTED;
    }
    grn_obj_set_value(ctx, (*grn_field)->obj, gid, &wrapper, GRN_OBJ_SET);
  }
  grn_obj_close(ctx, &wrapper);
  return 0;
}
#else
int ha_groonga::write_row(uchar *buf)
{
  MRN_HTRACE;
  mrn_info *minfo = this->minfo;
  mrn_record *record = mrn_init_record(ctx, minfo);
  Field **field;
  int i;
  for (i=0, field = table->field; *field; i++, field++)
  {
    /* TODO: replace if-else into swtich-case */
    if ((*field)->type() == MYSQL_TYPE_LONG) {
      int val = (*field)->val_int();
      GRN_TEXT_SET(ctx, record->value[i], (char*)&val, sizeof(val));
    } else if ((*field)->type() == MYSQL_TYPE_VARCHAR) {
      String tmp;
      const char *val = (*field)->val_str(&tmp)->ptr();
      GRN_TEXT_SET(ctx, record->value[i], val, strlen(val));
    } else {
      return HA_ERR_UNSUPPORTED;
    }
  }
  if (mrn_write_row(ctx, record) != 0)
  {
    return -1;
  }
  mrn_deinit_record(ctx, record);
  return 0;
}
#endif

#ifdef PROTOTYPE
int ha_groonga::index_read(uchar *buf, const uchar *key,
			   uint key_len, enum ha_rkey_function find_flag)
{
  Field *key_field= table->key_info[active_index].key_part->field;
  uint rc= 0;
  grn_id gid;
  grn_obj wrapper;
  Field **mysql_field;
  mrn_field **grn_field;
  int num;
  grn_search_flags flags = 0;

  int k;
  memcpy(&k,key,sizeof(int));
  gid = grn_table_lookup(ctx, share->obj,
			 (const void*) key, sizeof(key), &flags);

  GRN_TEXT_INIT(&wrapper,0);
  for (mysql_field = table->field, grn_field = share->field, num=0;
       *mysql_field;
       mysql_field++, grn_field++, num++) {
    if (num == share->pkey_field) {
      continue;
    }
    GRN_BULK_REWIND(&wrapper);
    grn_obj_get_value(ctx, (*grn_field)->obj, gid, &wrapper);
    int *tmp_int;
    char *tmp_char;
    switch((*mysql_field)->type()) {
    case (MYSQL_TYPE_LONG) :
      tmp_int = (int*) GRN_BULK_HEAD(&wrapper);
      (*mysql_field)->set_notnull();
      (*mysql_field)->store(*tmp_int);
      break;
    case (MYSQL_TYPE_VARCHAR) :
      tmp_char = (char*) GRN_BULK_HEAD(&wrapper);
      (*mysql_field)->set_notnull();
      (*mysql_field)->store(tmp_char,strlen(tmp_char), system_charset_info);
      break;
    }
  }
  grn_obj_close(ctx, &wrapper);

  if (key_field->field_index == table->s->primary_key)
  {
    key_field->set_key_image(key, key_len);
    key_field->set_notnull();
  }
  return rc;
}
#else
int ha_groonga::index_read(uchar *buf, const uchar *key,
			   uint key_len, enum ha_rkey_function find_flag)
{
  MRN_HTRACE;
  return 0;
}
#endif

#ifdef PROTOTYPE
int ha_groonga::index_next(uchar *buf)
{
  return HA_ERR_END_OF_FILE;
}
#else
int ha_groonga::index_next(uchar *buf)
{
  MRN_HTRACE;
  return HA_ERR_END_OF_FILE;
}
#endif

#ifdef PROTOTYPE
int ha_groonga::ft_init() {
  return 0;
}
#else
int ha_groonga::ft_init() {
  MRN_HTRACE;
  return 0;
}
#endif

#ifdef PROTOTYPE
FT_INFO *ha_groonga::ft_init_ext(uint flags, uint inx,String *key)
{
  const char *match_param;
  match_param = key->ptr();
  if (flags & FT_BOOL) {
    /* boolean search */
    grn_query *query;
    this->res = grn_table_create(ctx, NULL, 0, NULL, GRN_TABLE_HASH_KEY, share->obj, 0);
    query = grn_query_open(ctx, match_param, strlen(match_param), GRN_OP_OR, 32);
    grn_obj_search(ctx, share->field[1]->index, (grn_obj*) query, this->res, GRN_OP_OR, NULL);
    this->cursor = grn_table_cursor_open(ctx, res, NULL, 0, NULL, 0, 0);
    //grn_query_close(ctx, query);
  } else {
    /* nlq search */
    grn_obj buff;
    this->res = grn_table_create(ctx, NULL, 0, NULL, GRN_TABLE_HASH_KEY, share->obj, 0);
    GRN_TEXT_INIT(&buff, 0);
    GRN_TEXT_SET(ctx, &buff, match_param, strlen(match_param));
    grn_obj_search(ctx, share->field[1]->index, &buff, this->res, GRN_OP_OR, NULL);
    this->cursor = grn_table_cursor_open(ctx, res, NULL, 0, NULL, 0, 0);
    //grn_obj_close(ctx, &buff);
  }
  int nrec = grn_table_size(ctx, res);
  return NULL;
}
#else
FT_INFO *ha_groonga::ft_init_ext(uint flags, uint inx,String *key)
{
  MRN_HTRACE;
  return NULL;
}
#endif

#ifdef PROTOTYPE
int ha_groonga::ft_read(uchar *buf)
{
  /*
  if (mrn_counter == 0) {
    table->field[0]->set_notnull();
    table->field[0]->store(10);
    table->field[1]->set_notnull();
    table->field[1]->store("test", 4, system_charset_info);
    mrn_counter++;
    return 0;
  } else {
    mrn_counter=0;
    return HA_ERR_END_OF_FILE;
    }*/
  grn_id id, docid;
  grn_obj buff;
  GRN_TEXT_INIT(&buff,0);
  if  ((id = grn_table_cursor_next(ctx, this->cursor))) {
    GRN_BULK_REWIND(&buff);
    grn_table_get_key(ctx, this->res, id, &docid, sizeof(grn_id));
    grn_obj_get_value(ctx, share->field[1]->obj, docid, &buff);
    table->field[0]->set_notnull();
    table->field[0]->store(docid);
    table->field[1]->set_notnull();
    table->field[1]->store((char*) GRN_BULK_HEAD(&buff), GRN_TEXT_LEN(&buff), system_charset_info);
    return 0;
  }
  table->status = HA_ERR_END_OF_FILE;
  return HA_ERR_END_OF_FILE;
}
#else
int ha_groonga::ft_read(uchar *buf)
{
  MRN_HTRACE;
  return HA_ERR_END_OF_FILE;
}
#endif

const COND *ha_groonga::cond_push(const COND *cond)
{
  MRN_HTRACE;
  if (cond)
  {
    mrn_cond *tmp = (mrn_cond*) malloc(sizeof(mrn_cond));
    if (tmp == NULL)
    {
      goto err_oom;
    }
    tmp->cond = (COND *) cond;
    tmp->next = this->mcond;
    mcond = tmp;
    convert_cond((Item*) cond);
  }
  DBUG_RETURN(NULL);

err_oom:
  my_errno = HA_ERR_OUT_OF_MEM;
  GRN_LOG(ctx, GRN_LOG_ERROR, "malloc error in cond_push");
  return NULL;
}

void ha_groonga::cond_pop()
{
  MRN_HTRACE;
  if (mcond)
  {
    mrn_cond *tmp = mcond->next;
    free(mcond);
    mcond = tmp;
  }
}

int ha_groonga::convert_info(const char *name, TABLE_SHARE *share, mrn_info **_minfo)
{
  uint n_columns = share->fields, i;
  mrn_info *minfo = mrn_init_obj_info(ctx, n_columns);
  minfo->table->name = MRN_TABLE_NAME(name);
  minfo->table->name_size = strlen(MRN_TABLE_NAME(name));
  minfo->table->flags |= GRN_OBJ_TABLE_NO_KEY;

  for (i=0; i < n_columns; i++)
  {
    Field *field = share->field[i];
    minfo->columns[i]->name = field->field_name;
    minfo->columns[i]->name_size = strlen(minfo->columns[i]->name);
    minfo->columns[i]->flags |= GRN_OBJ_COLUMN_SCALAR;
    grn_builtin_type gtype = mrn_get_type(ctx, field->type());
    minfo->columns[i]->gtype = gtype;
    if (gtype != GRN_DB_VOID)
    {
      minfo->columns[i]->type = grn_ctx_at(ctx, gtype);
    }
    else
    {
      return -1;
    }
  }

  minfo->n_columns = n_columns;
  *_minfo = minfo;
  return 0;
}


const char *mrn_item_type_string[] = {
  "FIELD_ITEM", "FUNC_ITEM", "SUM_FUNC_ITEM", "STRING_ITEM",
  "INT_ITEM", "REAL_ITEM", "NULL_ITEM", "VARBIN_ITEM",
  "COPY_STR_ITEM", "FIELD_AVG_ITEM", "DEFAULT_VALUE_ITEM",
  "PROC_ITEM", "COND_ITEM", "REF_ITEM", "FIELD_STD_ITEM",
  "FIELD_VARIANCE_ITEM", "INSERT_VALUE_ITEM",
  "SUBSELECT_ITEM", "ROW_ITEM", "CACHE_ITEM", "TYPE_HOLDER",
  "PARAM_ITEM", "TRIGGER_FIELD_ITEM", "DECIMAL_ITEM",
  "XPATH_NODESET", "XPATH_NODESET_CMP",
  "VIEW_FIXER_ITEM"};

const char *mrn_functype_string[] = {
  "UNKNOWN_FUNC","EQ_FUNC","EQUAL_FUNC","NE_FUNC","LT_FUNC","LE_FUNC",
  "GE_FUNC","GT_FUNC","FT_FUNC",
  "LIKE_FUNC","ISNULL_FUNC","ISNOTNULL_FUNC",
  "COND_AND_FUNC", "COND_OR_FUNC", "COND_XOR_FUNC",
  "BETWEEN", "IN_FUNC", "MULT_EQUAL_FUNC",
  "INTERVAL_FUNC", "ISNOTNULLTEST_FUNC",
  "SP_EQUALS_FUNC", "SP_DISJOINT_FUNC","SP_INTERSECTS_FUNC",
  "SP_TOUCHES_FUNC","SP_CROSSES_FUNC","SP_WITHIN_FUNC",
  "SP_CONTAINS_FUNC","SP_OVERLAPS_FUNC",
  "SP_STARTPOINT","SP_ENDPOINT","SP_EXTERIORRING",
  "SP_POINTN","SP_GEOMETRYN","SP_INTERIORRINGN",
  "NOT_FUNC", "NOT_ALL_FUNC",
  "NOW_FUNC", "TRIG_COND_FUNC",
  "SUSERVAR_FUNC", "GUSERVAR_FUNC", "COLLATE_FUNC",
  "EXTRACT_FUNC", "CHAR_TYPECAST_FUNC", "FUNC_SP", "UDF_FUNC",
  "NEG_FUNC", "GSYSVAR_FUNC"};

extern int string2my_decimal(uint mask, const String *str, my_decimal *d);
#include "item_func.h"
#include "my_decimal.h"

/*
int ha_groonga::convert_cond(Item *cond)
{
  char buff[256];
  String str(buff,(uint32) sizeof(buff), system_charset_info);
  str.length(0);
  cond->print(&str, QT_ORDINARY);
  str.append("\0");
  printf("COND* name=%s, type()=%s", cond->name, mrn_item_type_string[cond->type()]);
  if (cond->type() == Item::FUNC_ITEM) {
    Item_func *item = (Item_func*) cond;
    printf(", functype=%s", mrn_functype_string[item->functype()]);
  } else if (cond->type() == Item::INT_ITEM) {
    Item_int *item = (Item_int*) cond;
    printf(", value=%d", item->value);
  } else if (cond->type() == Item::FIELD_ITEM) {
  }
  printf(", str=%s",str.ptr());
  printf("\n");
  if (cond->next != NULL)
  {
    convert_cond(cond->next);
  }
  else
  {
    printf("-----\n");
  }
  return 0;
}
*/
/*
int ha_groonga::convert_cond(Item *cond)
{
  Item *tmp = cond;
  printf("%s: ", this->minfo->table->name);
  while (tmp)
  {
    //printf(" %s", mrn_item_type_string[(int) tmp->type()]);
    if (tmp->type() == Item::FUNC_ITEM)
    {
      Item_func *func = (Item_func*) tmp;
      printf("(%s)", mrn_functype_string[(int) func->functype()]);
    }
    else
    {
      printf("(%s)", tmp->name);
    }
    tmp = tmp->next;
  }
  printf("\n");
  return 0;
}
*/

int ha_groonga::convert_cond(Item *cond)
{
  int idx, max_size=256;
  int src[max_size];
  Item *item = cond;
  memset(src, 0, sizeof(int)*max_size);
  // currently only pruning columns access
  for (idx=0; item;)
  {
    if (item->type() == Item::FIELD_ITEM)
    {
      int i=0;
      if (is_own_field((Item_field*) item))
      {
        Field *field = ((Item_field*) item)->field;
        src[idx] = field->field_index;
        if (idx++ >= max_size)
        {
          GRN_LOG(ctx, GRN_LOG_ERROR, "index overflow on convert_cond");
          return -1;
        }
      }
    }
    item = item->next;
  }
  mcond->list = mrn_init_column_list(ctx, minfo, src, idx);
  return 0;
}

int ha_groonga::is_own_field(Item_field *item)
{
  Field *field = item->field;
  if ((field != NULL) &&
      (strncmp(table_share->table_name.str,
               field->table->s->table_name.str,
               table_share->table_name.length) == 0) &&
      (strncmp(table_share->db.str,
               field->table->s->db.str,
               table_share->db.length) == 0))
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

#ifdef __cplusplus
}
#endif
