#!/bin/bash

set -exu

package=$1

echo "::group::Prepare repository"

mysql_version=$(echo "${package}" | grep -o '[0-9]*\.[0-9]*')

os=$(cut -d: -f4 /etc/system-release-cpe)

major_version=$(cut -d: -f5 /etc/system-release-cpe | grep -o "^[0-9]")
case ${major_version} in
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
    sudo ${DNF} update -y
    sudo dnf module -y disable mariadb
    sudo dnf module -y disable mysql
    ;;
esac

sudo ${DNF} install -y \
  https://packages.groonga.org/${os}/${major_version}/groonga-release-latest.noarch.rpm

echo "::endgroup::"


echo "::group::Prepare MySQL/MariaDB"

ha_mroonga_so=ha_mroonga.so
have_auto_generated_password=no
case ${package} in
  mariadb-*)
    old_package=${package}
    mysql_package_prefix=MariaDB
    service_name=mariadb
    ha_mroonga_so=ha_mroonga_official.so
    test_package_name=MariaDB-test
    baseurl=https://yum.mariadb.org/${mysql_version}/rhel/${major_version}/x86_64
    sudo tee /etc/yum.repos.d/MariaDB.repo <<-REPO
[mariadb]
name = MariaDB
baseurl = ${baseurl}
gpgkey=https://yum.mariadb.org/RPM-GPG-KEY-MariaDB
gpgcheck=1
REPO
    ;;
  mysql-community-minimal-*)
    mysql_package_prefix=mysql-community-minimal
    mysql_package_version=$(echo ${mysql_version} | sed -e 's/\.//g')
    old_package=${package}

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
    old_package=${package}
    # TODO: Remove this after we release packages for
    # mysql-community-8.4 on pckages.groonga.org.
    if [ "${mysql_version}" = "8.4" ]; then
      old_package=
    fi
    sudo ${DNF} install -y \
         https://repo.mysql.com/mysql${mysql_package_version}-community-release-el${major_version}.rpm
    ;;
  percona-*)
    service_name=mysqld
    have_auto_generated_password=yes
    sudo ${DNF} install -y \
         https://repo.percona.com/yum/percona-release-latest.noarch.rpm
    percona_package_version=$(echo ${mysql_version} | sed -e 's/\.//g')
    old_package=${package}
    sudo percona-release setup ps${percona_package_version}
    mysql_package_prefix=percona-server
    test_package_name=percona-server-test
    sudo ${DNF} install -y percona-icu-data-files
    ;;
esac

echo "::endgroup::"


echo "::group::Install"

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

repositories_dir=/host/packages/${package}/yum/repositories
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

echo "::endgroup::"


echo "::group::Prepare test"

sudo ${DNF} install -y \
  ${test_package_name} \
  gdb \
  patch

case ${os}-${major_version} in
  almalinux-9)
    sudo ${DNF} install -y perl-lib
    ;;
esac

test_directory=/usr/share/mysql-test
if [ -d /usr/share/mariadb-test ]; then
  test_directory=/usr/share/mariadb-test
fi
cd ${test_directory}
if [ -d plugin ]; then
  sudo mv plugin plugin.backup
else
  sudo rm -rf plugin
fi
sudo mkdir -p plugin
sudo cp -a /host/mysql-test/mroonga/ plugin/
sed -i'' -e "s/ha_mroonga\\.so/${ha_mroonga_so}/g" \
  plugin/mroonga/include/mroonga/check_ha_mroonga_so.inc
sed -i'' -e "s/\$HA_MROONGA_SO/${ha_mroonga_so}/g" \
  plugin/mroonga/include/mroonga/have_mroonga.opt

parallel=$(nproc)
case ${package} in
  mysql-8.*|mysql-community-8.*|percona-server-8.*)
    parallel=1
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

echo "::endgroup::"


echo "::group::Test"

sudo \
  ./mtr \
  --force \
  --mysqld=--loose-plugin-load-add=${ha_mroonga_so} \
  --mysqld=--loose-plugin-mroonga=ON \
  --no-check-testcases \
  --parallel=${parallel} \
  --retry=3 \
  --suite="${test_suite_names}"

echo "::endgroup::"


echo "::group::Test with binary protocol"

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

echo "::endgroup::"


echo "::group::Postpare test"

sudo rm -rf plugin
if [ -d plugin.backup ]; then
  sudo mv plugin.backup plugin
fi

echo "::endgroup::"


echo "::group::Upgrade"
if [ -n "${old_package}" ]; then
  # TODO: Remove this after we release a new version. Old Mroonga package
  # requires "which" in rpm/post.sh.
  sudo ${DNF} install -y which
  sudo ${DNF} erase -y \
       ${package} \
       "${mysql_package_prefix}-*"
  sudo rm -rf /var/lib/mysql

  sudo ${DNF} install -y ${old_package}
  sudo ${DNF} install -y \
       ${repositories_dir}/${os}/${major_version}/*/Packages/*.rpm

  mroonga_is_registered
fi
echo "::endgroup::"
