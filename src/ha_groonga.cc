#include <mysql_priv.h>
#include <mysql/plugin.h>
#include "ha_yase.h"

static int yase_init_plugin(void *p);
static int yase_deinit_plugin(void *p);
static handler *yase_create_handler(handlerton *hton,
				    TABLE_SHARE *share,
				    MEM_ROOT *root);

struct st_mysql_storage_engine storage_engine_structure =
{ MYSQL_HANDLERTON_INTERFACE_VERSION };

mysql_declare_plugin(yase)
{
  MYSQL_STORAGE_ENGINE_PLUGIN,
  &storage_engine_structure,
  "YASE",
  "Tetsuro IKEDA, Tritonn Project",
  "Yet Another Skeleton Engine",
  PLUGIN_LICENSE_GPL,
  yase_init_plugin,
  yase_deinit_plugin,
  0x0001,
  NULL,
  NULL,
  NULL
}
mysql_declare_plugin_end;

ha_yase::ha_yase(handlerton *hton, TABLE_SHARE *share)
  :handler(hton, share)
{
  DBUG_ENTER("ha_yase::ha_yase");
  DBUG_VOID_RETURN;
}

ha_yase::~ha_yase()
{
  DBUG_ENTER("ha_yase::~ha_yase");
  DBUG_VOID_RETURN;
}

const char *ha_yase::table_type() const
{
  DBUG_ENTER("ha_yase::table_type");
  DBUG_RETURN("YASE");
}

static const char*ha_yase_exts[] = {
  NullS
};
const char **ha_yase::bas_ext() const
{
  DBUG_ENTER("ha_yase::bas_ext");
  DBUG_RETURN(ha_yase_exts);
}

ulonglong ha_yase::table_flags() const
{
  DBUG_ENTER("ha_yase::table_flags");
  DBUG_RETURN(0);
}

ulong ha_yase::index_flags(uint idx, uint part, bool all_parts) const
{
  DBUG_ENTER("ha_yase::index_flags");
  DBUG_RETURN(0);
}

int ha_yase::create(const char *name, TABLE *form, HA_CREATE_INFO *info)
{
  DBUG_ENTER("ha_yase::create");
  DBUG_RETURN(0);
}

int ha_yase::open(const char *name, int mode, uint test_if_locked)
{
  DBUG_ENTER("ha_yase::open");
  DBUG_RETURN(0);
}

int ha_yase::close()
{
  DBUG_ENTER("ha_yase::close");
  DBUG_RETURN(0);
}

int ha_yase::info(uint flag)
{
  DBUG_ENTER("ha_yase::info");
  DBUG_RETURN(0);
}

THR_LOCK_DATA **ha_yase::store_lock(THD *thd,
				    THR_LOCK_DATA **to,
				    enum thr_lock_type lock_type)
{
  DBUG_ENTER("ha_yase::store_locko");
  DBUG_RETURN(to);
}

int ha_yase::rnd_init(bool scan)
{
  DBUG_ENTER("ha_yase::rnd_init");
  DBUG_RETURN(0);
}

int ha_yase::rnd_next(uchar *buf)
{
  DBUG_ENTER("ha_yase::rnd_next");
  DBUG_RETURN(HA_ERR_END_OF_FILE);
}

int ha_yase::rnd_pos(uchar *buf, uchar *pos)
{
  DBUG_ENTER("ha_yase::rnd_pos");
  DBUG_RETURN(HA_ERR_WRONG_COMMAND);
}

void ha_yase::position(const uchar *record)
{
  DBUG_ENTER("ha_yase::position");
  DBUG_VOID_RETURN;
}

static int yase_init_plugin(void *p)
{
  DBUG_ENTER("yase_init_plugin");
  handlerton *hton;
  hton = (handlerton *)p;
  hton->state = SHOW_OPTION_YES;
  hton->create = yase_create_handler;
  hton->flags = 0;
  DBUG_RETURN(0);
}

static int yase_deinit_plugin(void *p)
{
  DBUG_ENTER("yase_deinit_plugin");
  DBUG_RETURN(0);
}

static handler *yase_create_handler(handlerton *hton,
				    TABLE_SHARE *share,
				    MEM_ROOT *root)
{
  DBUG_ENTER("yase_create_handler");
  DBUG_RETURN(new (root) ha_yase(hton, share));
}
