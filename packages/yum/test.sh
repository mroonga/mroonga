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
  centos|almalinux)
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
        DNF="dnf --enablerepo=powertools"
        sudo dnf module -y disable mariadb
        sudo dnf module -y disable mysql
        ;;
    esac
    ;;
  linux) # Oracle Linux
    os=almalinux
    major_version=$(cut -d: -f5 /etc/system-release-cpe | grep -o "^[0-9]")
    DNF="dnf --enablerepo=ol${major_version}_codeready_builder"
    sudo ${DNF} install -y \
      https://apache.jfrog.io/artifactory/arrow/almalinux/${major_version}/apache-arrow-release-latest.rpm
    case ${major_version} in
      8)
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
        10.5|10.6|10.7|10.8|10.9|10.10|10.11)
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
    service_name=mysqld
    test_package_name=mysql-community-test
    have_auto_generated_password=yes
    mysql_package_version=$(echo ${mysql_version} | sed -e 's/\.//g')
    old_package=mysql${mysql_package_version}-community-minimal-mroonga
    case ${major_version} in
#      7)
#        sudo ${DNF} install -y \
#             https://repo.mysql.com/mysql80-community-release-el7-5.noarch.rpm
#        ;;
      8)
        sudo ${DNF} install -y \
             https://repo.mysql.com/mysql-community-minimal-release-el8-1.noarch.rpm
        echo "module_hotfixes=true" | sudo tee -a /etc/yum.repos.d/mysql-community-minimal.repo
        sudo sed -i -e 's/enabled=0/enabled=1/g' /etc/yum.repos.d/mysql-community-minimal.repo
        ;;
#      *)
#        sudo ${DNF} install -y \
#             https://repo.mysql.com/mysql80-community-release-el${major_version}.rpm
#        ;;
    esac
    if [ "${major_version}" = "7" ] && [ "${mysql_version}" = "5.7" ]; then
      sudo yum-config-manager --disable mysql80-community
      sudo yum-config-manager --enable mysql57-community
    fi
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
    if [ "${major_version}" = "7" ] && [ "${mysql_version}" = "5.7" ]; then
      sudo yum-config-manager --disable mysql80-community
      sudo yum-config-manager --enable mysql57-community
    fi
    ;;
  percona-*)
    service_name=mysqld
    have_auto_generated_password=yes
    sudo ${DNF} install -y \
         https://repo.percona.com/yum/percona-release-latest.noarch.rpm
    percona_package_version=$(echo ${mysql_version} | sed -e 's/\.//g')
    old_package=percona-server-${percona_package_version}-mroonga
    sudo percona-release setup ps${percona_package_version}
    case ${mysql_version} in
      5.7)
        mysql_package_prefix=Percona-Server
        test_package_name=Percona-Server-test-57
        ;;
      *)
        mysql_package_prefix=percona-server
        test_package_name=percona-server-test
        sudo ${DNF} install -y percona-icu-data-files
        ;;
    esac
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

repositories_dir=/vagrant/packages/${package}/yum/repositories
sudo ${DNF} install -y \
  ${repositories_dir}/${os}/${major_version}/*/Packages/*.rpm

mroonga_is_registered

# Run test
sudo ${DNF} install -y \
  ${test_package_name} \
  gdb \
  patch

case ${os}-${major_version} in
  almalinux-9)
    sudo ${DNF} install -y perl-lib
    ;;
  oracle-linux-9)
    sudo ${DNF} install -y perl-base perl-lib
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

# Disable upgrade test for first time packages.
case ${os}-${major_version} in
  oracle-linux-*) # TODO: Remove this after 13.01 release.
    exit
    ;;
esac
case ${package} in
  mariadb-10.11-*) # TODO: Remove this after 13.01 release.
    exit
    ;;
esac

sudo ${DNF} install -y ${old_package}
sudo ${DNF} install -y \
  ${repositories_dir}/${os}/${major_version}/*/Packages/*.rpm

mroonga_is_registered
