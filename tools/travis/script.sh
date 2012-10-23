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
    if [ -d /usr/lib/mysql-testsuite/ ]; then
	sudo cp -a /usr/lib/mysql-testsuite/ /usr/mysql-test/
    elif [ -d /opt/mysql/ ]; then
	sudo ln -s /opt/mysql/server-*/mysql-test/ /usr/mysql-test/
    else
	sudo cp -a ${MYSQL_SOURCE_DIR}/mysql-test/ /usr/mysql-test/
    fi
    (cd /usr/mysql-test && sudo chown -R $(id -u):$(id -g) .)

    cp -a ${top_dir}/test/sql/include/*.inc /usr/mysql-test/include/
    cp -a ${top_dir}/test/sql/suite/mroonga/ /usr/mysql-test/suite/
}

collect_test_suite_names()
{
    cd /usr/mysql-test/suite/
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

    cd /usr/mysql-test
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
