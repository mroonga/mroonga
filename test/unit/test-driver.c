#include <cutter.h>
#include "driver.h"

void test_mrn_init()
{
  grn_fin();

  mrn_db = NULL;
  mrn_hash = NULL;
  mrn_lexicon = NULL;

  cut_assert_equal_int(0, mrn_init());
  cut_assert_not_null(mrn_db);
  cut_assert_not_null(mrn_hash);
  cut_assert_not_null(mrn_lexicon);

  grn_fin();
}
