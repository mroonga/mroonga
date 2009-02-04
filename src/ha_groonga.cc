#include <mysql_priv.h>
#include <mysql/plugin.h>
#include <groonga/groonga.h>
#include "ha_groonga.h"

#ifdef __cplusplus
extern "C" {
#endif

static int mrn_init(void *p);
static int mrn_fin(void *p);
static handler *mrn_handler_create(handlerton *hton,
				   TABLE_SHARE *share,
				   MEM_ROOT *root);

struct st_mysql_storage_engine storage_engine_structure =
{ MYSQL_HANDLERTON_INTERFACE_VERSION };

mysql_declare_plugin(mroonga)
{
  MYSQL_STORAGE_ENGINE_PLUGIN,
  &storage_engine_structure,
  "mroonga",
  "Tetsuro IKEDA",
  "groonga storage engine",
  PLUGIN_LICENSE_GPL,
  mrn_init,
  mrn_fin,
  0x0001,
  NULL,
  NULL,
  NULL
}
mysql_declare_plugin_end;

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
  MRN_RETURN_S("MROONGA");
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
  MRN_RETURN(0);
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

static int mrn_init(void *p)
{
  MRN_ENTER;
  handlerton *hton;
  hton = (handlerton *)p;
  hton->state = SHOW_OPTION_YES;
  hton->create = mrn_handler_create;
  hton->flags = 0;
  grn_init();
  MRN_RETURN(0);
}

static int mrn_fin(void *p)
{
  MRN_ENTER;
  MRN_RETURN(0);
  grn_fin();
}

static handler *mrn_handler_create(handlerton *hton,
				    TABLE_SHARE *share,
				    MEM_ROOT *root)
{
  MRN_ENTER;
  MRN_RETURN_P(new (root) ha_groonga(hton, share));
}

#ifdef __cplusplus
}
#endif
