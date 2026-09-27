/* -*- c-basic-offset: 2 -*- */
/*
  Copyright(C) 2010 Tetsuro IKEDA
  Copyright(C) 2011-2013 Kentoku SHIBA
  Copyright(C) 2011-2015 Kouhei Sutou <kou@clear-code.com>

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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <mrn_mysql.h>
#include <mrn_constants.hpp>

#include "mrn_path_mapper.hpp"

#include <string.h>
#include <cstring>

namespace mrn {
  char *PathMapper::default_path_prefix = NULL;
  char *PathMapper::default_mysql_data_home_path = NULL;

  PathMapper::PathMapper(const char *original_mysql_path,
                         const char *path_prefix,
                         const char *mysql_data_home_path)
    : original_mysql_path_(original_mysql_path),
      path_prefix_(path_prefix),
      mysql_data_home_path_(mysql_data_home_path),
      db_path_(),
      db_name_(),
      table_name_(),
      mysql_table_name_(),
      mysql_path_() {
  }

  /**
   * "./${db}/${table}"                              ==> "${db}.mrn"
   * "./${db}/"                                      ==> "${db}.mrn"
   * "/tmp/mysql-test/var/tmp/mysqld.1/#sql27c5_1_0" ==>
   *   "/tmp/mysql-test/var/tmp/mysqld.1/#sql27c5_1_0.mrn"
   */
  const char *PathMapper::db_path() {
    if (!db_path_.empty()) {
      return db_path_.c_str();
    }

    if (original_mysql_path_[0] == FN_CURLIB &&
        original_mysql_path_[1] == FN_LIBCHAR) {
      if (path_prefix_) {
        db_path_ = path_prefix_;
      }
      const char *db_name = original_mysql_path_ + 2;
      const char *db_name_end = strchr(db_name, FN_LIBCHAR);
      if (db_name_end) {
        db_path_.append(db_name, db_name_end - db_name);
      } else {
        db_path_ += db_name;
      }
    } else if (mysql_data_home_path_) {
      size_t mysql_data_home_length = strlen(mysql_data_home_path_);
      const char *db_name = original_mysql_path_ + mysql_data_home_length;
      const char *db_name_end = nullptr;
      if (strlen(original_mysql_path_) > mysql_data_home_length &&
          strncmp(original_mysql_path_,
                  mysql_data_home_path_,
                  mysql_data_home_length) == 0) {
        db_name_end = strchr(db_name, FN_LIBCHAR);
      }
      if (db_name_end) {
        if (path_prefix_ && path_prefix_[0] == FN_LIBCHAR) {
          db_path_ = path_prefix_;
        } else {
          db_path_.assign(mysql_data_home_path_, mysql_data_home_length);
          if (path_prefix_) {
            if (path_prefix_[0] == FN_CURLIB &&
                path_prefix_[1] == FN_LIBCHAR) {
              db_path_ += path_prefix_ + 2;
            } else {
              db_path_ += path_prefix_;
            }
          }
        }
        db_path_.append(db_name, db_name_end - db_name);
      } else {
        db_path_ = original_mysql_path_;
      }
    } else {
      db_path_ = original_mysql_path_;
    }
    db_path_ += MRN_DB_FILE_SUFFIX;
    return db_path_.c_str();
  }

  /**
   * "./${db}/${table}"                              ==> "${db}"
   * "./${db}/"                                      ==> "${db}"
   * "/tmp/mysql-test/var/tmp/mysqld.1/#sql27c5_1_0" ==>
   *   "/tmp/mysql-test/var/tmp/mysqld.1/#sql27c5_1_0"
   */
  const char *PathMapper::db_name() {
    if (!db_name_.empty()) {
      return db_name_.c_str();
    }

    if (original_mysql_path_[0] == FN_CURLIB &&
        original_mysql_path_[1] == FN_LIBCHAR) {
      const char *db_name = original_mysql_path_ + 2;
      const char *db_name_end = strchr(db_name, FN_LIBCHAR);
      if (db_name_end) {
        db_name_.assign(db_name, db_name_end - db_name);
      } else {
        db_name_ = db_name;
      }
    } else if (mysql_data_home_path_) {
      size_t mysql_data_home_length = strlen(mysql_data_home_path_);
      const char *db_name = original_mysql_path_ + mysql_data_home_length;
      const char *db_name_end = nullptr;
      if (strlen(original_mysql_path_) > mysql_data_home_length &&
          strncmp(original_mysql_path_,
                  mysql_data_home_path_,
                  mysql_data_home_length) == 0) {
        db_name_end = strchr(db_name, FN_LIBCHAR);
      }
      if (db_name_end) {
        db_name_.assign(db_name, db_name_end - db_name);
      } else {
        db_name_ = original_mysql_path_;
      }
    } else {
      db_name_ = original_mysql_path_;
    }
    return db_name_.c_str();
  }

  /**
   * "./${db}/${table}" ==> "${table}" (with encoding first '_')
   */
  const char *PathMapper::table_name() {
    if (!table_name_.empty()) {
      return table_name_.c_str();
    }

    const char *separator = strrchr(original_mysql_path_, FN_LIBCHAR);
    const char *table_name = separator ? separator + 1 : original_mysql_path_;
    if (table_name[0] == '_') {
      table_name_ = "@005f";
      table_name++;
    }
    table_name_ += table_name;
    return table_name_.c_str();
  }

  /**
   * "./${db}/${table}" ==> "${table}" (without encoding first '_')
   */
  const char *PathMapper::mysql_table_name() {
    if (!mysql_table_name_.empty()) {
      return mysql_table_name_.c_str();
    }

    const char *separator = strrchr(original_mysql_path_, FN_LIBCHAR);
    const char *table_name = separator ? separator + 1 : original_mysql_path_;
    const char *partition = strstr(table_name, "#P#");
    if (partition) {
      mysql_table_name_.assign(table_name, partition - table_name);
    } else {
      mysql_table_name_ = table_name;
    }
    return mysql_table_name_.c_str();
  }

  /**
   * "./${db}/${table}"       ==> "./${db}/${table}"
   * "./${db}/${table}#P#xxx" ==> "./${db}/${table}"
   */
  const char *PathMapper::mysql_path() {
    if (!mysql_path_.empty()) {
      return mysql_path_.c_str();
    }

    const char *partition = strstr(original_mysql_path_, "#P#");
    if (partition) {
      mysql_path_.assign(original_mysql_path_,
                         partition - original_mysql_path_);
    } else {
      mysql_path_ = original_mysql_path_;
    }
    return mysql_path_.c_str();
  }

  bool PathMapper::is_internal_table_name() {
    return mysql_table_name()[0] == '#';
  }
}
