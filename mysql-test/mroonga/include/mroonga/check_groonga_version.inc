# -*- mode: sql; sql-product: mysql -*-
#
# Copyright(C) 2021  Sutou Kouhei <kou@clear-code.com>
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

--disable_query_log
--enable_query_log
let $groonga_version = `SELECT @@mroonga_libgroonga_version;`;
let $groonga_version_major =
  `SELECT CAST(SUBSTRING_INDEX(@@mroonga_libgroonga_version, '.', 1) AS UNSIGNED);`;
let $groonga_version_minor =
  `SELECT CAST(SUBSTRING_INDEX(
     SUBSTRING_INDEX(@@mroonga_libgroonga_version, '.', 2),
     '.',
     -1)
   AS UNSIGNED);`;
let $groonga_version_micro =
  `SELECT CAST(
     SUBSTRING_INDEX(
       SUBSTRING_INDEX(
	 SUBSTRING_INDEX(@@mroonga_libgroonga_version, '.', 3),
	 '.',
	 -1
       ),
       '-',
       1
     )
   AS UNSIGNED);`;

let $groonga_version_11_0_1_or_later =
  `SELECT (($groonga_version_major * 100) +
           ($groonga_version_minor * 10) +
           ($groonga_version_micro)) >= 1101;`;
--enable_query_log
