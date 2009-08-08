#include <gcutter.h>
#include <glib/gstdio.h>
#include <groonga.h>

static gchar *base_directory;
static gchar *tmp_directory;
static grn_ctx *ctx;

void cut_startup()
{
  base_directory = g_get_current_dir();
  tmp_directory = g_build_filename(base_directory, "/tmp", NULL);
  cut_remove_path(tmp_directory, NULL);
  g_mkdir_with_parents(tmp_directory, 0700);
  g_chdir(tmp_directory);
}

void cut_shutdown()
{
  g_chdir(base_directory);
  g_free(tmp_directory);
}
