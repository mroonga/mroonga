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
    MYSQL_VERSION="$(make -s -C $top_dir echo-mysql-version)"
fi
export MYSQL_VERSION

test_suite_names="groonga_storage,groonga_wrapper"
source_mysql_test_dir="${MYSQL_SOURCE}/mysql-test"
build_mysql_test_dir="${MYSQL_BUILD}/mysql-test"
source_test_suites_dir="${source_mysql_test_dir}/suite"
build_test_suites_dir="${build_mysql_test_dir}/suite"
case "${MYSQL_VERSION}" in
    5.1.*)
	plugins_dir="${MYSQL_BUILD}/lib/mysql/plugin"
	if ! test -d "${build_test_suites_dir}"; then
	    mkdir -p "${build_test_suites_dir}"
	fi
	;;
    *-MariaDB*)
	if ! test -d "${build_test_suites_dir}"; then
	    ln -s "${source_test_suites_dir}" "${build_test_suites_dir}"
	fi
	if ! test -d "${MYSQL_BUILD}/plugin/mroonga"; then
	    ln -s "${top_dir}" "${MYSQL_BUILD}/plugin/mroonga"
	fi
	;;
    *)
	if ! test -d "${build_test_suites_dir}"; then
	    ln -s "${source_test_suites_dir}" "${build_test_suites_dir}"
	fi
	plugins_dir="${MYSQL_SOURCE}/lib/plugin"
	;;
esac

for test_suite_name in groonga_include $(echo $test_suite_names | sed -e 's/,/ /g'); do
    local_groonga_mysql_test_suite_dir="${BASE_DIR}/sql/${test_suite_name}"
    groonga_mysql_test_suite_dir="${build_test_suites_dir}/${test_suite_name}"
    if ! test -e "${groonga_mysql_test_suite_dir}"; then
	ln -s "${local_groonga_mysql_test_suite_dir}" \
	    "${groonga_mysql_test_suite_dir}"
    fi
done

if test -n "${plugins_dir}"; then
    make -C ${top_dir} \
	install-pluginLTLIBRARIES \
	plugindir=${plugins_dir} > /dev/null || \
	exit 1
else
    make -C ${top_dir} > /dev/null || exit 1
fi

(cd "$build_mysql_test_dir" && \
    ./mysql-test-run.pl \
    --no-check-testcases \
    --retry=1 \
    --suite="${test_suite_names}" \
    --force \
    "$@")
