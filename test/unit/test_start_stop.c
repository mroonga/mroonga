#include <gcutter.h>
#include <glib/gstdio.h>
#include "driver.h"

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

void test_mrn_init()
{
  mrn_system_db = NULL;
  mrn_system_hash = NULL;

  cut_assert_equal_int(0, mrn_init(0));
  cut_assert_not_null(mrn_logfile);
  cut_assert_not_null(mrn_system_db);
  cut_assert_not_null(mrn_system_hash);
  cut_assert_not_null(mrn_lock);
  cut_assert_equal_int(0, pthread_mutex_lock(mrn_lock));
  cut_assert_equal_int(0, pthread_mutex_unlock(mrn_lock));

  mrn_deinit();
}

void test_mrn_deinit()
{
  cut_assert_equal_int(0, mrn_init(0));
  cut_assert_equal_int(0, mrn_deinit());
  cut_assert_null(mrn_logfile);
  cut_assert_null(mrn_system_db);
  cut_assert_null(mrn_system_hash);
  cut_assert_null(mrn_lock);
}
