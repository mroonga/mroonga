#!/bin/bash
#
# Copyright(C) 2012-2019 Kouhei Sutou <kou@clear-code.com>
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

# set -x
set -e

if [ "${MROONGA_BUNDLED}" = "yes" ]; then
  cmake_args=(-DCMAKE_BUILD_TYPE=Debug -DWITH_UNIT_TESTS=FALSE)
  cmake_args=("${cmake_args[@]}" -DGRN_WITH_BUNDLED_MESSAGE_PACK=TRUE)
  cmake_args=("${cmake_args[@]}" -DWITHOUT_ARCHIVE=TRUE)
  cmake_args=("${cmake_args[@]}" -DWITHOUT_BLACKHOLE=TRUE)
  cmake_args=("${cmake_args[@]}" -DWITHOUT_CASSANDRA=TRUE)
  cmake_args=("${cmake_args[@]}" -DWITHOUT_CONNECT=TRUE)
  cmake_args=("${cmake_args[@]}" -DWITHOUT_CSV=TRUE)
  cmake_args=("${cmake_args[@]}" -DWITHOUT_EXAMPLE=TRUE)
  cmake_args=("${cmake_args[@]}" -DWITHOUT_FEDERATED=TRUE)
  cmake_args=("${cmake_args[@]}" -DWITHOUT_FEDERATEDX=TRUE)
  cmake_args=("${cmake_args[@]}" -DWITHOUT_HEAP=TRUE)
  cmake_args=("${cmake_args[@]}" -DWITHOUT_MYISAMMRG=TRUE)
  cmake_args=("${cmake_args[@]}" -DWITHOUT_OQGRAPH=TRUE)
  cmake_args=("${cmake_args[@]}" -DWITHOUT_SEQUENCE=TRUE)
  cmake_args=("${cmake_args[@]}" -DWITHOUT_SPHINX=TRUE)
  cmake_args=("${cmake_args[@]}" -DWITHOUT_SPIDER=TRUE)
  cmake_args=("${cmake_args[@]}" -DWITHOUT_TEST_SQL_DISCOVERY=TRUE)
  cmake_args=("${cmake_args[@]}" -DWITHOUT_TOKUDB=TRUE)
  if [ "${MROONGA_TEST_EMBEDDED}" = "yes" ]; then
    cmake_args=("${cmake_args[@]}" -DWITH_EMBEDDED_SERVER=TRUE)
    cmake_args=("${cmake_args[@]}" -DMRN_BUILD_FOR_EMBEDDED_SERVER=TRUE)
  fi
  cmake . "${cmake_args[@]}"
else
  ./autogen.sh

  if [ -d /opt/mysql/ ]; then
    PATH=$(echo /opt/mysql/server-*/bin/):$PATH
  fi
  configure_args=("--with-mysql-source=$PWD/vendor/mysql")
  case "${MYSQL_VERSION}" in
    mysql-system)
      (cd vendor/mysql && \
       fakeroot debian/rules override_dh_auto_configure && \
       cd builddir/libservices && \
       make > /dev/null)
      configure_args=("${configure_args[@]}"
                      "--with-mysql-build=$PWD/vendor/mysql/builddir")
      ;;
    mysql-5.7)
      sudo mkdir -p /usr/global/share
      sudo chown -R ${USER}: /usr/global/share/
      sed -i 's/-DWITH_BOOST/-DDOWNLOAD_BOOST=1 -DWITH_BOOST/' vendor/mysql/debian/rules
      # Remove https://bugs.mysql.com/bug.php?id=97278 is fixed (hardcoded build host path issue exists on MySQL 5.7.28)
      sed -i '/^-DWITH_SSL/d' vendor/mysql/debian/rules
      (cd vendor/mysql && fakeroot debian/rules override_dh_auto_configure)
      configure_args=("${configure_args[@]}"
                      "--with-mysql-build=$PWD/vendor/mysql/release")
      ;;
    mysql-8.0)
      sudo mkdir -p /usr/global/share
      sudo chown -R ${USER}: /usr/global/share/
      # Use DOWNLOAD_BOOST=1 explicitly to avoid https://bugs.mysql.com/bug.php?id=96266.
      sed -i 's/-DWITH_BOOST/-DDOWNLOAD_BOOST=1 -DWITH_BOOST/' vendor/mysql/debian/rules
      (cd vendor/mysql && fakeroot debian/rules override_dh_auto_configure)
      configure_args=("${configure_args[@]}"
                      "--with-mysql-build=$PWD/vendor/mysql/release")
      ;;
    percona-server-5.6)
      (cd vendor/mysql && \
       fakeroot debian/rules configure SKIP_DEBUG_BINARY=yes && \
       cd builddir/libservices && \
       make > /dev/null && \
       cd ../extra && \
       make > /dev/null)
      configure_args=("${configure_args[@]}"
                      "--enable-fast-mutexes"
                      "--with-mysql-build=$PWD/vendor/mysql/builddir"
                      "--with-mysql-config=$PWD/vendor/mysql/builddir/scripts/mysql_config")
      ;;
    percona-server-5.7)
      (cd vendor/mysql && \
       fakeroot debian/rules override_dh_auto_configure SKIP_DEBUG_BINARY=yes && \
       cd builddir/libservices && \
       make > /dev/null && \
       cd ../extra && \
       make > /dev/null)
      configure_args=("${configure_args[@]}"
                      "--with-mysql-build=$PWD/vendor/mysql/builddir"
                      "--with-mysql-config=$PWD/vendor/mysql/builddir/scripts/mysql_config")
      ;;
    *)
      :
      ;;
  esac
  ./configure "${configure_args[@]}"
fi
