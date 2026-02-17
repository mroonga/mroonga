#!/bin/bash

set -exu

package="$1"

os_version=$(cut -d: -f5 /etc/system-release-cpe | cut -d. -f1)

case "${os_version}" in
  8)
    DNF_INSTALL="dnf install -y --enablerepo=powertools"
    sudo dnf module -y disable mysql
    ;;
  *)
    DNF_INSTALL="dnf install -y"
    ;;
esac

sudo ${DNF_INSTALL} \
     "https://packages.apache.org/artifactory/arrow/almalinux/${os_version}/apache-arrow-release-latest.rpm" \
     "https://packages.groonga.org/almalinux/${os_version}/groonga-release-latest.noarch.rpm"

case "${package}" in
  mariadb-*)
    mariadb_version=$(echo "${package}" | cut -d'-' -f2)
    service_name=mariadb
    mysql_command=mariadb
    have_auto_generated_password="no"

    cat <<REPO | sudo tee /etc/yum.repos.d/MariaDB.repo
[mariadb]
name = MariaDB
baseurl = "https://rpm.mariadb.org/${mariadb_version}/rhel/\$releasever/\$basearch"
gpgkey = https://rpm.mariadb.org/RPM-GPG-KEY-MariaDB
gpgcheck = 1
REPO
    if [ "${os_version}" = "8" ] || [ "${os_version}" = "9" ]; then
      sudo dnf module -y disable mariadb
    fi
    ;;
  mysql-community-minimal-*)
    service_name=mysqld
    mysql_command=mysql

    sudo ${DNF_INSTALL} \
         "https://repo.mysql.com/mysql-community-minimal-release-el${os_version}.rpm"
    echo "module_hotfixes=true" | sudo tee -a /etc/yum.repos.d/mysql-community-minimal.repo
    sudo sed -i -e 's/^enabled=0/enabled=1/g' /etc/yum.repos.d/mysql-community-minimal.repo
    ;;
  mysql-community-*)
    mysql_version=$(echo "${package}" | cut -d'-' -f3)
    mysql_package_version=$(echo "${mysql_version}" | sed -e 's/\.//g')
    service_name=mysqld
    mysql_command=mysql
    have_auto_generated_password="yes"

    sudo ${DNF_INSTALL} \
         "https://repo.mysql.com/mysql${mysql_package_version}-community-release-el${os_version}.rpm"
    ;;
  percona-*)
    percona_server_version=$(echo "${package}" | cut -d'-' -f3)
    service_name=mysqld
    mysql_command=mysql
    have_auto_generated_password=yes

    sudo ${DNF_INSTALL} \
         https://repo.percona.com/yum/percona-release-latest.noarch.rpm
    percona_package_version=$(echo ${percona_server_version} | sed -e 's/\.//g')
    if [ "${percona_package_version}" = "80" ]; then
      sudo percona-release setup ps${percona_package_version}
    else
      sudo percona-release enable-only ps-${percona_package_version}-lts release
    fi
    sudo ${DNF_INSTALL} percona-icu-data-files
    ;;
esac

sudo ${DNF_INSTALL} "${package}"

case "${package}" in
  mysql-community-minimal-*)
    sudo mkdir -p /var/lib/mysql /var/run/mysqld
    sudo chown mysql:mysql /var/lib/mysql /var/run/mysqld
    sudo chmod 1777 /var/lib/mysql /var/run/mysqld

    # mysql --initialize outputs the following logs to stderror:
    #
    #   2025-01-20T02:57:06.620776Z 0 [System] [MY-013169] [Server] /usr/sbin/mysqld (mysqld 8.0.40) initializing of server in progress as process 593
    #   2025-01-20T02:57:06.631994Z 1 [System] [MY-013576] [InnoDB] InnoDB initialization has started.
    #   2025-01-20T02:57:06.852935Z 1 [System] [MY-013577] [InnoDB] InnoDB initialization has ended.
    #   2025-01-20T02:57:07.484688Z 6 [Note] [MY-010454] [Server] A temporary password is generated for root@localhost: xxxxxxxxxxxx
    #
    # We can connect stderror of a command to stdin of the other
    # command through the pipe by using "|&".  So, we can get the
    # temporary password by using "|&" and "awk 'END{print $NF}'".
    #
    # Execution example:
    #   $ mysqld --initialize |& awk 'END{print $NF}'
    #   xxxxxxxxxxxx
    auto_generated_password=$(mysqld --initialize |& awk 'END{print $NF}')
    mysql="${mysql_command} -u root -p${auto_generated_password}"
    "${service_name}" &
    while ! mysqladmin ping -hlocalhost --silent; do
      sleep 1
    done
    sudo ${mysql} \
         --connect-expired-password \
         -e "ALTER USER user() IDENTIFIED BY '$auto_generated_password'"
    sudo ${mysql} < /usr/share/mroonga/install.sql
    ;;
  *)
    sudo systemctl start "${service_name}"
    mysql="${mysql_command} -u root"
    if [ "${have_auto_generated_password}" = "yes" ]; then
      auto_generated_password=$(sudo awk '/root@localhost/{print $NF}' /var/log/mysqld.log | tail -n 1)
      mysql="${mysql_command} -u root -p${auto_generated_password}"
      sudo ${mysql} \
           --connect-expired-password \
           -e "ALTER USER user() IDENTIFIED BY '$auto_generated_password'"
    fi
    ;;
esac

sudo ${mysql} -e "SHOW ENGINES" | grep Mroonga
