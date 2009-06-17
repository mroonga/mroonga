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

/* handler declaration */


handler *mrn_handler_create(handlerton *hton,
			    TABLE_SHARE *share,
			    MEM_ROOT *root)
{
  return (new (root) ha_groonga(hton, share));
}

bool mrn_plugin_flush_logs(handlerton *hton)
{
  return (bool) mrn_flush_logs();
}

int mrn_plugin_init(void *p)
{
  handlerton *hton;
  hton = (handlerton *)p;
  hton->state = SHOW_OPTION_YES;
  hton->create = mrn_handler_create;
  hton->flush_logs = mrn_plugin_flush_logs;
  hton->flags = 0;
  mrn_init(NULL);
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
  mrn_deinit,
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
  mrn_counter=0;
}

ha_groonga::~ha_groonga()
{
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
  ".grn",
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
int ha_groonga::create(const char *name, TABLE *form, HA_CREATE_INFO *info)
{
  mrn_ctx_init();
  MRN_TRACE;

  const char *obj_name = MRN_TABLE_NAME(name);
  char path[MRN_MAX_KEY_LEN];
  MRN_TABLE_PATH(path, obj_name);

  grn_obj_flags table_flags = GRN_OBJ_PERSISTENT;
  grn_obj *key_type;
  uint value_size;

  /* check if pkey is exists */
  if (form->s->primary_key != MAX_KEY) {
    int pkey_parts = form->key_info[form->s->primary_key].key_parts;
    if (pkey_parts > 1) {
      MRN_LOG(GRN_LOG_ERROR, "primary key must be single column");
      return HA_WRONG_CREATE_OPTION;
    }
    table_flags |= GRN_OBJ_TABLE_PAT_KEY;
    value_size = GRN_TABLE_MAX_KEY_SIZE;
    key_type = grn_ctx_at(mrn_ctx_tls, GRN_DB_INT32);
  } else {
    if (form->s->keys > 0) {
      MRN_LOG(GRN_LOG_ERROR, "cannot create index other than primary key");
      return HA_WRONG_CREATE_OPTION;
    }
    table_flags |= GRN_OBJ_TABLE_NO_KEY;
    value_size = 0;
    key_type = NULL;
  }

  MRN_LOG(GRN_LOG_DEBUG, "-> grn_table_create: name='%s', path='%s'",obj_name, path);
  grn_obj *table_obj = grn_table_create(mrn_ctx_tls, obj_name, strlen(obj_name), path,
					table_flags, key_type, value_size);

  int i;
  grn_obj *type;
  grn_obj *column_obj, *ft_obj;
  char buf[MRN_MAX_KEY_LEN];
  for (i=0; i < form->s->fields; i++) {
    Field *field = form->s->field[i];
    if (field->flags & PRI_KEY_FLAG) {
      /* we embed pkey column in table so skip here */
      MRN_LOG(GRN_LOG_DEBUG, "-> column '%s' is pkey", field->field_name);
      continue;
    }
    MRN_COLUMN_PATH(buf, form->s->db.str, form->s->table_name.str, field->field_name);
    type = mrn_get_type(field->type());
    if (type != NULL) {
      MRN_LOG(GRN_LOG_DEBUG, "-> grn_column_create: name='%s', path='%s', mtype=%d", 
	      field->field_name, buf,field->type());
      column_obj = grn_column_create(mrn_ctx_tls, table_obj,
				     field->field_name, strlen(field->field_name),
				     buf, GRN_OBJ_PERSISTENT|GRN_OBJ_COLUMN_SCALAR, type);
      /* auto fulltex index */
      if (field->type() == MYSQL_TYPE_VARCHAR) {
	ft_obj = grn_column_create(mrn_ctx_tls, mrn_lexicon_sys,
				   field->field_name, strlen(field->field_name),
				   NULL, GRN_OBJ_COLUMN_INDEX|GRN_OBJ_PERSISTENT, table_obj);
	grn_id id = grn_obj_id(mrn_ctx_tls, column_obj);
	grn_obj buff;
	GRN_TEXT_INIT(&buff,0);
	GRN_TEXT_SET(mrn_ctx_tls, &buff, (char*) &id, sizeof(grn_id));
	grn_obj_set_info(mrn_ctx_tls, ft_obj, GRN_INFO_SOURCE, &buff);
	grn_obj_close(mrn_ctx_tls, ft_obj);
      }

      grn_obj_close(mrn_ctx_tls, column_obj);
    } else {
      goto err;
    }
  }

  MRN_LOG(GRN_LOG_DEBUG, "-> grn_obj_close: table_obj=%p", table_obj);
  grn_obj_close(mrn_ctx_tls, table_obj);
  grn_obj_close(mrn_ctx_tls, key_type);
  grn_obj_close(mrn_ctx_tls, type);
  return 0;

 err:
  MRN_LOG(GRN_LOG_ERROR, "got error while CREATE TABLE '%s'", obj_name);
  MRN_LOG(GRN_LOG_DEBUG, "-> grn_obj_remove: table_obj=%p", table_obj);
  grn_obj_remove(mrn_ctx_tls, table_obj);
  return HA_WRONG_CREATE_OPTION;
}

int ha_groonga::open(const char *name, int mode, uint test_if_locked)
{
  mrn_ctx_init();
  MRN_TRACE;

  thr_lock_init(&thr_lock);
  thr_lock_data_init(&thr_lock, &thr_lock_data, NULL);

  mrn_share *share;

  if ((share = mrn_share_get(MRN_TABLE_NAME(name)))) {
    this->share = share;
  } else {
    share = (mrn_share*) MRN_MALLOC(sizeof(mrn_share));
    share->name = MRN_TABLE_NAME(name);
    share->name_len = strlen(share->name);
    char path[MRN_MAX_KEY_LEN];
    MRN_TABLE_PATH(path, share->name);
    MRN_LOG(GRN_LOG_DEBUG, "-> grn_table_open: name='%s', path='%s'", share->name, path);
    grn_obj *obj = grn_table_open(mrn_ctx_tls, share->name, strlen(share->name), path);
    share->obj = obj;

    if (this->table_share->primary_key == MAX_KEY) {
      MRN_LOG(GRN_LOG_DEBUG, "-> table doesn't have pkey");
      share->pkey_field = -1;
    }

    share->fields = this->table_share->fields;
    share->field = (mrn_field**) MRN_MALLOC(sizeof(mrn_field*) * (share->fields + 1));
    int i;
    char buf[MRN_MAX_KEY_LEN];
    for (i=0; i < share->fields; i++) {
      Field *mysql_field = this->table_share->field[i];
      mrn_field *field = (mrn_field*) MRN_MALLOC(sizeof(mrn_field));
      field->name = mysql_field->field_name;
      field->name_len = strlen(field->name);
      field->field_no = i;
      if (mysql_field->flags & PRI_KEY_FLAG) {
	share->pkey_field = i;
      } else {
	snprintf(buf,1023,"%s.%s.grn", share->name, field->name);
	/* NOTE: currently only support INT */
	grn_obj *type = mrn_get_type(mysql_field->type());
	MRN_LOG(GRN_LOG_DEBUG, "-> grn_column_open: name='%s', path='%s'",
		field->name, buf);
	field->obj = grn_column_open(mrn_ctx_tls, share->obj,
				     field->name, field->name_len,
				     buf, type);
	MRN_LOG(GRN_LOG_DEBUG, "-> field->obj=%p", field->obj);
	if (mysql_field->type() == MYSQL_TYPE_VARCHAR) {
	  field->index = grn_column_open(mrn_ctx_tls, mrn_lexicon_sys,
					 field->name, field->name_len, NULL, share->obj);
	  MRN_LOG(GRN_LOG_DEBUG,"-> grn_column_open: name(ft)=%s, obj=%p", field->name, field->index);
	}
      }
      share->field[i] = field;
    }
    share->field[i] = NULL;

    mrn_share_put(share);
    this->share = share;
  }
  share->use_count++;
  return 0;
}

int ha_groonga::close()
{
  mrn_ctx_init();
  MRN_TRACE;

  thr_lock_delete(&thr_lock);

  mrn_share *share = this->share;
  mrn_share_remove(share);
  MRN_LOG(GRN_LOG_DEBUG, "-> grn_obj_close: '%s'", share->name);
  grn_obj_close(mrn_ctx_tls, share->obj);

  mrn_field **field;
  for (field = share->field; *field; field++) {
    MRN_FREE(*field);
  }
  MRN_FREE(share->field);
  MRN_FREE(share);

  return 0;
}

int ha_groonga::info(uint flag)
{
  mrn_ctx_init();
  MRN_TRACE;

  stats.records = (ha_rows) grn_table_size(mrn_ctx_tls, share->obj);

  return 0;
}

THR_LOCK_DATA **ha_groonga::store_lock(THD *thd,
				       THR_LOCK_DATA **to,
				       enum thr_lock_type lock_type)
{
  mrn_ctx_init();
  MRN_TRACE;
  if (lock_type != TL_IGNORE && thr_lock_data.type == TL_UNLOCK)
    thr_lock_data.type = lock_type;
  *to++ = &thr_lock_data;
  return to;
}

int ha_groonga::rnd_init(bool scan)
{
  MRN_TRACE;
  MRN_LOG(GRN_LOG_DEBUG, "-> grn_table_cursor_open: share->obj=%p", share->obj);
  this->cursor = grn_table_cursor_open(mrn_ctx_tls, share->obj,
				       NULL, 0, NULL, 0, 0);
  return 0;
}

int ha_groonga::rnd_next(uchar *buf)
{
  MRN_TRACE;
  MRN_LOG(GRN_LOG_DEBUG, "-> grn_table_cursor_next: this->cursor=%p", this->cursor);
  grn_id gid = grn_table_cursor_next(mrn_ctx_tls, this->cursor);
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
	grn_table_cursor_get_key(mrn_ctx_tls, this->cursor, (void**) &val);
	(*mysql_field)->set_notnull();
	(*mysql_field)->store(*val);
      } else {
	GRN_BULK_REWIND(&obj);
	grn_obj_get_value(mrn_ctx_tls, (*grn_field)->obj, gid, &obj);
	/* TODO: refactoring. following can be share with index_read */
	int *tmp_int;
	char *tmp_char;
	switch((*mysql_field)->type()) {
	case (MYSQL_TYPE_LONG) :
	  tmp_int = (int*) GRN_BULK_HEAD(&obj);
	  MRN_LOG(GRN_LOG_DEBUG, "-> grn_obj_get_value: gid=%d, obj=%p, val=%d",
		  gid, (*grn_field)->obj, *tmp_int);
	  (*mysql_field)->set_notnull();
	  (*mysql_field)->store(*tmp_int);
	  break;
	case (MYSQL_TYPE_VARCHAR) :
	  tmp_char = (char*) GRN_BULK_HEAD(&obj);
	  MRN_LOG(GRN_LOG_DEBUG, "-> grn_obj_get_value: gid=%d, obj=%p, val=%s",
		  gid, (*grn_field)->obj, tmp_char);
	  (*mysql_field)->set_notnull();
	  (*mysql_field)->store(tmp_char,strlen(tmp_char), system_charset_info);
	  break;
	}
      }
    }
    this->record_id = gid;
    grn_obj_close(mrn_ctx_tls, &obj);
    return 0;
  } else {
    MRN_LOG(GRN_LOG_DEBUG, "-> grn_table_cursor_close: this->cursor=%p", this->cursor);
    grn_table_cursor_close(mrn_ctx_tls, this->cursor);
    return HA_ERR_END_OF_FILE;
  }
}

int ha_groonga::rnd_pos(uchar *buf, uchar *pos)
{
  MRN_TRACE;
  grn_id gid = *((grn_id*) pos);
  MRN_LOG(GRN_LOG_DEBUG,"-> gid=%d", gid);

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
      int ret_val = grn_table_get_key(mrn_ctx_tls, share->obj, gid, (void*) &pkey_val, sizeof(int));
      (*mysql_field)->set_notnull();
      (*mysql_field)->store(pkey_val);
    } else {
      GRN_BULK_REWIND(&obj);
      grn_obj_get_value(mrn_ctx_tls, (*grn_field)->obj, gid, &obj);
      val = (int*) GRN_BULK_HEAD(&obj);
      MRN_LOG(GRN_LOG_DEBUG, "-> grn_obj_get_value: gid=%d, obj=%p, val=%d",
	      gid, (*grn_field)->obj, *val);
      (*mysql_field)->set_notnull();
      (*mysql_field)->store(*val);
    }
  }
  return 0;
}

void ha_groonga::position(const uchar *record)
{
  MRN_TRACE;
  memcpy(this->ref, &this->record_id, sizeof(grn_id));
}

int ha_groonga::delete_table(const char *name)
{
  mrn_ctx_init();
  MRN_TRACE;

  const char *obj_name = MRN_TABLE_NAME(name);
  char path[MRN_MAX_KEY_LEN];
  MRN_TABLE_PATH(path, obj_name);
  MRN_LOG(GRN_LOG_DEBUG, "-> grn_table_open: name='%s', path='%s'", obj_name, path);
  grn_obj *obj = grn_table_open(mrn_ctx_tls, obj_name, strlen(obj_name), path);
  MRN_LOG(GRN_LOG_DEBUG, "-> grn_obj_remove: obj=%p", obj);
  grn_obj_remove(mrn_ctx_tls, obj);

  return 0;
}

int ha_groonga::write_row(uchar *buf)
{
  MRN_TRACE;

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
    gid = grn_table_lookup(mrn_ctx_tls, share->obj,
			   (const void*) &pkey_value, sizeof(pkey_value), &flags);
    MRN_LOG(GRN_LOG_DEBUG, "-> added record: pkey_value=%d, gid=%d",pkey_value,gid);
  } else {
    int emu_key = 0;
    gid = grn_table_add(mrn_ctx_tls, share->obj, (const void*) &emu_key, 4, NULL);
    MRN_LOG(GRN_LOG_DEBUG, "-> added record w/o pkey, gid=%d",gid);
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
      GRN_TEXT_SET(mrn_ctx_tls, &wrapper, (char*)&val, sizeof(val));
    } else if ((*mysql_field)->type() == MYSQL_TYPE_VARCHAR) {
      String tmp;
      const char *val = (*mysql_field)->val_str(&tmp)->ptr();
      GRN_TEXT_SET(mrn_ctx_tls, &wrapper, val, strlen(val));
    } else {
      MRN_LOG(GRN_LOG_DEBUG, "unsupported data type specified");
      return HA_ERR_UNSUPPORTED;
    }
    MRN_LOG(GRN_LOG_DEBUG, "-> grn_obj_set_value: gid=%d, obj=%p",
	    gid, (*grn_field)->obj);
    grn_obj_set_value(mrn_ctx_tls, (*grn_field)->obj, gid, &wrapper, GRN_OBJ_SET);
  }
  grn_obj_close(mrn_ctx_tls, &wrapper);
  return 0;
}

int ha_groonga::index_read(uchar *buf, const uchar *key,
			   uint key_len, enum ha_rkey_function find_flag)
{
  MRN_TRACE;
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
  gid = grn_table_lookup(mrn_ctx_tls, share->obj,
			 (const void*) key, sizeof(key), &flags);
  MRN_LOG(GRN_LOG_DEBUG, "-> found record: key=%d, gid=%d",k,gid);

  GRN_TEXT_INIT(&wrapper,0);
  for (mysql_field = table->field, grn_field = share->field, num=0;
       *mysql_field;
       mysql_field++, grn_field++, num++) {
    if (num == share->pkey_field) {
      continue;
    }
    GRN_BULK_REWIND(&wrapper);
    MRN_LOG(GRN_LOG_DEBUG, "-> grn_obj_get_value: gid=%d, obj=%p",
	    gid, (*grn_field)->obj);
    grn_obj_get_value(mrn_ctx_tls, (*grn_field)->obj, gid, &wrapper);
    int *tmp_int;
    char *tmp_char;
    switch((*mysql_field)->type()) {
    case (MYSQL_TYPE_LONG) :
      tmp_int = (int*) GRN_BULK_HEAD(&wrapper);
      MRN_LOG(GRN_LOG_DEBUG, "-> grn_obj_get_value: gid=%d, obj=%p, val=%d",
	      gid, (*grn_field)->obj, *tmp_int);
      (*mysql_field)->set_notnull();
      (*mysql_field)->store(*tmp_int);
      break;
    case (MYSQL_TYPE_VARCHAR) :
      tmp_char = (char*) GRN_BULK_HEAD(&wrapper);
      MRN_LOG(GRN_LOG_DEBUG, "-> grn_obj_get_value: gid=%d, obj=%p, val=%s",
	      gid, (*grn_field)->obj, tmp_char);
      (*mysql_field)->set_notnull();
      (*mysql_field)->store(tmp_char,strlen(tmp_char), system_charset_info);
      break;
    }
  }
  grn_obj_close(mrn_ctx_tls, &wrapper);

  if (key_field->field_index == table->s->primary_key)
  {
    key_field->set_key_image(key, key_len);
    key_field->set_notnull();
  }
  return rc;
}

int ha_groonga::index_next(uchar *buf)
{
  MRN_TRACE;
  return HA_ERR_END_OF_FILE;
}


int ha_groonga::ft_init() {
  MRN_TRACE;
  return 0;
}

FT_INFO *ha_groonga::ft_init_ext(uint flags, uint inx,String *key)
{
  const char *match_param;
  MRN_TRACE;
  MRN_LOG(GRN_LOG_DEBUG, "-> flags=%d, inx=%d, key='%s'", flags, inx, key->ptr());
  match_param = key->ptr();
  if (flags & FT_BOOL) {
    /* boolean search */
    grn_query *query;
    this->res = grn_table_create(mrn_ctx_tls, NULL, 0, NULL, GRN_TABLE_HASH_KEY, share->obj, 0);
    query = grn_query_open(mrn_ctx_tls, match_param, strlen(match_param), GRN_SEL_OR, 32);
    grn_obj_search(mrn_ctx_tls, share->field[1]->index, (grn_obj*) query, this->res, GRN_SEL_OR, NULL);
    this->cursor = grn_table_cursor_open(mrn_ctx_tls, res, NULL, 0, NULL, 0, 0);
    //grn_query_close(mrn_ctx_tls, query);
  } else {
    /* nlq search */
    grn_obj buff;
    this->res = grn_table_create(mrn_ctx_tls, NULL, 0, NULL, GRN_TABLE_HASH_KEY, share->obj, 0);
    GRN_TEXT_INIT(&buff, 0);
    GRN_TEXT_SET(mrn_ctx_tls, &buff, match_param, strlen(match_param));
    grn_obj_search(mrn_ctx_tls, share->field[1]->index, &buff, this->res, GRN_SEL_OR, NULL);
    this->cursor = grn_table_cursor_open(mrn_ctx_tls, res, NULL, 0, NULL, 0, 0);
    //grn_obj_close(mrn_ctx_tls, &buff);
  }
  int nrec = grn_table_size(mrn_ctx_tls, res);
  MRN_LOG(GRN_LOG_DEBUG, "-> hits=%d", nrec);
  return NULL;
}

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
  if  ((id = grn_table_cursor_next(mrn_ctx_tls, this->cursor))) {
    GRN_BULK_REWIND(&buff);
    grn_table_get_key(mrn_ctx_tls, this->res, id, &docid, sizeof(grn_id));
    grn_obj_get_value(mrn_ctx_tls, share->field[1]->obj, docid, &buff);
    table->field[0]->set_notnull();
    table->field[0]->store(docid);
    table->field[1]->set_notnull();
    table->field[1]->store((char*) GRN_BULK_HEAD(&buff), GRN_TEXT_LEN(&buff), system_charset_info);
    return 0;
  }
  table->status = HA_ERR_END_OF_FILE;
  return HA_ERR_END_OF_FILE;
}

#ifdef __cplusplus
}
#endif

