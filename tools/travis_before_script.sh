#!/bin/sh

set -x
set -e

touch NEWS # For old automake
./autogen.sh

PATH=$PWD/vendor/mysql/scripts:$PATH
./configure \
    --with-mysql-source=$PWD/vendor/mysql
