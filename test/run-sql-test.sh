#!/bin/sh

export BASE_DIR="$(cd $(dirname $0); pwd)"
top_dir="$BASE_DIR/.."

if test -z "$MYSQL_BUILD"; then
    MYSQL_BUILD="$(make -s -C $BASE_DIR echo-mysql-build)"
fi
export MYSQL_BUILD

test_suite_name="groonga"
local_groonga_mysql_test_suite_dir="${BASE_DIR}/sql"
mysql_test_dir="${MYSQL_BUILD}/mysql-test"
test_suites_dir="${mysql_test_dir}/suite"
groonga_mysql_test_suite_dir="${test_suites_dir}/${test_suite_name}"
plugins_dir="${MYSQL_BUILD}/lib/mysql/plugin"

if ! test -e "${groonga_mysql_test_suite_dir}"; then
    ln -s "${local_groonga_mysql_test_suite_dir}" \
	"${groonga_mysql_test_suite_dir}"
fi

make -C ${top_dir} install-pluginLTLIBRARIES plugindir=${plugins_dir}

(cd "$mysql_test_dir" && \
    ./mysql-test-run.pl --no-check-testcases --suite="${test_suite_name}" --force)
