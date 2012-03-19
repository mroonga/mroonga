/* -*- c-basic-offset: 2 -*- */
/*
  Copyright(C) 2012 Kouhei Sutou <kou@clear-code.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <string.h>
#include <cppcutter.h>

#include <mrn_path_mapper.hpp>

namespace test_mrn_path_mapper {
  namespace db_path {
    void test_normal_db() {
      mrn::PathMapper mapper("./db/");
      cppcut_assert_equal("db.mrn", mapper.db_path());
    }

    void test_normal_table() {
      mrn::PathMapper mapper("./db/table");
      cppcut_assert_equal("db.mrn", mapper.db_path());
    }

    void test_temporary_table() {
      mrn::PathMapper mapper("/tmp/mysqld.1/#sql27c5_1_0");
      cppcut_assert_equal("/tmp/mysqld.1/#sql27c5_1_0.mrn",
                          mapper.db_path());
    }
  }

  namespace db_name {
    void test_normal_db() {
      mrn::PathMapper mapper("./db/");
      cppcut_assert_equal("db", mapper.db_name());
    }

    void test_normal_table() {
      mrn::PathMapper mapper("./db/table");
      cppcut_assert_equal("db", mapper.db_name());
    }

    void test_temporary_table() {
      mrn::PathMapper mapper("/tmp/mysqld.1/#sql27c5_1_0");
      cppcut_assert_equal("/tmp/mysqld.1/#sql27c5_1_0",
                          mapper.db_name());
    }
  }
}

