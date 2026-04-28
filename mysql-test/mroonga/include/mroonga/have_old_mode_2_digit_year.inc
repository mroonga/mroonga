# -*- mode: sql; sql-product: mysql -*-
#
# Copyright(C) 2026  Horimoto Yasuhiro <horimoto@clear-code.com>
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
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

--source ../../include/mroonga/check_version.inc
--source ../../include/mroonga/check_mariadb.inc

--disable_query_log
if ($mariadb) {
  if ($version_13_0_or_later) {
--disable_warnings
    set old_mode=2_DIGIT_YEAR;
--enable_warnings
  }
}
--enable_query_log
