# Copyright(C) 2012-2018 Kouhei Sutou <kou@clear-code.com>
# Copyright(C) 2022 Horimoto Yasuhiro <horimoto@clear-code.com>
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
let $version_major = `SELECT CAST(SUBSTRING_INDEX(@@global.version, '.', 1) AS UNSIGNED);`;
let $version_minor =
  `SELECT CAST(SUBSTRING_INDEX(
     SUBSTRING_INDEX(@@global.version, '.', 2),
     '.',
     -1)
   AS UNSIGNED);`;
let $version_micro =
  `SELECT CAST(SUBSTRING_INDEX(
     SUBSTRING_INDEX(@@global.version, '-', 1),
     '.',
     -1)
   AS UNSIGNED);`;

let $version_major_minor =
  `SELECT CAST(SUBSTRING_INDEX(@@global.version, '.', 2) AS DECIMAL(4, 2))`;

let $version_5_5           = `SELECT $version_major_minor = 5.5`;
let $version_5_6           = `SELECT $version_major_minor = 5.6`;
let $version_5_7           = `SELECT $version_major_minor = 5.7`;
let $version_8_0           = `SELECT $version_major_minor = 8.0`;
let $version_10_0          = `SELECT $version_major_minor = 10.0`;
let $version_10_1          = `SELECT $version_major_minor = 10.1`;
let $version_10_2          = `SELECT $version_major_minor = 10.2`;
let $version_10_3          = `SELECT $version_major_minor = 10.3`;
let $version_10_4          = `SELECT $version_major_minor = 10.4`;
let $version_10_5          = `SELECT $version_major_minor = 10.5`;
let $version_10_6          = `SELECT $version_major_minor = 10.6`;

let $version_5_5_or_later  = `SELECT $version_major_minor >= 5.5`;
let $version_5_6_or_later  = `SELECT $version_major_minor >= 5.6`;
let $version_5_7_or_later  = `SELECT $version_major_minor >= 5.7`;
let $version_8_0_or_later  = `SELECT $version_major_minor >= 8.0`;
let $version_10_0_or_later = `SELECT $version_major_minor >= 10.0`;
let $version_10_2_or_later = `SELECT $version_major_minor >= 10.2`;
let $version_10_3_or_later = `SELECT $version_major_minor >= 10.3`;
let $version_10_4_or_later = `SELECT $version_major_minor >= 10.4`;
let $version_10_5_or_later = `SELECT $version_major_minor >= 10.5`;

let $version_10_3_37_or_later =
  `SELECT ((($version_major * 10000) +
           ($version_minor * 100) +
           ($version_micro)) >= 100337) &&
          ((($version_major * 10000) +
           ($version_minor * 100) +
           ($version_micro)) < 100400);`;
let $version_10_4_27_or_later =
  `SELECT (($version_major * 10000) +
           ($version_minor * 100) +
           ($version_micro)) >= 100427 &&
          (($version_major * 10000) +
           ($version_minor * 100) +
           ($version_micro)) < 100500;`;
let $version_10_5_18_or_later =
  `SELECT (($version_major * 10000) +
           ($version_minor * 100) +
           ($version_micro)) >= 100518 &&
          (($version_major * 10000) +
           ($version_minor * 100) +
           ($version_micro)) < 100600;`;
let $version_10_6_11_or_later =
  `SELECT (($version_major * 10000) +
           ($version_minor * 100) +
           ($version_micro)) >= 100611 &&
          (($version_major * 10000) +
           ($version_minor * 100) +
           ($version_micro)) < 100700;`;
let $version_10_7_7_or_later =
  `SELECT (($version_major * 10000) +
           ($version_minor * 100) +
           ($version_micro)) >= 100707 &&
          (($version_major * 10000) +
           ($version_minor * 100) +
           ($version_micro)) < 100800;`;
let $version_10_8_6_or_later =
  `SELECT (($version_major * 10000) +
           ($version_minor * 100) +
           ($version_micro)) >= 100806 &&
          (($version_major * 10000) +
           ($version_minor * 100) +
           ($version_micro)) < 100900;`;
let $version_10_9_4_or_later =
  `SELECT (($version_major * 10000) +
           ($version_minor * 100) +
           ($version_micro)) >= 100904 &&
          (($version_major * 10000) +
           ($version_minor * 100) +
           ($version_micro)) < 101000;`;
--enable_query_log
