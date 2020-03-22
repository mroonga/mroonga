#!/bin/sh
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

set -x
set -e

# export GROONGA_MASTER=yes
# export GROONGA_NORMALIZER_MYSQL_MASTER=yes

mariadb_download_base=https://downloads.mariadb.org/f
#mariadb_apt_base=http://mirror.jmu.edu/pub/mariadb
mariadb_apt_base=http://ftp.osuosl.org/pub/mariadb

version=$(echo "$MYSQL_VERSION" | sed -r -e 's/^(mysql|mariadb|percona-server)-//')
series=$(echo "$version" | sed -r -e 's/^([0-9]+\.[0-9]+).*$/\1/g')

setup_mariadb_apt()
{
  distribution=$(lsb_release --short --id | tr 'A-Z' 'a-z')
  code_name=$(lsb_release --short --codename)
  component=main
  apt_url_base="${mariadb_apt_base}/repo/${series}"
  cat <<EOF | sudo tee /etc/apt/sources.list.d/mariadb.list
deb ${apt_url_base}/${distribution}/ ${code_name} ${component}
deb-src ${apt_url_base}/${distribution}/ ${code_name} ${component}
EOF
  sudo apt-key adv --recv-keys --keyserver hkp://keyserver.ubuntu.com:80 0xF1656F24C74CD1D8
  sudo apt -qq update
}

setup_percona_apt()
{
  code_name=$(lsb_release --short --codename)
  release_deb_version=1.0-15
  release_deb=percona-release_${release_deb_version}.${code_name}_all.deb
  wget https://www.percona.com/downloads/percona-release/ubuntu/${release_deb_version}/${release_deb}
  sudo dpkg -i ${release_deb}
  sudo apt -qq update
}

if [ "${MROONGA_BUNDLED}" = "yes" ]; then
  mkdir -p .mroonga
  mv * .mroonga/
  mv .mroonga/tools ./
  setup_mariadb_apt
  sudo apt -qq -y build-dep mariadb-server
  # Support MariaDB for now.
  download_base=${mariadb_download_base}/${MYSQL_VERSION}
  tar_gz=${MYSQL_VERSION}.tar.gz
  curl --location -O ${download_base}/source/${tar_gz}
  tar xzf $tar_gz
  mv ${MYSQL_VERSION}/* ./
  rm -rf storage/mroonga
  mv .mroonga storage/mroonga
  rm -rf ${MYSQL_VERSION}
  git clone --recursive --depth 1 \
      https://github.com/groonga/groonga.git \
      storage/mroonga/vendor/groonga
  (cd storage/mroonga/vendor/groonga/vendor && \
     ruby download_message_pack.rb)
  git clone --recursive --depth 1 \
      https://github.com/groonga/groonga-normalizer-mysql.git \
      storage/mroonga/vendor/groonga/vendor/plugins/groonga-normalizer-mysql
else
  curl --silent --location \
       https://raw.githubusercontent.com/groonga/groonga/master/data/travis/setup.sh | sh
  curl --silent --location \
       https://raw.githubusercontent.com/groonga/groonga-normalizer-mysql/master/data/travis/setup.sh | sh
  # curl --silent --location \
  #      https://raw.githubusercontent.com/clear-code/cutter/master/data/travis/setup.sh | sh

  if [ ! -f /usr/lib/groonga/plugins/tokenizers/mecab.so ]; then
    sudo apt -qq -y install groonga-tokenizer-mecab
  fi

  sudo apt -qq -y install \
    autoconf-archive \
    fakeroot

  mkdir -p vendor
  cd vendor

  case "$MYSQL_VERSION" in
    mysql-*)
      sudo apt -qq update
      sudo apt -qq -y build-dep mysql-server
      if [ "$version" = "system" ]; then
        sudo rm -rf /var/lib/mysql
        sudo apt -y install \
             mysql-server \
             mysql-client \
             mysql-testsuite \
             libmysqld-dev
        apt source mysql-server
        ln -s $(find . -maxdepth 1 -type d | sort | tail -1) mysql
      else
        repository_deb=mysql-apt-config_0.8.12-1_all.deb
        curl -O http://repo.mysql.com/${repository_deb}
        sudo env MYSQL_SERVER_VERSION=mysql-${series} \
             dpkg -i ${repository_deb}
        sudo apt -qq update
        sudo apt -qq -y purge \
             mysql-common \
             mysql-client-core-5.7 \
             mysql-server-core-5.7
        sudo rm -rf /var/lib/mysql
        sudo apt -qq -y build-dep mysql-server
        sudo apt -qq -y install \
             mysql-server \
             libmysqlclient-dev \
             mysql-testsuite
        case "$MYSQL_VERSION" in
          mysql-5.*)
            sudo apt -qq -y install libmysqld-dev
            ;;
        esac
        apt -qq source mysql-server
        ln -s $(find . -maxdepth 1 -type d | sort | tail -1) mysql
      fi
      ;;
    percona-server-*)
      setup_percona_apt
      sudo apt -qq -y purge \
           mysql-common \
           mysql-client-core-5.7 \
           mysql-server-core-5.7
      sudo rm -rf /var/lib/mysql
      sudo apt -qq -y build-dep percona-server-server-${series}
      sudo apt -qq -y install \
           percona-server-server-${series} \
           percona-server-client-${series} \
           percona-server-test-${series}
      apt -qq source percona-server-server-${series}
      ln -s $(find . -maxdepth 1 -type d | sort | tail -1) mysql
      ;;
  esac

  cd ..
fi
