#include <mysql_priv.h>
#include <mysql/plugin.h>
#include <groonga.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "ha_groonga.h"

/* function definition */
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

/* variables */
static grn_ctx *mrn_ctx_sys;
static grn_hash *mrn_hash_sys;
static pthread_mutex_t *mrn_mutex_sys;
static const char *mrn_logfile_name="groonga.log";
static FILE *mrn_logfile = NULL;

static grn_logger_info mrn_logger_info = {
  GRN_LOG_DUMP,
  GRN_LOG_TIME|GRN_LOG_MESSAGE|GRN_LOG_LOCATION,
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

/* handler declaration */
struct st_mysql_storage_engine storage_engine_structure =
{ MYSQL_HANDLERTON_INTERFACE_VERSION };

mysql_declare_plugin(groonga)
{
  MYSQL_STORAGE_ENGINE_PLUGIN,
  &storage_engine_structure,
  "Groonga",
  "Tetsuro IKEDA",
  "An Embeddable Fulltext Search Engine",
  PLUGIN_LICENSE_GPL,
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
  MRN_ENTER;
  MRN_RETURN_VOID;
}

ha_groonga::~ha_groonga()
{
  MRN_ENTER;
  MRN_RETURN_VOID;
}

const char *ha_groonga::table_type() const
{
  MRN_ENTER;
  MRN_RETURN_S("Groonga");
}

const char *ha_groonga::index_type(uint inx)
{
  MRN_ENTER;
  MRN_RETURN_S("NONE");
}

static const char*ha_groonga_exts[] = {
  NullS
};
const char **ha_groonga::bas_ext() const
{
  MRN_ENTER;
  MRN_RETURN_P(ha_groonga_exts);
}

ulonglong ha_groonga::table_flags() const
{
  MRN_ENTER;
  MRN_RETURN(0);
}

ulong ha_groonga::index_flags(uint idx, uint part, bool all_parts) const
{
  MRN_ENTER;
  MRN_RETURN(0);
}

int ha_groonga::create(const char *name, TABLE *form, HA_CREATE_INFO *info)
{
  MRN_ENTER;
  char path[1024];
  char *dbname = form->s->db.str;
  char *tblname = form->s->table_name.str;
  sprintf(path, "%s/grn.db",dbname);
  grn_obj *db = grn_db_create(mrn_ctx_sys, path, NULL);
  sprintf(path, "%s/%s.grn",dbname,tblname);
  grn_obj *key_type = grn_ctx_get(mrn_ctx_sys, GRN_DB_SHORTTEXT);
  grn_obj *tbl = grn_table_create(mrn_ctx_sys,tblname,strlen(tblname),path,
				  GRN_OBJ_PERSISTENT|GRN_OBJ_TABLE_HASH_KEY,
				  key_type,1000,GRN_ENC_UTF8);
  MRN_RETURN(0);
}

int ha_groonga::open(const char *name, int mode, uint test_if_locked)
{
  MRN_ENTER;
  MRN_RETURN(0);
}

int ha_groonga::close()
{
  MRN_ENTER;
  MRN_RETURN(0);
}

int ha_groonga::info(uint flag)
{
  MRN_ENTER;
  MRN_RETURN(0);
}

THR_LOCK_DATA **ha_groonga::store_lock(THD *thd,
				    THR_LOCK_DATA **to,
				    enum thr_lock_type lock_type)
{
  MRN_ENTER;
  MRN_RETURN(to);
}

int ha_groonga::rnd_init(bool scan)
{
  MRN_ENTER;
  MRN_RETURN(0);
}

int ha_groonga::rnd_next(uchar *buf)
{
  MRN_ENTER;
  MRN_RETURN(HA_ERR_END_OF_FILE);
}

int ha_groonga::rnd_pos(uchar *buf, uchar *pos)
{
  MRN_ENTER;
  MRN_RETURN(HA_ERR_WRONG_COMMAND);
}

void ha_groonga::position(const uchar *record)
{
  MRN_ENTER;
  MRN_RETURN_VOID;
}

/* additional functions */
static bool mrn_flush_logs(handlerton *hton)
{
  MRN_ENTER;
  pthread_mutex_lock(mrn_mutex_sys);
  MRN_LOG(GRN_LOG_NOTICE, "logfile closed by FLUSH LOGS");
  fflush(mrn_logfile);
  fclose(mrn_logfile); /* reopen logfile for rotation */
  mrn_logfile = fopen(mrn_logfile_name, "a");
  MRN_LOG(GRN_LOG_NOTICE, "logfile re-opened by FLUSH LOGS");
  pthread_mutex_unlock(mrn_mutex_sys);
  MRN_RETURN(true);
}

static int mrn_init(void *p)
{
  MRN_ENTER;
  handlerton *hton;
  hton = (handlerton *)p;
  hton->state = SHOW_OPTION_YES;
  hton->create = mrn_handler_create;
  hton->flush_logs = mrn_flush_logs;
  hton->flags = 0;

  /* libgroonga init */
  grn_init();

  /* ctx init */
  mrn_ctx_sys = (grn_ctx*) MRN_MALLOC(sizeof(grn_ctx));
  grn_ctx_init(mrn_ctx_sys, GRN_CTX_USE_DB, GRN_ENC_UTF8);

  /* hash init */
  mrn_hash_sys = grn_hash_create(mrn_ctx_sys,NULL,MRN_MAX_IDENTIFIER_LEN,sizeof(size_t),
				 GRN_OBJ_KEY_VAR_SIZE, GRN_ENC_UTF8);

  /* log init */
  if (!(mrn_logfile = fopen(mrn_logfile_name, "a"))) {
    MRN_RETURN(-1);
  }
  grn_logger_info_set(mrn_ctx_sys, &mrn_logger_info);
  MRN_LOG(GRN_LOG_NOTICE, "gronnga engine started");

  /* mutex init */
  mrn_mutex_sys = (pthread_mutex_t*) MRN_MALLOC(sizeof(pthread_mutex_t));
  pthread_mutex_init(mrn_mutex_sys, MY_MUTEX_INIT_FAST);

  MRN_RETURN(0);
}

static int mrn_deinit(void *p)
{
  MRN_ENTER;

  /* mutex deinit*/
  pthread_mutex_destroy(mrn_mutex_sys);
  MRN_FREE(mrn_mutex_sys);

  /* log deinit */
  MRN_LOG(GRN_LOG_NOTICE, "stopping groonga engine");
  fclose(mrn_logfile);

  /* hash deinit */
  grn_hash_close(mrn_ctx_sys, mrn_hash_sys);

  /* ctx deinit */
  grn_ctx_fin(mrn_ctx_sys);
  MRN_FREE(mrn_ctx_sys);

  /* libgroonga deinit */
  grn_fin();

  MRN_RETURN(0);
}

static handler *mrn_handler_create(handlerton *hton,
				    TABLE_SHARE *share,
				    MEM_ROOT *root)
{
  MRN_ENTER;
  MRN_RETURN_P(new (root) ha_groonga(hton, share));
}


static void mrn_logger_func(int level, const char *time, const char *title,
		     const char *msg, const char *location, void *func_arg)
{
  const char slev[] = " EACewnid-";
  if ((mrn_logfile)) {
    fprintf(mrn_logfile, "%s|%c|%u|%s %s\n", time,
	    *(slev + level), (uint)pthread_self(), location, msg);
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


#ifdef __cplusplus
}
#endif

