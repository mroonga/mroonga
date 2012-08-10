#!/bin/sh

set -x
set -e

mkdir -p vendor
cd vendor
if [ "$MYSQL_VERSION" = "system" ]; then
    sudo apt-get -y build-dep mysql-server
    apt-cache show mysql-server
    sudo cat /etc/apt/sources.list
    sudo cat /etc/apt/sources.list.d/*.list
    apt-get source mysql-server
    ln -s $(find . -maxdepth 1 -type d | sort | tail -1) mysql
    cd mysql
    debuild -us -uc -Tconfigure
    make -j$(grep '^processor' /proc/cpuinfo | wc -l) > /dev/null
    cd ..
else
    sudo apt-get -y install cmake
    if [ "$MYSQL_TYPE" = "mariadb" ]; then
	wget http://mirror3.layerjet.com/mariadb/mariadb-${MYSQL_VERSION}/kvm-tarbake-jaunty-x86/mariadb-${MYSQL_VERSION}.tar.gz
	tar xzf mariadb-${MYSQL_VERSION}.tar.gz
	ln -s mariadb-${MYSQL_VERSION} mysql
    else
	wget http://cdn.mysql.com/Downloads/MySQL-5.5/mysql-${MYSQL_VERSION}.tar.gz
	tar xzf mysql-${MYSQL_VERSION}.tar.gz
	ln -s mysql-${MYSQL_VERSION} mysql
    fi
    cd mysql
    BUILD/compile-amd64-max
    cd ..
fi
cd ..

touch NEWS # For old automake
./autogen.sh
./configure \
    --with-mysql-source=$PWD/vendor/mysql \
    --with-mysql-config=$PWD/vendor/mysql/scripts/mysql_config
