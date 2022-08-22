#!/bin/bash

set -exu

package=$1

mysql_version=$(echo "${package}" | grep -o '[0-9]*\.[0-9]*')

echo "debconf debconf/frontend select Noninteractive" | \
  sudo debconf-set-selections

sudo sed -i'' -e 's/in\.archive/archive/g' /etc/apt/sources.list

sudo apt update
sudo apt purge -V -y grub-pc
sudo apt install -V -y apparmor lsb-release wget

distribution=$(lsb_release --id --short | tr 'A-Z' 'a-z')
case ${distribution} in
  debian)
    repository=main
    ;;
  ubuntu)
    repository=universe
  ;;
esac
code_name=$(lsb_release --codename --short)
architecture=$(dpkg --print-architecture)

wget \
  https://packages.groonga.org/${distribution}/groonga-apt-source-latest-${code_name}.deb
sudo apt install -V -y ./groonga-apt-source-latest-${code_name}.deb
sudo apt update

case ${package} in
  mariadb-*)
    old_package=$(echo ${package} | sed -e 's/mariadb-/mariadb-server-/')
    mysql_package_prefix=mariadb
    client_dev_package=libmariadb-dev
    test_package=mariadb-test
    mysql_test_dir=/usr/share/mysql/mysql-test
    ;;
  mysql-community-*)
    old_package=${package}
    wget https://repo.mysql.com/mysql-apt-config_0.8.22-1_all.deb
    sudo \
      env DEBIAN_FRONTEND=noninteractive \
          MYSQL_SERVER_VERSION=mysql-${mysql_version} \
        apt install -y ./mysql-apt-config_*_all.deb
    pwd
    sudo apt update
    mysql_package_prefix=mysql
    client_dev_package=libmysqlclient-dev
    test_package=mysql-testsuite
    mysql_test_dir=/usr/lib/mysql-test
    ;;
  mysql-*)
    old_package=$(echo ${package} | sed -e 's/mysql-/mysql-server-/')
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
pushd /tmp/
mkdir -p conf/
cat <<DISTRIBUTIONS > conf/distributions
Codename: ${code_name}
Components: main
Architectures: amd64 source
SignWith: ${GPG_KEY_ID}
DISTRIBUTIONS
reprepro includedeb ${code_name} \
  ${repositories_dir}/${distribution}/pool/${code_name}/${repository}/*/*/*_{${architecture},all}.deb
cat <<APT_SOURCES | sudo tee /etc/apt/sources.list.d/${package}.list
deb [signed-by=/usr/share/keyrings/${package}.gpg] file://${PWD} ${code_name} main
APT_SOURCES
popd

sudo apt update
sudo apt install -V -y ${package}

sudo mysql -e "SHOW ENGINES" | grep Mroonga

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

case ${package} in
  mysql-8.*)
    ( \
      echo "/usr/lib/mysql-test/var/ r,"; \
      echo "/usr/lib/mysql-test/var/** rwk,"; \
    ) | sudo tee --append /etc/apparmor.d/local/usr.sbin.mysqld
    sudo systemctl restart apparmor
    ;;
esac

mtr_args=()
mtr_args+=(--force)
mtr_args+=(--no-check-testcases)
mtr_args+=(--parallel=${parallel})
mtr_args+=(--retry=3)
mtr_args+=(--suite="${test_suite_names}")
if [ -d "${mysql_test_dir}/bin" ]; then
  mtr_args+=(--client-bindir="${mysql_test_dir}/bin")
fi
sudo ./mtr "${mtr_args[@]}"
case ${package} in
  mariadb-*)
    # Test with binary protocol
    sudo ./mtr "${mtr_args[@]}" --ps-protocol
    ;;
esac

# Upgrade
if [ -n "${old_package}" ]; then
  sudo apt purge -V -y \
    ${package} \
    "${mysql_package_prefix}-*"
  sudo rm -rf /var/lib/mysql
  sudo mv /etc/apt/sources.list.d/${package}.list /tmp/
  case ${package} in
    mysql-community-8.*)
      pwd
      sudo \
        env DEBIAN_FRONTEND=noninteractive \
            MYSQL_SERVER_VERSION=mysql-${mysql_version} \
          apt install -y ../mysql-apt-config_*_all.deb
      ;;
  esac
  sudo apt update
  sudo apt install -V -y ${old_package}
  sudo mv /tmp/${package}.list /etc/apt/sources.list.d/
  sudo apt update
  sudo apt upgrade -V -y
  sudo mysql -e "SHOW ENGINES" | grep Mroonga
fi
