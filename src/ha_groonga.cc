#include <mysql_priv.h>
#include <mysql/plugin.h>
#include <groonga.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "ha_groonga.h"

/* static function definition */
static int mrn_init(void *p);
static int mrn_deinit(void *p);
static handler *mrn_handler_create(handlerton *hton,
				   TABLE_SHARE *share,
				   MEM_ROOT *root);
static void mrn_logger_func(int level, const char *time, const char *title,
			    const char *msg, const char *location, void *func_arg);
static bool mrn_flush_logs(handlerton *hton);
static grn_encoding mrn_charset_mysql_groonga(const char *csname);
static const char *mrn_charset_groonga_mysql(grn_encoding encoding);
static void mrn_ctx_init();
static grn_obj *mrn_db_open_or_create();
static mrn_share *mrn_share_get(const char *name);
static void mrn_share_put(mrn_share *share);
static void mrn_share_remove(mrn_share *share);


/* static variables */
static grn_hash *mrn_hash_sys;
static grn_obj *mrn_db_sys;
static pthread_mutex_t *mrn_mutex_sys;
  static const char *mrn_logfile_name=MRN_LOG_FILE_NAME;
static FILE *mrn_logfile = NULL;

static grn_logger_info mrn_logger_info = {
  GRN_LOG_DUMP,
  GRN_LOG_TIME|GRN_LOG_MESSAGE,
  mrn_logger_func,
  NULL
};

static MRN_CHARSET_MAP mrn_charset_map[] = {
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

/* TLS variables */
__thread grn_ctx *mrn_ctx_tls;

/* handler declaration */
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
  mrn_init,
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
  NullS
};
const char **ha_groonga::bas_ext() const
{
  return ha_groonga_exts;
}

ulonglong ha_groonga::table_flags() const
{
  return HA_NO_TRANSACTIONS;
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

  grn_obj *key_type = grn_ctx_get(mrn_ctx_tls, GRN_DB_SHORTTEXT);
  MRN_LOG(GRN_LOG_DEBUG, "-> grn_table_create: name='%s', path='%s'",obj_name, path);
  grn_obj *table_obj = grn_table_create(mrn_ctx_tls, obj_name, strlen(obj_name), path,
				  GRN_OBJ_PERSISTENT|GRN_OBJ_TABLE_HASH_KEY,
				  key_type,1000,GRN_ENC_UTF8);
  int i;
  grn_obj *type;
  grn_obj *column_obj;
  char buf[MRN_MAX_KEY_LEN];
  for (i=0; i < form->s->fields; i++) {
    Field *field = form->s->field[i];
    switch(field->type()) {
    case MYSQL_TYPE_LONG:
      MRN_COLUMN_PATH(buf, form->s->db.str, form->s->table_name.str, field->field_name);
      type = grn_ctx_get(mrn_ctx_tls, GRN_DB_INT);
      /* NOTE: currently using NULL as path but this should be replaced by buf */
      MRN_LOG(GRN_LOG_DEBUG, "-> grn_column_create: name='%s', path='%s', type=GRN_DB_INT", 
	      field->field_name, NULL);
      column_obj = grn_column_create(mrn_ctx_tls, table_obj,
				     field->field_name, strlen(field->field_name),
				     NULL, GRN_OBJ_PERSISTENT|GRN_OBJ_COLUMN_SCALAR, type);
      grn_obj_close(mrn_ctx_tls, column_obj);
      break;
    default:
      goto err;
    }
  }

  MRN_LOG(GRN_LOG_DEBUG, "-> grn_obj_close: table_obj=%p", table_obj);
  grn_obj_close(mrn_ctx_tls, table_obj);

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

    share->fields = this->table_share->fields;
    share->field = (mrn_field**) MRN_MALLOC(sizeof(mrn_field*) * (share->fields + 1));
    int i;
    char buf[MRN_MAX_KEY_LEN];
    for (i=0; i < share->fields; i++) {
      Field *mysql_field = this->table_share->field[i];
      mrn_field *field = (mrn_field*) MRN_MALLOC(sizeof(mrn_field));
      field->name = mysql_field->field_name;
      field->name_len = strlen(field->name);
      /* NOTE: currently using NULL as path but this should be replaced */
      field->path = NULL;
      /* NOTE: currently only support INT */
      grn_obj *type = grn_ctx_get(mrn_ctx_tls, GRN_DB_INT);
      MRN_LOG(GRN_LOG_DEBUG, "-> grn_column_open: name='%s', path='%s'",
	      field->name, field->path);
      field->obj = grn_column_open(mrn_ctx_tls, share->obj,
				   field->name, field->name_len,
				   field->path, type);

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
  return 0;
}

THR_LOCK_DATA **ha_groonga::store_lock(THD *thd,
				    THR_LOCK_DATA **to,
				    enum thr_lock_type lock_type)
{
  mrn_ctx_init();
  MRN_TRACE;
  return to;
}

int ha_groonga::rnd_init(bool scan)
{
  MRN_TRACE;
  return 0;
}

int ha_groonga::rnd_next(uchar *buf)
{
  MRN_TRACE;
  return HA_ERR_END_OF_FILE;
}

int ha_groonga::rnd_pos(uchar *buf, uchar *pos)
{
  MRN_TRACE;
  return HA_ERR_WRONG_COMMAND;
}

void ha_groonga::position(const uchar *record)
{
  MRN_TRACE;
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

/* additional functions */
static bool mrn_flush_logs(handlerton *hton)
{
  MRN_TRACE;
  pthread_mutex_lock(mrn_mutex_sys);
  MRN_LOG(GRN_LOG_NOTICE, "logfile closed by FLUSH LOGS");
  fflush(mrn_logfile);
  fclose(mrn_logfile); /* reopen logfile for rotation */
  mrn_logfile = fopen(mrn_logfile_name, "a");
  MRN_LOG(GRN_LOG_NOTICE, "-------------------------------");
  MRN_LOG(GRN_LOG_NOTICE, "logfile re-opened by FLUSH LOGS");
  pthread_mutex_unlock(mrn_mutex_sys);
  return true;
}

static int mrn_init(void *p)
{
  handlerton *hton;
  hton = (handlerton *)p;
  hton->state = SHOW_OPTION_YES;
  hton->create = mrn_handler_create;
  hton->flush_logs = mrn_flush_logs;
  hton->flags = 0;

  /* libgroonga init */
  grn_init();
  mrn_ctx_init();

  /* log init */
  if (!(mrn_logfile = fopen(mrn_logfile_name, "a"))) {
    return -1;
  }
  grn_logger_info_set(mrn_ctx_tls, &mrn_logger_info);
  MRN_LOG(GRN_LOG_NOTICE, "++++++ starting mroonga ++++++");
  MRN_TRACE;

  /* init meta-data repository */
  mrn_hash_sys = grn_hash_create(mrn_ctx_tls,NULL,
				 MRN_MAX_KEY_LEN,sizeof(size_t),
				 GRN_OBJ_KEY_VAR_SIZE, GRN_ENC_UTF8);
  mrn_db_sys = mrn_db_open_or_create();

  /* mutex init */
  mrn_mutex_sys = (pthread_mutex_t*) MRN_MALLOC(sizeof(pthread_mutex_t));
  pthread_mutex_init(mrn_mutex_sys, MY_MUTEX_INIT_FAST);

  return 0;
}

/*
  TODO: release all grn_obj in global hash
*/
static int mrn_deinit(void *p)
{
  mrn_ctx_init();
  MRN_TRACE;

  MRN_LOG(GRN_LOG_DEBUG, "-> grn_obj_close: '%s'", MRN_DB_FILE_PATH);
  grn_obj_close(mrn_ctx_tls, mrn_db_sys);

  /* mutex deinit*/
  pthread_mutex_destroy(mrn_mutex_sys);
  MRN_FREE(mrn_mutex_sys);

  /* log deinit */
  MRN_LOG(GRN_LOG_NOTICE, "------ stopping mroonga ------");
  fclose(mrn_logfile);
  mrn_logfile = NULL;

  /* hash deinit */
  grn_hash_close(mrn_ctx_tls, mrn_hash_sys);

  /* libgroonga deinit */
  grn_fin();

  return 0;
}

static handler *mrn_handler_create(handlerton *hton,
				    TABLE_SHARE *share,
				    MEM_ROOT *root)
{
  return (new (root) ha_groonga(hton, share));
}


static void mrn_logger_func(int level, const char *time, const char *title,
		     const char *msg, const char *location, void *func_arg)
{
  const char slev[] = " EACewnid-";
  if ((mrn_logfile)) {
    fprintf(mrn_logfile, "%s|%c|%u|%s\n", time,
	    *(slev + level), (uint)pthread_self(), msg);
    fflush(mrn_logfile);
  }
}


static grn_encoding mrn_charset_mysql_groonga(const char *csname)
{
  if (!csname) return GRN_ENC_NONE;
  int i;
  for (i = 0; mrn_charset_map[i].csname_mysql; i++) {
    if (!(my_strcasecmp(system_charset_info, csname,
			mrn_charset_map[i].csname_mysql)))
      return mrn_charset_map[i].csname_groonga;
  }
  return GRN_ENC_NONE;
}

static const char *mrn_charset_groonga_mysql(grn_encoding encoding)
{
  int i;
  for (i = 0; (mrn_charset_map[i].csname_groonga != GRN_ENC_DEFAULT); i++) {
    if (mrn_charset_map[i].csname_groonga == encoding)
      return mrn_charset_map[i].csname_mysql;
  }
  return NULL;
}

static void mrn_ctx_init()
{
  if (mrn_ctx_tls == NULL) {
    mrn_ctx_tls = (grn_ctx*) MRN_MALLOC(sizeof(grn_ctx));
    grn_ctx_init(mrn_ctx_tls, 0, GRN_ENC_UTF8);
    if ((mrn_db_sys))
      grn_ctx_use(mrn_ctx_tls, mrn_db_sys);
  }
}

static grn_obj *mrn_db_open_or_create()
{
  grn_obj *obj;
  struct stat dummy;
  if ((stat(MRN_DB_FILE_PATH, &dummy))) { // check if file not exists
    MRN_LOG(GRN_LOG_DEBUG, "-> grn_db_create: '%s'", MRN_DB_FILE_PATH);
    obj = grn_db_create(mrn_ctx_tls, MRN_DB_FILE_PATH, NULL);
  } else {
    MRN_LOG(GRN_LOG_DEBUG, "-> grn_db_open: '%s'", MRN_DB_FILE_PATH);
    obj = grn_db_open(mrn_ctx_tls, MRN_DB_FILE_PATH);
  }
  return obj;
}

static void mrn_share_put(mrn_share *share)
{
  void *value;
  grn_search_flags flags = GRN_TABLE_ADD;
  /* TODO: check duplication */
  MRN_LOG(GRN_LOG_DEBUG,"-> grn_hash_lookup(put): name='%s'", share->name);
  grn_hash_lookup(mrn_ctx_tls, mrn_hash_sys, share->name,
		  strlen(share->name), &value, &flags);
  memcpy(value, share, sizeof(share));
}

/* returns NULL if specified obj_name is not found in grn_hash */
static mrn_share *mrn_share_get(const char *name)
{
  void *value;
  grn_search_flags flags = 0;
  MRN_LOG(GRN_LOG_DEBUG,"-> grn_hash_lookup(get): name='%s'", name);
  grn_id rid = grn_hash_lookup(mrn_ctx_tls, mrn_hash_sys, name,
			       strlen(name), &value, &flags);
  if (rid == 0) {
    return NULL;
  } else {
    return (mrn_share*) value;
  }
}

static void mrn_share_remove(mrn_share *share)
{
  /* TODO: check return value */
  MRN_LOG(GRN_LOG_DEBUG, "-> grn_hash_delete: name='%s'", share->name);
  grn_hash_delete(mrn_ctx_tls, mrn_hash_sys, share->name,
		  strlen(share->name), NULL);
}

static void mrn_share_remove_all()
{
  /* TODO: implement this function by using GRN_HASH_EACH */
}

#define LOG_FIELD(x) MRN_LOG(GRN_LOG_DEBUG, "-> %s %s", field->field_name, x); break;

static void mrn_print_field_type(Field *field)
{
  switch (field->type()) {
  case MYSQL_TYPE_DECIMAL:
    LOG_FIELD("MYSQL_TYPE_DECIMAL");
  case MYSQL_TYPE_TINY:
    LOG_FIELD("MYSQL_TYPE_TINY");
  case MYSQL_TYPE_SHORT:
    LOG_FIELD("MYSQL_TYPE_SHORT");
  case MYSQL_TYPE_LONG:
    LOG_FIELD("MYSQL_TYPE_LONG");
  case MYSQL_TYPE_FLOAT:
    LOG_FIELD("MYSQL_TYPE_FLOAT");
  case MYSQL_TYPE_DOUBLE:
    LOG_FIELD("MYSQL_TYPE_DOUBLE");
  case MYSQL_TYPE_NULL:
    LOG_FIELD("MYSQL_TYPE_NULL");
  case MYSQL_TYPE_TIMESTAMP:
    LOG_FIELD("MYSQL_TYPE_TIMESTAMP");
  case MYSQL_TYPE_LONGLONG:
    LOG_FIELD("MYSQL_TYPE_LONGLONG");
  case MYSQL_TYPE_INT24:
    LOG_FIELD("MYSQL_TYPE_INT24");
  case MYSQL_TYPE_DATE:
    LOG_FIELD("MYSQL_TYPE_DATE");
  case MYSQL_TYPE_TIME:
    LOG_FIELD("MYSQL_TYPE_TIME");
  case MYSQL_TYPE_DATETIME:
    LOG_FIELD("MYSQL_TYPE_DATETIME");
  case MYSQL_TYPE_YEAR:
    LOG_FIELD("MYSQL_TYPE_YEAR");
  case MYSQL_TYPE_NEWDATE:
    LOG_FIELD("MYSQL_TYPE_NEWDATE");
  case MYSQL_TYPE_VARCHAR:
    LOG_FIELD("MYSQL_TYPE_VARCHAR");
  case MYSQL_TYPE_BIT:
    LOG_FIELD("MYSQL_TYPE_BIT");
  case MYSQL_TYPE_NEWDECIMAL:
    LOG_FIELD("MYSQL_TYPE_NEWDECIMAL");
  case MYSQL_TYPE_ENUM:
    LOG_FIELD("MYSQL_TYPE_ENUM");
  case MYSQL_TYPE_SET:
    LOG_FIELD("MYSQL_TYPE_SET");
  case MYSQL_TYPE_TINY_BLOB:
    LOG_FIELD("MYSQL_TYPE_TINY_BLOB");
  case MYSQL_TYPE_MEDIUM_BLOB:
    LOG_FIELD("MYSQL_TYPE_MEDIUM_BLOB");
  case MYSQL_TYPE_LONG_BLOB:
    LOG_FIELD("MYSQL_TYPE_LONG_BLOB");
  case MYSQL_TYPE_BLOB:
    LOG_FIELD("MYSQL_TYPE_BLOB");
  case MYSQL_TYPE_VAR_STRING:
    LOG_FIELD("MYSQL_TYPE_VAR_STRING");
  case MYSQL_TYPE_STRING:
    LOG_FIELD("MYSQL_TYPE_STRING");
  case MYSQL_TYPE_GEOMETRY:
    LOG_FIELD("MYSQL_TYPE_GEOMETRY");
  }
}

#ifdef __cplusplus
}
#endif

