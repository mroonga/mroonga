#include <mysql_priv.h>
#include <mysql/plugin.h>
#include "ha_groonga.h"

static int groonga_init_plugin(void *p);
static int groonga_deinit_plugin(void *p);
static handler *groonga_create_handler(handlerton *hton,
				    TABLE_SHARE *share,
				    MEM_ROOT *root);

struct st_mysql_storage_engine storage_engine_structure =
{ MYSQL_HANDLERTON_INTERFACE_VERSION };

mysql_declare_plugin(groonga)
{
  MYSQL_STORAGE_ENGINE_PLUGIN,
  &storage_engine_structure,
  "Groonga",
  "Tetsuro IKEDA, Tritonn Project",
  "Full inverted index, column oriented storage engine",
  PLUGIN_LICENSE_GPL,
  groonga_init_plugin,
  groonga_deinit_plugin,
  0x0001,
  NULL,
  NULL,
  NULL
}
mysql_declare_plugin_end;

ha_groonga::ha_groonga(handlerton *hton, TABLE_SHARE *share)
  :handler(hton, share)
{
  DBUG_ENTER("ha_groonga::ha_groonga");
  DBUG_VOID_RETURN;
}

ha_groonga::~ha_groonga()
{
  DBUG_ENTER("ha_groonga::~ha_groonga");
  DBUG_VOID_RETURN;
}

const char *ha_groonga::table_type() const
{
  DBUG_ENTER("ha_groonga::table_type");
  DBUG_RETURN("GROONGA");
}

static const char*ha_groonga_exts[] = {
  NullS
};
const char **ha_groonga::bas_ext() const
{
  DBUG_ENTER("ha_groonga::bas_ext");
  DBUG_RETURN(ha_groonga_exts);
}

ulonglong ha_groonga::table_flags() const
{
  DBUG_ENTER("ha_groonga::table_flags");
  DBUG_RETURN(0);
}

ulong ha_groonga::index_flags(uint idx, uint part, bool all_parts) const
{
  DBUG_ENTER("ha_groonga::index_flags");
  DBUG_RETURN(0);
}

int ha_groonga::create(const char *name, TABLE *form, HA_CREATE_INFO *info)
{
  DBUG_ENTER("ha_groonga::create");
  DBUG_RETURN(0);
}

int ha_groonga::open(const char *name, int mode, uint test_if_locked)
{
  DBUG_ENTER("ha_groonga::open");
  DBUG_RETURN(0);
}

int ha_groonga::close()
{
  DBUG_ENTER("ha_groonga::close");
  DBUG_RETURN(0);
}

int ha_groonga::info(uint flag)
{
  DBUG_ENTER("ha_groonga::info");
  DBUG_RETURN(0);
}

THR_LOCK_DATA **ha_groonga::store_lock(THD *thd,
				    THR_LOCK_DATA **to,
				    enum thr_lock_type lock_type)
{
  DBUG_ENTER("ha_groonga::store_lock");
  DBUG_RETURN(to);
}

int ha_groonga::rnd_init(bool scan)
{
  DBUG_ENTER("ha_groonga::rnd_init");
  DBUG_RETURN(0);
}

int ha_groonga::rnd_next(uchar *buf)
{
  DBUG_ENTER("ha_groonga::rnd_next");
  DBUG_RETURN(HA_ERR_END_OF_FILE);
}

int ha_groonga::rnd_pos(uchar *buf, uchar *pos)
{
  DBUG_ENTER("ha_groonga::rnd_pos");
  DBUG_RETURN(HA_ERR_WRONG_COMMAND);
}

void ha_groonga::position(const uchar *record)
{
  DBUG_ENTER("ha_groonga::position");
  DBUG_VOID_RETURN;
}

static int groonga_init_plugin(void *p)
{
  DBUG_ENTER("groonga_init_plugin");
  handlerton *hton;
  hton = (handlerton *)p;
  hton->state = SHOW_OPTION_YES;
  hton->create = groonga_create_handler;
  hton->flags = 0;
  DBUG_RETURN(0);
}

static int groonga_deinit_plugin(void *p)
{
  DBUG_ENTER("groonga_deinit_plugin");
  DBUG_RETURN(0);
}

static handler *groonga_create_handler(handlerton *hton,
				    TABLE_SHARE *share,
				    MEM_ROOT *root)
{
  DBUG_ENTER("groonga_create_handler");
  DBUG_RETURN(new (root) ha_groonga(hton, share));
}
