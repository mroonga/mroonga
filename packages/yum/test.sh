#!/bin/bash

set -exu

package=$1

mysql_version=$(echo "${package}" | grep -o '[0-9]*\.[0-9]*')

centos_version=$(cut -d: -f5 /etc/system-release-cpe)
case ${centos_version} in
  7)
    DNF=yum
    ;;
  *)
    DNF="dnf --enablerepo=powertools"
    sudo dnf module -y disable mariadb
    sudo dnf module -y disable mysql
    ;;
esac

sudo ${DNF} install -y \
  https://packages.groonga.org/centos/groonga-release-latest.noarch.rpm

ha_mroonga_so=ha_mroonga.so
have_auto_generated_password=no
case ${package} in
  mariadb-*)
    mysql_package_prefix=MariaDB
    mroonga_package=$(echo "${package}" | sed -e '' -e 's/-server//g')
    service_name=mariadb
    ha_mroonga_so=ha_mroonga_official.so
    test_package_name=MariaDB-test
    sudo tee /etc/yum.repos.d/MariaDB.repo <<-REPO
[mariadb]
name = MariaDB
baseurl = http://yum.mariadb.org/${mysql_version}/centos${centos_version}-amd64
gpgkey=https://yum.mariadb.org/RPM-GPG-KEY-MariaDB
gpgcheck=1
REPO
    ;;
  mysql-*)
    mysql_package_prefix=mysql-community
    # mysql-server-5.7-mroonga ->
    # mysql57-community-mroonga
    mroonga_package=$(echo "${package}" | \
                        sed 's/-server-\([0-9]*\)\.\([0-9]*\)/\1\2-community/g')
    service_name=mysqld
    test_package_name=mysql-community-test
    have_auto_generated_password=yes
    mysql_package_version=$(echo ${mysql_version} | sed -e 's/\.//g')
    sudo ${DNF} install -y \
         https://repo.mysql.com/mysql${mysql_package_version}-community-release-el${centos_version}.rpm
    ;;
  percona-*)
    # percona-server-5.7-mroonga ->
    # percona-server-57-mroonga ->
    mroonga_package=$(echo "${package}" | sed 's/\([0-9]*\)\.\([0-9]*\)/\1\2/g')
    service_name=mysqld
    case ${mysql_version} in
      5.7)
        mysql_package_prefix=Percona-Server
        test_package_name=Percona-Server-test-57
        ;;
      *)
        mysql_package_prefix=percona-server
        test_package_name=percona-server-test
        ;;
    esac
    have_auto_generated_password=yes
    sudo ${DNF} install -y \
         https://repo.percona.com/yum/percona-release-latest.noarch.rpm
    percona_package_version=$(echo ${mysql_version} | sed -e 's/\.//g')
    sudo percona-release setup ps${percona_package_version}
    ;;
esac


# Install
repositories_dir=/vagrant/packages/${package}/yum/repositories
sudo ${DNF} install -y \
  ${repositories_dir}/centos/${centos_version}/*/Packages/*.rpm

sudo systemctl start ${service_name}
mysql="mysql -u root"
if [ "${have_auto_generated_password}" = "yes" ]; then
  auto_generated_password=$(sudo awk '/root@localhost/{print $NF}' /var/log/mysqld.log | tail -n 1)
  mysql="${mysql} -p${auto_generated_password}"
  sudo ${mysql} --connect-expired-password -e "ALTER USER user() IDENTIFIED BY '$auto_generated_password'"
fi
sudo ${mysql} -e 'SHOW ENGINES' | grep Mroonga
if [ "${have_auto_generated_password}" = "yes" ] ; then
  sudo ${mysql} -e "ALTER USER root@localhost PASSWORD EXPIRE"
fi
sudo systemctl stop ${service_name}


# Run test
sudo ${DNF} install -y \
  ${test_package_name} \
  gdb \
  patch

cd /usr/share/mysql-test/
sudo rm -rf plugin
sudo mkdir -p plugin
sudo cp -a /vagrant/mysql-test/mroonga/ plugin/
sudo rm -rf plugin/mroonga/{storage,wrapper}/suite.{opt,pm}
sudo sed -i'' -e "s/ha_mroonga\\.so/${ha_mroonga_so}/g" \
  plugin/mroonga/include/mroonga/check_ha_mroonga_so.inc

parallel=$(nproc)
case ${package} in
  mysql-server-8.*|percona-server-8.*)
    parallel=1
    # TODO: Remove the following "rm" as soon as possible
    # when these functionality is supported or test case is fixed for MySQL 8.0.
    rm -rf plugin/mroonga/wrapper
    pushd plugin/mroonga/storage
    rm -rf alter_table/add_column
    rm -rf alter_table/add_index/unique
    rm -rf create/table/table/default_tokenizer
    rm -rf create/table/index/lexicon
    rm -rf optimization/condition_push_down
    rm -rf optimization/order_limit
    rm -rf optimization/count_skip
    rm -rf column/set
    rm -rf foreign_key
    rm -f alter_table/disable_keys/t/truncate.test
    rm -f alter_table/t/spatial.test
    rm -f create/table/t/TODO_SPLIT_ME.test
    rm -f create/table/index/lexicon/t/comment.test
    rm -f delete/t/normal_column.test
    rm -f geometry/strict_sql_mode/t/bulk_insert_null.test
    rm -f like/t/unicode_ci.test
    rm -f select/empty_key/t/where_not_equal.test
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
  --mysqld=--loose-plugin-load-add=${ha_mroonga_so} \
  --mysqld=--loose-plugin-mroonga=ON \
  --no-check-testcases \
  --parallel=${parallel} \
  --retry=3 \
  --suite="${test_suite_names}"


# Upgrade
sudo ${DNF} erase -y \
  ${mroonga_package} \
  "${mysql_package_prefix}-*"
sudo ${DNF} install -y ${mroonga_package}
sudo ${DNF} install -y \
  ${repositories_dir}/centos/${centos_version}/*/Packages/*.rpm
