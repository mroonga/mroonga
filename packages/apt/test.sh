#!/bin/bash

set -exu

package=$1

sudo apt update
sudo apt install -V -y lsb-release wget

code_name=$(lsb_release --codename --short)
architecture=$(dpkg --print-architecture)

wget \
  https://packages.groonga.org/debian/groonga-apt-source-latest-${code_name}.deb
sudo apt install -V -y ./groonga-apt-source-latest-${code_name}.deb
sudo apt update

sudo apt install -V -y ${package}

repositories_dir=/vagrant/packages/${package}/apt/repositories
sudo apt install -V -y \
  ${repositories_dir}/debian/pool/${code_name}/main/*/*/*_{${architecture},all}.deb

sudo apt install -V -y \
  gdb \
  libmariadbclient-dev \
  mariadb-test \
  patch

cd /usr/share/mysql/mysql-test/
sudo rm -rf plugin/mroonga
sudo cp -a /vagrant/mysql-test/mroonga/ plugin/

test_suite_names=""
for test_suite_name in $(find plugin/mroonga -type d '!' -name '[tr]'); do
  if [ -n "${test_suite_names}" ]; then
    test_suite_names="${test_suite_names},"
  fi
  test_suite_names="${test_suite_names}${test_suite_name}"
done
sudo \
  ./mtr \
  --force \
  --no-check-testcases \
  --parallel=$(nproc) \
  --retry=3 \
  --suite="${test_suite_names}"

case ${package} in
  mariadb-*)
    # Test with binary protocol
    sudo \
      ./mtr \
      --force \
      --no-check-testcases \
      --parallel=$(nproc) \
      --ps-protocol \
      --retry=3 \
      --suite="${test_suite_names}"
    ;;
esac
