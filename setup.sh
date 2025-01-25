#!/bin/bash
#
# Copyright(C) 2023  Sutou Kouhei <kou@clear-code.com>
# Copyright(C) 2024  Horimoto Yasuhiro <horimoto@clear-code.com>
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

set -eux

if type sudo > /dev/null 2>&1; then
  SUDO=sudo
else
  SUDO=
fi

if [ -f /etc/debian_version ]; then
  ${SUDO} apt update
  ${SUDO} apt install -y -V lsb-release wget
  wget https://apache.jfrog.io/artifactory/arrow/$(lsb_release --id --short | tr 'A-Z' 'a-z')/apache-arrow-apt-source-latest-$(lsb_release --codename --short).deb
  ${SUDO} apt install -y -V ./apache-arrow-apt-source-latest-$(lsb_release --codename --short).deb
  rm apache-arrow-apt-source-latest-$(lsb_release --codename --short).deb
  ${SUDO} apt update
else
  echo "This OS setup is not supported."
  exit 1
fi

if type lsb_release > /dev/null 2>&1; then
  distribution=$(lsb_release --id --short | tr 'A-Z' 'a-z')
  code_name=$(lsb_release --codename --short)
else
  distribution=unknown
  code_name=unknown
fi

package_names=()
package_names+=(
  ccache
  cmake
  curl
  doxygen
  g++
  gcc
  gettext
  graphviz
  jq
  libarrow-dev
  libedit-dev
  liblz4-dev
  libmecab-dev
  libmsgpack-dev
  libsimdjson-dev
  libstemmer-dev
  libxxhash-dev
  libzstd-dev
  ninja-build
  pkg-config
  python3-pip
  rapidjson-dev
  zlib1g-dev
)

${SUDO} apt install -y -V "${package_names[@]}"

mariadb_latest_version=$(curl https://downloads.mariadb.org/rest-api/mariadb/11.4/latest/ | \
                           jq -r '.releases[].release_id')
wget "https://archive.mariadb.org/mariadb-${mariadb_latest_version}/source/mariadb-${mariadb_latest_version}.tar.gz"
tar -zxvf "mariadb-${mariadb_latest_version}.tar.gz" -C /tmp
mkdir -p /tmp/mariadb.build
cmake \
  -S/tmp/mariadb-${mariadb_latest_version} \
  -B/tmp/mariadb.build \
  -GNinja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_INSTALL_PREFIX=/tmp/local \
  -DWITH_DEBUG=ON
cmake --build /tmp/mariadb.build
cmake --install /tmp/mariadb.build

groonga_latest_version=$(curl https://api.github.com/repos/groonga/groonga/releases/latest | \
                           jq -r '.["tag_name"]' | sed 's/^v//')
wget https://github.com/groonga/groonga/releases/download/v${groonga_latest_version}/groonga-${groonga_latest_version}.tar.gz
tar -zxvf groonga-${groonga_latest_version}.tar.gz -C /tmp
mkdir -p /tmp/groonga.build
cmake \
  -S/tmp/groonga-${groonga_latest_version} \
  -B/tmp/groonga.build \
  --preset=debug-default \
  -DCMAKE_INSTALL_PREFIX=/tmp/local
cmake --build /tmp/groonga.build
cmake --install /tmp/groonga.build
