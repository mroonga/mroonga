# Copyright(C) 2010 Tetsuro IKEDA
# Copyright(C) 2013-2014 Kentoku SHIBA
# Copyright(C) 2018 Kouhei Sutou <kou@clear-code.com>
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

--source ../../include/mroonga/check_ha_mroonga_so.inc
--source ../../include/mroonga/check_version.inc
--source ../../include/mroonga/check_mariadb.inc

disable_query_log;

let have_mroonga_storage_engine=`SELECT 1 FROM information_schema.plugins WHERE plugin_name = "mroonga"`;
if (!$have_mroonga_storage_engine) {
  eval INSTALL PLUGIN mroonga SONAME $ha_mroonga_so;
  eval INSTALL PLUGIN mroonga_stats SONAME $ha_mroonga_so;
}

let original_default_storage_engine=`SELECT @@global.default_storage_engine`;
set default_storage_engine=Mroonga;

if ($mariadb) {
  let have_default_tmp_storage_engine_variable=$version_10_2_or_later;
}
if (!$mariadb) {
  let have_default_tmp_storage_engine_variable=$version_5_6_or_later;
}
if ($have_default_tmp_storage_engine_variable) {
  let original_default_tmp_storage_engine=`SELECT @@global.default_tmp_storage_engine`;
  if (!$original_default_tmp_storage_engine) {
    let original_default_tmp_storage_engine=NULL;
  }
  set default_tmp_storage_engine=Mroonga;
}

enable_query_log;
