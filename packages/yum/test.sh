#!/bin/bash

set -exu

package=$1

mysql_version=$(echo "${package}" | grep -o '[0-9]*\.[0-9]*')

os=$(cut -d: -f4 /etc/system-release-cpe)
case ${os} in
  amazon)
    os=amazon-linux
    major_version=$(cut -d: -f6 /etc/system-release-cpe)
    DNF=yum
    sudo amazon-linux-extras install -y epel
    sudo yum install -y ca-certificates
    ;;
  centos|almalinux|linux)
    major_version=$(cut -d: -f5 /etc/system-release-cpe | grep -o "^[0-9]")
    case ${major_version} in
      7)
        DNF=yum
        ;;
      9)
        DNF="dnf --enablerepo=crb"
        sudo ${DNF} install -y \
          https://apache.jfrog.io/artifactory/arrow/almalinux/${major_version}/apache-arrow-release-latest.rpm
        ;;
      *)
        if [ ${os} = "linux" ]; then
          DNF="dnf --enablerepo=ol${major_version}_codeready_builder"
          sudo ${DNF} install -y \
            https://apache.jfrog.io/artifactory/arrow/almalinux/${major_version}/apache-arrow-release-latest.rpm
          os=almalinux # Because we can use packages for AlmaLinux on Oracle Linux.
        else
          DNF="dnf --enablerepo=powertools"
        fi
        sudo dnf module -y disable mariadb
        sudo dnf module -y disable mysql
        ;;
    esac
    ;;
esac

sudo ${DNF} install -y \
  https://packages.groonga.org/${os}/${major_version}/groonga-release-latest.noarch.rpm

ha_mroonga_so=ha_mroonga.so
have_auto_generated_password=no
case ${package} in
  mariadb-*)
    if [ ${os} = "amazon-linux" ]; then
      old_package=${package}
      mysql_package_prefix=mariadb
      service_name=mariadb
      ha_mroonga_so=ha_mroonga_official.so
      test_package_name=mariadb-test
      sudo amazon-linux-extras install -y \
           $(echo ${package} | sed -e 's/-//g' -e 's/mroonga$//g')
    else
      old_package=${package}
      mysql_package_prefix=MariaDB
      service_name=mariadb
      ha_mroonga_so=ha_mroonga_official.so
      test_package_name=MariaDB-test
      case ${mysql_version} in
        10.5|10.6|10.11)
          baseurl=https://yum.mariadb.org/${mysql_version}/rhel${major_version}-amd64
          ;;
        *)
          baseurl=https://yum.mariadb.org/${mysql_version}/centos${major_version}-amd64
          ;;
      esac
      sudo tee /etc/yum.repos.d/MariaDB.repo <<-REPO
[mariadb]
name = MariaDB
baseurl = ${baseurl}
gpgkey=https://yum.mariadb.org/RPM-GPG-KEY-MariaDB
gpgcheck=1
REPO
    fi
    ;;
  mysql-community-minimal-*)
    mysql_package_prefix=mysql-community-minimal
    mysql_package_version=$(echo ${mysql_version} | sed -e 's/\.//g')
    old_package=mysql${mysql_package_version}-community-minimal-mroonga

    sudo ${DNF} install -y \
         https://repo.mysql.com/mysql-community-minimal-release-el${major_version}.rpm
    echo "module_hotfixes=true" | sudo tee -a /etc/yum.repos.d/mysql-community-minimal.repo
    sudo sed -i -e 's/^enabled=0/enabled=1/g' /etc/yum.repos.d/mysql-community-minimal.repo
    ;;
  mysql-community-*)
    mysql_package_prefix=mysql-community
    service_name=mysqld
    test_package_name=mysql-community-test
    have_auto_generated_password=yes
    mysql_package_version=$(echo ${mysql_version} | sed -e 's/\.//g')
    old_package=mysql${mysql_package_version}-community-mroonga
    case ${major_version} in
      7)
        sudo ${DNF} install -y \
             https://repo.mysql.com/mysql80-community-release-el7-5.noarch.rpm
        ;;
      8)
        sudo ${DNF} install -y \
             https://repo.mysql.com/mysql80-community-release-el8-3.noarch.rpm
        ;;
      *)
        sudo ${DNF} install -y \
             https://repo.mysql.com/mysql80-community-release-el${major_version}.rpm
        ;;
    esac
    ;;
  percona-*)
    service_name=mysqld
    have_auto_generated_password=yes
    sudo ${DNF} install -y \
         https://repo.percona.com/yum/percona-release-latest.noarch.rpm
    percona_package_version=$(echo ${mysql_version} | sed -e 's/\.//g')
    old_package=percona-server-${percona_package_version}-mroonga
    sudo percona-release setup ps${percona_package_version}
    mysql_package_prefix=percona-server
    test_package_name=percona-server-test
    sudo ${DNF} install -y percona-icu-data-files
    ;;
esac


# Install
function mroonga_is_registered() {
  sudo systemctl start ${service_name}
  mysql="mysql -u root"
  if [ "${have_auto_generated_password}" = "yes" ]; then
    auto_generated_password=$(sudo awk '/root@localhost/{print $NF}' /var/log/mysqld.log | tail -n 1)
    mysql="${mysql} -p${auto_generated_password}"
    sudo ${mysql} --connect-expired-password -e "ALTER USER user() IDENTIFIED BY '$auto_generated_password'"
  fi

  sudo ${mysql} -e "SHOW ENGINES" | grep Mroonga

  if [ "${have_auto_generated_password}" = "yes" ] ; then
    sudo ${mysql} -e "ALTER USER root@localhost PASSWORD EXPIRE"
  fi
  sudo systemctl stop ${service_name}
}

function mroonga_can_be_registered_for_mysql_community_minimal() {
  sudo mkdir -p /var/lib/mysql /var/run/mysqld
  sudo chown mysql:mysql /var/lib/mysql /var/run/mysqld
  sudo chmod 1777 /var/lib/mysql /var/run/mysqld

  auto_generated_password=$(mysqld --initialize |& awk 'END{print $NF}')
  mysql="mysql -u root -p${auto_generated_password}"
  mysqld &

  while ! mysqladmin ping -hlocalhost --silent; do sleep 1; done

  sudo ${mysql} --connect-expired-password -e "ALTER USER user() IDENTIFIED BY '$auto_generated_password'"

  sudo ${mysql} < /usr/share/mroonga/install.sql
  sudo ${mysql} -e "SHOW ENGINES" | grep Mroonga
  mysqladmin -u root -p${auto_generated_password} shutdown
}

repositories_dir=/vagrant/packages/${package}/yum/repositories
sudo ${DNF} install -y \
  ${repositories_dir}/${os}/${major_version}/*/Packages/*.rpm

case ${package} in
  mysql-community-minimal-*)
    mroonga_can_be_registered_for_mysql_community_minimal
    # mysql-community-minimal doesn't execute tests.
    # Because we can not install mysql-community-test package on the enviromnment that is installed mysql-community-minimal.
    # Because the mysql-community-minimal package and the mysql-community-test package are conflict.
    # Also, mysql-community-minimal doesn't execute upgrade test.
    # Because this package is only used in Docker. Docker image doesn't use package upgrade.
    exit
    ;;
  *)
    mroonga_is_registered
    ;;
esac

# Run test
sudo ${DNF} install -y \
  ${test_package_name} \
  gdb \
  patch

case ${os}-${major_version} in
  almalinux-9)
    sudo ${DNF} install -y perl-lib
    ;;
esac

cd /usr/share/mysql-test/
if [ -d plugin ]; then
  sudo mv plugin plugin.backup
else
  sudo rm -rf plugin
fi
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

sudo rm -rf plugin
if [ -d plugin.backup ]; then
  sudo mv plugin.backup plugin
fi

# Upgrade
sudo ${DNF} erase -y \
  ${package} \
  "${mysql_package_prefix}-*"
sudo rm -rf /var/lib/mysql

sudo ${DNF} install -y ${old_package}
sudo ${DNF} install -y \
  ${repositories_dir}/${os}/${major_version}/*/Packages/*.rpm

mroonga_is_registered
