# Copyright(C) 2012-2018 Kouhei Sutou <kou@clear-code.com>
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
let $version_major =
  `SELECT CAST(SUBSTRING_INDEX(@@global.version, '.', 1) AS UNSIGNED)`;
let $version_minor =
  `SELECT CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(@@global.version, '.', 2), '.', -1) AS UNSIGNED)`;
let $version_major_minor = 
  `SELECT $version_major * 100 + $version_minor`;

let $version_5_5           = `SELECT $version_major_minor = 505`;
let $version_5_6           = `SELECT $version_major_minor = 506`;
let $version_5_7           = `SELECT $version_major_minor = 507`;
let $version_8_0           = `SELECT $version_major_minor = 800`;
let $version_10_0          = `SELECT $version_major_minor = 1000`;
let $version_10_1          = `SELECT $version_major_minor = 1001`;
let $version_10_2          = `SELECT $version_major_minor = 1002`;
let $version_10_3          = `SELECT $version_major_minor = 1003`;
let $version_10_4          = `SELECT $version_major_minor = 1004`;
let $version_10_5          = `SELECT $version_major_minor = 1005`;
let $version_10_6          = `SELECT $version_major_minor = 1006`;

let $version_5_5_or_later  = `SELECT $version_major_minor >= 505`;
let $version_5_6_or_later  = `SELECT $version_major_minor >= 506`;
let $version_5_7_or_later  = `SELECT $version_major_minor >= 507`;
let $version_8_0_or_later  = `SELECT $version_major_minor >= 800`;
let $version_10_0_or_later = `SELECT $version_major_minor >= 1000`;
let $version_10_2_or_later = `SELECT $version_major_minor >= 1002`;
let $version_10_3_or_later = `SELECT $version_major_minor >= 1003`;
let $version_10_4_or_later = `SELECT $version_major_minor >= 1004`;
let $version_10_5_or_later = `SELECT $version_major_minor >= 1005`;
--enable_query_log
