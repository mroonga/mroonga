#!/bin/sh

set -x
set -e

curl https://raw.github.com/groonga/groonga/master/data/travis/setup.sh | sh
curl https://raw.github.com/clear-code/cutter/master/data/travis/setup.sh | sh
sudo apt-get -y install groonga-tokenizer-mecab

mkdir -p vendor
cd vendor
if [ "$MYSQL_VERSION" = "system" ]; then
    sudo apt-get -y build-dep mysql-server
    sudo apt-get -y -V install mysql-server
    apt-get source mysql-server
    ln -s $(find . -maxdepth 1 -type d | sort | tail -1) mysql
elif [ -n "$MARIADB_VERSION" ]; then
    distribution=$(lsb_release --short --id | tr 'A-Z' 'a-z')
    code_name=$(lsb_release --short --codename)
    component=main
    seriese=$(echo "$MARIADB_VERSION" | sed -e 's/\.[0-9]*$//g')
    apt_url_base="http://ftp.osuosl.org/pub/mariadb/repo/${seriese}"
    cat <<EOF | sudo tee /etc/apt/sources.list.d/mariadb.list
deb ${apt_url_base}/${distribution}/ ${code_name} ${component}
deb-src ${apt_url_base}/${distribution}/ ${code_name} ${component}
EOF
    sudo apt-get -y build-dep mysql-server
    sudo apt-get -y -V install mysql-server
    apt-get source mysql-server
    ln -s $(find . -maxdepth 1 -type d | sort | tail -1) mysql
else
    sudo apt-get -y install cmake
    wget http://cdn.mysql.com/Downloads/MySQL-5.5/mysql-${MYSQL_VERSION}.tar.gz
    tar xzf mysql-${MYSQL_VERSION}.tar.gz
    ln -s mysql-${MYSQL_VERSION} mysql
    cd mysql
    BUILD/compile-amd64-max
    cd ..
fi
cd ..
