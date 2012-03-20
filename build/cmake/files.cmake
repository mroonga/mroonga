# Copyright(C) 2012 Kouhei Sutou <kou@clear-code.com>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

set(MROONGA_C_SOURCES
  mrn_sys.c
  mrn_sys.h

  mrn_macro.h
  mrn_err.h
  mrn_mysql.h
  mrn_mysql_compat.h
  )
set(MROONGA_CPP_SOURCES
  ha_mroonga.cc
  ha_mroonga.h
  mrn_table.cc
  mrn_table.h
  lib/mrn_path_mapper.cpp
  lib/mrn_path_mapper.hpp
  )
set(MROONGA_SOURCES ${MROONGA_C_SOURCES} ${MROONGA_CPP_SOURCES})
