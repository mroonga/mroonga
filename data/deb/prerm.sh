#!/bin/bash

package=$1
action=$2

case "${action}" in
  remove)
    ;;
  *)
    exit
    ;;
esac

. /usr/share/debconf/confmodule

uninstall_apparmor() {
  mysql_apparmor_profile_name=usr.sbin.mysqld
  mysql_apparmor_profile=/etc/apparmor.d/${mysql_apparmor_profile_name}
  mysql_local_apparmor_profile=/etc/apparmor.d/local/${mysql_apparmor_profile_name}
  mysql_abstraction_apparmor_profile=/etc/apparmor.d/abstractions/mysql
  apparmor_profile_name=${package}
  if test -f "${mysql_local_apparmor_profile}"; then
    include_profile="#include <abstractions/${apparmor_profile_name}>"
    if grep -q "${include_profile}" "${mysql_local_apparmor_profile}"; then
      sed -i'' -e "s,${include_profile},," \
          "${mysql_local_apparmor_profile}"
    fi
  else
    start_marker_re="^# ${apparmor_profile_name}: START$"
    end_marker_re="^# ${apparmor_profile_name}: END$"
    if test -f "${mysql_abstraction_apparmor_profile}" && \
        grep -q "${start_marker_re}" \
             "${mysql_abstraction_apparmor_profile}"; then
      sed -i'' -e "/${start_marker_re}/,/${end_marker_re}/d" \
          "${mysql_abstraction_apparmor_profile}"
    fi
  fi

  rm -f "/etc/apparmor.d/local/${apparmor_profile_name}" || :
  rmdir /etc/apparmor.d/local 2>/dev/null || :

  if aa-status --enabled 2>/dev/null; then
    apparmor_parser -r -T -W "${mysql_apparmor_profile}" || :
  fi
}

uninstall_mroonga() {
  try_auto_uninstall=no
  need_stop=no
  if systemctl is-active mysql > /dev/null; then
    try_auto_uninstall=yes
  else
    if systemctl start ${service_name} > /dev/null; then
      need_stop=yes
      try_auto_uninstall=yes
    fi
  fi

  need_manual_unregister=yes

  uninstall_sql=/usr/share/mroonga/uninstall.sql

  if [ "${try_auto_uninstall}" = "yes" ]; then
    if [ -f /etc/mysql/debian.cnf ]; then
      password_options="--defaults-file=/etc/mysql/debian.cnf"
    else
      db_input high ${package}/root-password || :
      db_go
      db_get ${package}/root-password
      password="${RET}"
      db_set ${package}/root-password ""
      if [ -z "${password}" ]; then
        try_auto_uninstall=no
      else
        password_options="-u root -p$(printf %q "${password}")"
      fi
    fi
  fi

  if [ "${try_auto_uninstall}" = "yes" ]; then
    mysql="mysql ${password_options}"
    if ! ${mysql} < ${uninstall_sql}; then
      need_manual_unregister=yes
    fi
  else
    mysql="mysql -u root"
  fi

  if [ "${need_stop}" = "yes" ]; then
    systemctl stop mysql
  fi

  if [ "${need_manual_unregister}" = "yes" ]; then
    echo "Run the following command line to unregister Mroonga:"
    echo "  ${mysql} < ${uninstall_sql}"
  fi
}

uninstall_mroonga
uninstall_apparmor
