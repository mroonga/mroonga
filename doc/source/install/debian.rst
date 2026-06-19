Debian GNU/Linux
================

This section describes how to install Mroonga related deb packages on
Debian GNU/Linux. You can install them by ``apt``.

trixie (MariaDB)
----------------

Install::

  $ sudo apt update
  $ sudo apt install -y -V wget
  $ wget https://packages.apache.org/artifactory/arrow/debian/apache-arrow-apt-source-latest-trixie.deb
  $ sudo apt install -y -V ./apache-arrow-apt-source-latest-trixie.deb
  $ rm -rf apache-arrow-apt-source-latest-trixie.deb
  $ wget https://packages.groonga.org/debian/groonga-apt-source-latest-trixie.deb
  $ sudo apt install -y -V ./groonga-apt-source-latest-trixie.deb
  $ rm -rf groonga-apt-source-latest-trixie.deb
  $ sudo apt update
  $ sudo apt install -y -V mariadb-11.8-mroonga

If you want to use `MeCab <https://taku910.github.io/mecab/>`_ as a tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  $ sudo apt install -y -V groonga-tokenizer-mecab

trixie (with the Oracle MySQL 8.4 package)
------------------------------------------

Install::

  $ sudo apt update
  $ sudo apt install -y -V wget
  $ wget https://packages.groonga.org/debian/groonga-apt-source-latest-trixie.deb
  $ sudo apt install -y -V ./groonga-apt-source-latest-trixie.deb
  $ rm -rf groonga-apt-source-latest-trixie.deb
  $ wget https://repo.mysql.com/mysql-apt-config.deb
  $ sudo env DEBIAN_FRONTEND=noninteractive MYSQL_SERVER_VERSION=mysql-8.4-lts apt install -y ./mysql-apt-config.deb
  $ rm -rf mysql-apt-config.deb
  $ sudo apt update
  $ sudo apt install -y -V mysql-community-8.4-mroonga

If you want to use `MeCab <https://taku910.github.io/mecab/>`_ as a tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  $ sudo apt install -y -V groonga-tokenizer-mecab
