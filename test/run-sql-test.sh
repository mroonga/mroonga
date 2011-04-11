#!/bin/sh

export BASE_DIR="$(cd $(dirname $0); pwd)"
top_dir="$BASE_DIR/.."

if test -z "$MYSQL_SOURCE"; then
    MYSQL_SOURCE="$(make -s -C $top_dir echo-mysql-source)"
fi
export MYSQL_SOURCE

if test -z "$MYSQL_BUILD"; then
    MYSQL_BUILD="$(make -s -C $top_dir echo-mysql-build)"
fi
export MYSQL_BUILD

if test -z "$MYSQL_VERSION"; then
    MYSQL_VERSEION="$(make -s -C $top_dir echo-mysql-version)"
fi
export MYSQL_VERSION

test_suite_name="groonga"
local_groonga_mysql_test_suite_dir="${BASE_DIR}/sql"
source_mysql_test_dir="${MYSQL_SOURCE}/mysql-test"
build_mysql_test_dir="${MYSQL_BUILD}/mysql-test"
source_test_suites_dir="${source_mysql_test_dir}/suite"
build_test_suites_dir="${build_mysql_test_dir}/suite"
groonga_mysql_test_suite_dir="${build_test_suites_dir}/${test_suite_name}"
case "${MYSQL_VERSION}" in
    5.1)
	plugins_dir="${MYSQL_BUILD}/lib/mysql/plugin"
	if ! test -d "${build_test_suites_dir}"; then
	    mkdir -p "${build_test_suites_dir}"
	fi
	;;
    *)
	if ! test -d "${build_test_suites_dir}"; then
	    ln -s "${source_test_suites_dir}" "${build_test_suites_dir}"
	fi
	plugins_dir="${MYSQL_SOURCE}/lib/plugin"
	;;
esac

if ! test -e "${groonga_mysql_test_suite_dir}"; then
    ln -s "${local_groonga_mysql_test_suite_dir}" \
	"${groonga_mysql_test_suite_dir}"
fi

make -C ${top_dir} install-pluginLTLIBRARIES plugindir=${plugins_dir} > /dev/null

(cd "$build_mysql_test_dir" && \
    ./mysql-test-run.pl --no-check-testcases \
    --suite="${test_suite_name}" --force "$@")
