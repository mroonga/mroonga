Debian GNU/Linux
================

This section describes how to install Mroonga related deb packages on
Debian GNU/Linux. You can install them by ``apt``.

bookworm (MariaDB)
------------------

Install::

  % sudo apt update
  % sudo apt install -y -V wget lsb-release
  % wget https://apache.jfrog.io/artifactory/arrow/$(lsb_release --id --short | tr 'A-Z' 'a-z')/apache-arrow-apt-source-latest-bookworm.deb
  % sudo apt install -y -V ./apache-arrow-apt-source-latest-bookworm.deb
  % wget https://packages.groonga.org/debian/groonga-apt-source-latest-bookworm.deb
  % sudo apt install -y -V ./groonga-apt-source-latest-bookworm.deb
  % sudo apt update
  % sudo apt install -y -V mariadb-10.11-mroonga

If you want to use `MeCab <https://taku910.github.io/mecab/>`_ as a tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo apt install -y -V groonga-tokenizer-mecab

bookworm (with the Oracle MySQL 8.0 package)
--------------------------------------------

Install::

  % sudo apt update
  % sudo apt install -y -V wget
  % wget https://packages.groonga.org/debian/groonga-apt-source-latest-bookworm.deb
  % wget https://repo.mysql.com/mysql-apt-config.deb
  % sudo apt install -y -V ./groonga-apt-source-latest-bookworm.deb
  % sudo env DEBIAN_FRONTEND=noninteractive MYSQL_SERVER_VERSION=mysql-8.0 apt install -y ./mysql-apt-config.deb
  % sudo apt update
  % sudo apt install -y -V mysql-community-8.0-mroonga

If you want to use `MeCab <https://taku910.github.io/mecab/>`_ as a tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo apt install -y -V groonga-tokenizer-mecab
