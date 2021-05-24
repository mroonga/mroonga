#!/bin/bash

set -exu

package=$1

mysql_version=$(echo "${package}" | grep -o '[0-9]*\.[0-9]*')

echo "debconf debconf/frontend select Noninteractive" | \
  sudo debconf-set-selections

sudo apt update
sudo apt install -V -y apparmor lsb-release wget

code_name=$(lsb_release --codename --short)
architecture=$(dpkg --print-architecture)

wget \
  https://packages.groonga.org/debian/groonga-apt-source-latest-${code_name}.deb
sudo apt install -V -y ./groonga-apt-source-latest-${code_name}.deb
sudo apt update

case ${package} in
  mariadb-*)
    old_package=$(echo ${package} | sed -e 's/mariadb-/mariadb-server-/')
    mysql_package_prefix=mariadb
    client_dev_package=libmariadbclient-dev
    test_package=mariadb-test
    mysql_test_dir=/usr/share/mysql/mysql-test
    ;;
  mysql-community-*)
    old_package=
    wget https://repo.mysql.com/mysql-apt-config_0.8.17-1_all.deb
    sudo \
      env DEBIAN_FRONTEND=noninteractive \
          MYSQL_SERVER_VERSION=mysql-${mysql_version} \
        apt install -y ./mysql-apt-config_*_all.deb
    sudo apt update
    mysql_package_prefix=mysql
    client_dev_package=libmysqlclient-dev
    test_package=mysql-testsuite
    mysql_test_dir=/usr/lib/mysql-test
    ;;
esac

(echo "Key-Type: RSA"; \
 echo "Key-Length: 4096"; \
 echo "Name-Real: Test"; \
 echo "Name-Email: test@example.com"; \
 echo "%no-protection") | \
  gpg --full-generate-key --batch
GPG_KEY_ID=$(gpg --list-keys --with-colon test@example.com | grep fpr | cut -d: -f10)
gpg --export --armor test@example.com > keys
sudo gpg \
  --no-default-keyring \
  --keyring /usr/share/keyrings/${package}.gpg \
  --import keys

sudo apt install -V -y reprepro
repositories_dir=/vagrant/packages/${package}/apt/repositories
mkdir -p conf/
cat <<DISTRIBUTIONS > conf/distributions
Codename: ${code_name}
Components: main
Architectures: amd64 source
SignWith: ${GPG_KEY_ID}
DISTRIBUTIONS
reprepro includedeb ${code_name} \
  ${repositories_dir}/debian/pool/${code_name}/main/*/*/*_{${architecture},all}.deb

cat <<APT_SOURCES | sudo tee /etc/apt/sources.list.d/${package}.list
deb [signed-by=/usr/share/keyrings/${package}.gpg] file://${PWD} ${code_name} main
APT_SOURCES
sudo apt update
sudo apt install -V -y ${package}

sudo apt install -V -y \
  gdb \
  ${client_dev_package} \
  ${test_package} \
  patch

cd ${mysql_test_dir}
sudo rm -rf plugin/mroonga
sudo mkdir -p plugin
sudo cp -a /vagrant/mysql-test/mroonga/ plugin/

parallel=$(nproc)
case ${package} in
  mysql-8.*|mysql-community-8.*|percona-server-8.*)
    parallel=1
    # TODO: Remove the following "rm" as soon as possible
    # when these functionality is supported or test case is fixed for MySQL 8.0.
    rm -rf plugin/mroonga/wrapper
    pushd plugin/mroonga/storage
    rm -rf optimization/condition_push_down
    rm -rf optimization/order_limit
    rm -rf optimization/count_skip
    popd
    ;;
esac

test_suite_names=""
set +x
for test_suite_name in $(find plugin/mroonga -type d '!' -name '[tr]'); do
  if [ -n "${test_suite_names}" ]; then
    test_suite_names="${test_suite_names},"
  fi
  test_suite_names="${test_suite_names}${test_suite_name}"
done
set -x

sudo \
  ./mtr \
  --force \
  --no-check-testcases \
  --parallel=${parallel} \
  --retry=3 \
  --suite="${test_suite_names}"

case ${package} in
  mariadb-*)
    # Test with binary protocol
    sudo \
      ./mtr \
      --force \
      --no-check-testcases \
      --parallel=${parallel} \
      --ps-protocol \
      --retry=3 \
      --suite="${test_suite_names}"
    ;;
esac

# Upgrade
if [ -n "${old_package}" ]; then
  sudo apt purge -V -y \
    grub-pc
  sudo apt purge -V -y \
    ${package} \
    "${mysql_package_prefix}-*"
  sudo mv /etc/apt/sources.list.d/${package}.list /tmp/
  sudo apt update
  sudo apt install -V -y ${old_package}
  sudo mv /tmp/${package}.list /etc/apt/sources.list.d/
  sudo apt update
  sudo apt upgrade -V -y
fi
