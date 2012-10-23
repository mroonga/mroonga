#!/bin/sh

set -x
set -e

base_dir="$(cd $(dirname $0); pwd)"
top_dir="$base_dir/../.."
n_processors="$(grep '^processor' /proc/cpuinfo | wc -l)"
. "${top_dir}/config.sh"

build()
{
    make -j${n_processors} > /dev/null
}

run_unit_test()
{
    NO_MAKE=yes test/run-unit-test.sh
}

prepare_mysql_test_dir()
{
    mysql_test_dir=/usr/mysql-test
    if [ -d /usr/lib/mysql-testsuite/ ]; then
	sudo cp -a /usr/lib/mysql-testsuite/ ${mysql_test_dir}/
    elif [ -d /opt/mysql/ ]; then
	mysql_test_dir=$(echo /opt/mysql/server-*/mysql-test)
    else
	sudo cp -a ${MYSQL_SOURCE_DIR}/mysql-test/ ${mysql_test_dir}/
    fi
    sudo chown -R $(id -u):$(id -g) ${mysql_test_dir}/

    cp -a ${top_dir}/test/sql/include/*.inc ${mysql_test_dir}/include/
    cp -a ${top_dir}/test/sql/suite/mroonga/ ${mysql_test_dir}/suite/
}

collect_test_suite_names()
{
    cd ${mysql_test_dir}/suite/
    test_suite_names=""
    for test_suite_name in $(find mroonga -type d '!' -name '[tr]'); do
	if [ -n "${test_suite_names}" ]; then
	    test_suite_names="${test_suite_names},"
	fi
	test_suite_names="${test_suite_names}${test_suite_name}"
    done
    cd -
}

prepare_sql_test()
{
    sudo make install > /dev/null
    prepare_mysql_test_dir
    collect_test_suite_names
}

run_sql_test()
{
    prepare_sql_test

    cd ${mysql_test_dir}/
    ./mysql-test-run.pl \
	--no-check-testcases \
	--parallel="${n_processors}" \
	--retry=1 \
	--suite="${test_suite_names}" \
	--force
}

build
run_unit_test
run_sql_test
