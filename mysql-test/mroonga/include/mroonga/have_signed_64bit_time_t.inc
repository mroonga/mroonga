# Copyright(C) 2016  Kouhei Sutou <kou@clear-code.com>
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

--source ../../include/mroonga/check_64bit.inc
--source ../../include/mroonga/check_osx.inc
--source ../../include/mroonga/check_windows.inc

if (!$version_compile_64bit) {
  --skip Need a 64 binary for signed 64bit time_t
}

if ($version_compile_os_osx) {
  if (!$version_compile_os_osx_10_8_or_later) {
    --skip Need OS X 10.8 or later for signed 64bit time_t
  }
}

if ($version_compile_os_win) {
  --skip Windows doesn't have signed 64bit time_t
}
