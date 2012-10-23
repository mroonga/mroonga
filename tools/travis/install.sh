#!/bin/sh

# set -x
set -e

curl https://raw.github.com/groonga/groonga/master/data/travis/setup.sh | sh
curl https://raw.github.com/clear-code/cutter/master/data/travis/setup.sh | sh
sudo apt-get -qq -y install groonga-tokenizer-mecab

mkdir -p vendor
cd vendor

version=$(echo "$MYSQL_VERSION" | sed -e 's/^\(mysql\|mariadb\)-//')
series=$(echo "$version" | sed -e 's/\.[0-9]*\(-[a-z]*\)\?$//g')
case "$MYSQL_VERSION" in
    mysql-*)
	sudo apt-get -qq -y build-dep mysql-server
	if [ "$version" = "system" ]; then
	    sudo apt-get -qq -y install mysql-server mysql-testsuite
	    apt-get -qq source mysql-server
	    ln -s $(find . -maxdepth 1 -type d | sort | tail -1) mysql
	else
	    download_base="http://cdn.mysql.com/Downloads/MySQL-${series}/"
	    deb=mysql-${version}-debian6.0-i686.deb
	    tar_gz=mysql-${version}.tar.gz
	    curl -O ${download_base}${deb} &
	    curl -O ${download_base}${tar_gz} &
	    wait
	    sudo apt-get -qq -y install libaio1
	    sudo dpkg -i $deb
	    tar xzf $tar_gz
	    ln -s mysql-${version} mysql
	fi
	;;
    mariadb-*)
	distribution=$(lsb_release --short --id | tr 'A-Z' 'a-z')
	code_name=$(lsb_release --short --codename)
	component=main
	apt_url_base="http://ftp.osuosl.org/pub/mariadb/repo/${series}"
	cat <<EOF | sudo tee /etc/apt/sources.list.d/mariadb.list
deb ${apt_url_base}/${distribution}/ ${code_name} ${component}
deb-src ${apt_url_base}/${distribution}/ ${code_name} ${component}
EOF
	sudo apt-get -qq -y build-dep mysql-server
	sudo apt-get -qq -y install mysql-server mysql-testsuite
	apt-get -qq source mysql-server
	ln -s $(find . -maxdepth 1 -type d | sort | tail -1) mysql
	;;
esac

cd ..
