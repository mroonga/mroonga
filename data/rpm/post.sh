#!/bin/bash

# set -eux

mode=$1
variant=$2
data_dir=$3
groonga_required_version=$4

if [ "${mode}" = "1" ]; then
  action=install
elif [ "${mode}" = "2" ]; then
  action=upgrade
fi

may_have_auto_generated_password=no
case "${variant}" in
  mysql)
    service_name=mysqld
    variant_label=MySQL
    may_have_auto_generated_password=yes
    ;;
  mariadb)
    service_name=mariadb
    variant_label=MariaDB
    ;;
  percona)
    service_name=mysqld
    variant_label="Percona Server"
    may_have_auto_generated_password=yes
    ;;
esac

try_auto_prepare=no
need_stop=no
have_auto_generated_password=no
if systemctl is-active ${service_name} > /dev/null; then
  try_auto_prepare=yes
else
  if systemctl start ${service_name} > /dev/null; then
    need_stop=yes
    if [ "${may_have_auto_generated_password}" = "yes" ]; then
      auto_generated_password=$(awk '/root@localhost/{print $NF}' /var/log/mysqld.log | tail -n 1)
      if [ -n "${auto_generated_password}" ]; then
        have_auto_generated_password=yes
      fi
    fi
    try_auto_prepare=yes
  fi
fi

need_manual_register=no
need_manual_restart=no
need_manual_update=no
case "${action}" in
  install)
    need_manual_register=yes
    need_manual_update=yes
    ;;
  update)
    need_manual_restart=yes
    need_manual_update=yes
    ;;
esac

install_sql=${data_dir}/mroonga/install.sql
uninstall_sql=${data_dir}/mroonga/uninstall.sql
update_sql=${data_dir}/mroonga/update.sql

need_password_expire=no
if [ "${try_auto_prepare}" = "yes" ]; then
  mysql_command=$(which mysql)
  password_option=""
  if [ "${have_auto_generated_password}" = "yes" ]; then
    if "${mysql_command}" \
         -u root \
         --connect-expired-password \
         -p"${auto_generated_password}" \
         -e "quit" > /dev/null 2>&1; then
      if "${mysql_command}" \
           -u root \
           --connect-expired-password \
           -p"${auto_generated_password}" \
           -e "ALTER USER root@localhost IDENTIFIED BY '$auto_generated_password'"; then
        need_password_expire=yes
        password_option="-p${auto_generated_password}"
      fi
    fi
  fi
  if [ -z "${password_option}" ]; then
    if ! "${mysql_command}" -u root -e "quit" > /dev/null 2>&1; then
      password_option="-p"
      echo "${variant_label}'s root password will be asked several times to"
      echo "${action} Mroonga. You don't need to input the root password"
      echo "in this package ${action} process. But you need to ${action}"
      echo "Mroonga manually later. The way to ${action} Mroonga manually"
      echo "will be showed later."
    fi
  fi

  mysql="${mysql_command} -u root ${password_option}"

  if [ "${action}" = "install" ]; then
    if ${mysql} < ${install_sql}; then
      need_manual_register=no
    fi
  else
    if systemctl restart ${service_name}; then
      need_manual_restart=no
    fi
    if ${mysql} < ${update_sql}; then
      need_manual_update=no
    fi
  fi
else
  mysql="mysql -u root"
fi

if [ "${need_password_expire}" = "yes" ]; then
  ${mysql} -e "ALTER USER root@localhost PASSWORD EXPIRE"
fi

if [ "${need_stop}" = "yes" ]; then
  if systemctl stop ${service_name}; then
    need_manual_restart=no
  fi
fi

if [ "${need_manual_register}" = "yes" ]; then
  echo "Run the following command line to register Mroonga:"
  echo "  ${mysql} < ${install_sql}"
fi

if [ "${need_manual_restart}" = "yes" ]; then
  echo "Run the following command line to reload Mroonga:"
  echo "  systemctl restart ${service_name}"
fi

if [ "${need_manual_update}" = "yes" ]; then
  echo "Run the following command lines to update Mroonga:"
  echo "  ${mysql} < ${update}"
fi
