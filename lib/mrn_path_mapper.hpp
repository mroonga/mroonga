/* -*- c-basic-offset: 2 -*- */
/*
  Copyright(C) 2010 Tetsuro IKEDA
  Copyright(C) 2010-2012 Kentoku SHIBA
  Copyright(C) 2011-2012 Kouhei Sutou <kou@clear-code.com>

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

#ifndef MRN_PATH_MAPPER_HPP_
#define MRN_PATH_MAPPER_HPP_

#include "mrn_sys.hpp"

namespace mrn {
  class PathMapper {
  public:
    PathMapper(const char *mysql_path);
    const char *db_path();
    const char *db_name();
    const char *table_name();
    const char *mysql_table_name();
  private:
    const char *mysql_path_;
    char db_path_[MRN_MAX_PATH_SIZE];
    char db_name_[MRN_MAX_PATH_SIZE];
    char table_name_[MRN_MAX_PATH_SIZE];
    char mysql_table_name_[MRN_MAX_PATH_SIZE];
  };
}

#endif /* MRN_PATH_MAPPER_HPP_ */
