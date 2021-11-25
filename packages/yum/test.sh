#!/bin/bash

set -exu

package=$1

mysql_version=$(echo "${package}" | grep -o '[0-9]*\.[0-9]*')

os=$(cut -d: -f4 /etc/system-release-cpe)
major_version=$(cut -d: -f5 /etc/system-release-cpe | grep -o "^[0-9]")
case ${major_version} in
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
  https://packages.groonga.org/${os}/${major_version}/groonga-release-latest.noarch.rpm

ha_mroonga_so=ha_mroonga.so
have_auto_generated_password=no
case ${package} in
  mariadb-*)
    old_package=${package}
    mysql_package_prefix=MariaDB
    service_name=mariadb
    ha_mroonga_so=ha_mroonga_official.so
    test_package_name=MariaDB-test
    sudo tee /etc/yum.repos.d/MariaDB.repo <<-REPO
[mariadb]
name = MariaDB
baseurl = http://yum.mariadb.org/${mysql_version}/centos${major_version}-amd64
gpgkey=https://yum.mariadb.org/RPM-GPG-KEY-MariaDB
gpgcheck=1
REPO
    ;;
  mysql-community-*)
    mysql_package_prefix=mysql-community
    service_name=mysqld
    test_package_name=mysql-community-test
    have_auto_generated_password=yes
    mysql_package_version=$(echo ${mysql_version} | sed -e 's/\.//g')
    old_package=mysql${mysql_package_version}-community-mroonga
    sudo ${DNF} install -y \
         https://repo.mysql.com/mysql${mysql_package_version}-community-release-el${major_version}.rpm
    ;;
  percona-*)
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
    old_package=percona-server-${percona_package_version}-mroonga
    sudo percona-release setup ps${percona_package_version}
    ;;
esac


# Install
repositories_dir=/vagrant/packages/${package}/yum/repositories
sudo ${DNF} install -y \
  ${repositories_dir}/${os}/${major_version}/*/Packages/*.rpm

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
sed -i'' -e "s/ha_mroonga\\.so/${ha_mroonga_so}/g" \
  plugin/mroonga/include/mroonga/check_ha_mroonga_so.inc
sed -i'' -e "s/\$HA_MROONGA_SO/${ha_mroonga_so}/g" \
  plugin/mroonga/include/mroonga/have_mroonga.opt

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
  --mysqld=--loose-plugin-load-add=${ha_mroonga_so} \
  --mysqld=--loose-plugin-mroonga=ON \
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
      --mysqld=--loose-plugin-load-add=${ha_mroonga_so} \
      --mysqld=--loose-plugin-mroonga=ON \
      --parallel=${parallel} \
      --ps-protocol \
      --retry=3 \
      --suite="${test_suite_names}"
    ;;
esac

# Upgrade
sudo ${DNF} erase -y \
  ${package} \
  "${mysql_package_prefix}-*"

# Currently, this check is always fails in mariadb 10.6.
# The cause of failure is the Mroonga packages for
# MariaDB10.6 don't exist in packages.groonga.org.
# Because the Mroonga packages for MariaDB10.6 are made for the first time.
# Therefore, this check disable temporarily in mariadb 10.6.
# We enable this check again after we release the next release.
case ${package} in
  mariadb-10.6-*)
    exit
    ;;
esac

sudo ${DNF} install -y ${old_package}
sudo ${DNF} install -y \
  ${repositories_dir}/${os}/${major_version}/*/Packages/*.rpm
