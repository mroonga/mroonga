#!/bin/sh

export BASE_DIR="$(cd $(dirname $0); pwd)"
top_dir="$BASE_DIR/.."

if test "$NO_MAKE" != "yes"; then
    make -C ${top_dir} > /dev/null || exit 1
fi

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

test_suite_names="mroonga_storage,mroonga_wrapper"
source_mysql_test_dir="${MYSQL_SOURCE}/mysql-test"
build_mysql_test_dir="${MYSQL_BUILD}/mysql-test"
source_test_suites_dir="${source_mysql_test_dir}/suite"
build_test_suites_dir="${build_mysql_test_dir}/suite"
build_test_include_dir="${build_mysql_test_dir}/include"
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

same_link_p()
{
    src=$1
    dest=$2
    if test -L "$dest" -a "$(readlink "$dest")" = "$src"; then
	return 0
    else
	return 1
    fi
}

local_mroonga_mysql_test_include_dir="${BASE_DIR}/sql/include"
for test_include_name in $(ls $local_mroonga_mysql_test_include_dir | grep '\.inc$'); do
    local_mroonga_mysql_test_include="${local_mroonga_mysql_test_include_dir}/${test_include_name}"
    mroonga_mysql_test_include="${build_test_include_dir}/${test_include_name}"
    if ! same_link_p "${local_mroonga_mysql_test_include}" \
			"${mroonga_mysql_test_include}"; then
	rm -f "${mroonga_mysql_test_include}"
        ln -s "${local_mroonga_mysql_test_include}" \
	    "${mroonga_mysql_test_include}"
    fi
done

for test_suite_name in $(echo $test_suite_names | sed -e 's/,/ /g'); do
    local_mroonga_mysql_test_suite_dir="${BASE_DIR}/sql/suite/${test_suite_name}"
    mroonga_mysql_test_suite_dir="${build_test_suites_dir}/${test_suite_name}"
    if ! same_link_p "${local_mroonga_mysql_test_suite_dir}" \
			"${mroonga_mysql_test_suite_dir}"; then
	rm -f "${mroonga_mysql_test_suite_dir}"
	ln -s "${local_mroonga_mysql_test_suite_dir}" \
	    "${mroonga_mysql_test_suite_dir}"
    fi
done

innodb_test_suite_dir="${build_test_suites_dir}/innodb"
mroonga_wrapper_innodb_test_suite_dir="${build_test_suites_dir}/mroonga_wrapper_innodb"
if test "$0" -nt "$(dirname "${mroonga_wrapper_innodb_test_suite_dir}")"; then
    rm -rf "${mroonga_wrapper_innodb_test_suite_dir}"
fi
if ! test -d "${mroonga_wrapper_innodb_test_suite_dir}"; then
    cp -rp "${innodb_test_suite_dir}" "${mroonga_wrapper_innodb_test_suite_dir}"
    ruby -i'' \
	-pe "\$_.gsub!(/\bengine\s*=\s*innodb\b([^;\\n]*)/i,
                      \"ENGINE=mroonga\\\1 COMMENT='ENGINE \\\"InnoDB\\\"'\")
            " \
	${mroonga_wrapper_innodb_test_suite_dir}/r/*.result \
	${mroonga_wrapper_innodb_test_suite_dir}/t/*.test \
	${mroonga_wrapper_innodb_test_suite_dir}/include/*.inc \
	${build_test_include_dir}/innodb_*.inc # over our work. :<
    sed -i'' \
	-e '1 i --source include/have_mroonga.inc' \
	${mroonga_wrapper_innodb_test_suite_dir}/t/*.test
fi

if test -n "${plugins_dir}"; then
    make -C ${top_dir} \
	install-pluginLTLIBRARIES \
	plugindir=${plugins_dir} > /dev/null || \
	exit 1
fi

(cd "$build_mysql_test_dir" && \
    ./mysql-test-run.pl \
    --no-check-testcases \
    --retry=1 \
    --suite="${test_suite_names}" \
    --force \
    "$@")
