#!/bin/bash

package=$1
action=$2

if [ "${action}" != "configure" ]; then
  exit
fi

previous_version=$3

if [ -z "${previous_version}" ]; then
  configure_action="install"
else
  configure_action="update"
fi

. /usr/share/debconf/confmodule

install_apparmor() {
  mysql_apparmor_profile_name=usr.sbin.mysqld
  mysql_apparmor_profile=/etc/apparmor.d/${mysql_apparmor_profile_name}
  mysql_local_apparmor_profile=/etc/apparmor.d/local/${mysql_apparmor_profile_name}
  apparmor_profile_name=${package}
  include_profile="#include <abstractions/${apparmor_profile_name}>"
  local_apparmor_profile=/etc/apparmor.d/local/${apparmor_profile_name}
  if test -f "${mysql_local_apparmor_profile}"; then
    if ! grep -q "${include_profile}" "${mysql_local_apparmor_profile}"; then
      echo >> "${mysql_local_apparmor_profile}"
      echo "${include_profile}" >> "${mysql_local_apparmor_profile}"
    fi
  fi

  if ! test -e "${local_apparmor_profile}"; then
    mkdir -p $(dirname "${local_apparmor_profile}")
    cat <<PROFILE > "${local_apparmor_profile}"
# Site-specific additions and overrides for ${apparmor_profile_name}.
# For more details, please see /etc/apparmor.d/local/README.
PROFILE
  fi

  if aa-status --enabled 2>/dev/null; then
    apparmor_parser -r -T -W "${mysql_apparmor_profile}" || :
  fi

  :
}

install_mroonga() {
  try_auto_prepare=no
  need_stop=no
  have_auto_generated_password=no
  if systemctl is-active mysql > /dev/null; then
    try_auto_prepare=yes
  else
    if systemctl start mysql > /dev/null; then
      need_stop=yes
      try_auto_prepare=yes
    fi
  fi

  need_manual_register=no
  need_manual_restart=no
  need_manual_update=no
  if [ "${configure_action}" = "install" ]; then
    need_manual_register=yes
    need_manual_update=yes
  else
    need_manual_restart=yes
    need_manual_update=yes
  fi

  install_plugin_sql=/usr/share/mroonga/install_plugin.sql
  uninstall_sql=/usr/share/mroonga/uninstall.sql
  update_sql=/usr/share/mroonga/update.sql

  if [ "${try_auto_prepare}" = "yes" ]; then
    if [ -f /etc/mysql/debian.cnf ]; then
      password_options="--defaults-file=/etc/mysql/debian.cnf"
    else
      db_input high ${package}/root-password || :
      db_go
      db_get ${package}/root-password
      password="${RET}"
      db_set ${package}/root-password ""
      if [ -z "${password}" ]; then
        password_options="-u root --skip-password"
      else
        password_options="-u root -p$(printf %q "${password}")"
      fi
    fi
  fi

  if [ "${try_auto_prepare}" = "yes" ]; then
    mysql="mysql ${password_options}"

    if [ "${configure_action}" = "install" ]; then
      if ${mysql} < ${install_plugin_sql}; then
        need_manual_register=no
      fi
    else
      if systemctl restart mysql; then
        need_manual_restart=no
      fi
    fi
    if ${mysql} < ${update_sql}; then
      need_manual_update=no
    fi
  else
    mysql="mysql -u root"
  fi

  if [ "${need_stop}" = "yes" ]; then
    systemctl stop mysql
  fi

  if [ "${need_manual_register}" = "yes" ]; then
    echo "Run the following command line to register Mroonga:"
    echo "  ${mysql} < ${install_plugin_sql}"
  fi

  if [ "${need_manual_restart}" = "yes" ]; then
    echo "Run the following command line to reload Mroonga:"
    echo "  systemctl restart mysql"
  fi

  if [ "${need_manual_update}" = "yes" ]; then
    echo "Run the following command lines to update Mroonga:"
    echo "  ${mysql} < ${update_sql}"
  fi
}

install_apparmor
install_mroonga
