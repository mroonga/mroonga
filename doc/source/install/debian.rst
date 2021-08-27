Debian GNU/Linux
================

This section describes how to install Mroonga related deb packages on
Debian GNU/Linux. You can install them by ``apt``.

.. include:: 32bit-note.inc

bullseye (MariaDB)
------------------

Install::

  % sudo apt update
  % sudo apt install -y -V apt-transport-https
  % sudo apt install -y -V wget
  % wget https://packages.groonga.org/debian/groonga-apt-source-latest-bullseye.deb
  % sudo apt install -y -V ./groonga-apt-source-latest-bullseye.deb
  % sudo apt update
  % sudo apt install -y -V mariadb-server-10.5-mroonga

If you want to use `MeCab <https://taku910.github.io/mecab/>`_ as a tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo apt install -y -V groonga-tokenizer-mecab

buster (MariaDB)
----------------

Install::

  % sudo apt update
  % sudo apt install -y -V apt-transport-https
  % sudo apt install -y -V wget
  % wget https://packages.groonga.org/debian/groonga-apt-source-latest-buster.deb
  % sudo apt install -y -V ./groonga-apt-source-latest-buster.deb
  % sudo apt update
  % sudo apt install -y -V mariadb-server-10.3-mroonga

If you want to use `MeCab <https://taku910.github.io/mecab/>`_ as a tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo apt install -y -V groonga-tokenizer-mecab

buster (with the Oracle MySQL 5.7 package)
------------------------------------------

Install::

  % sudo apt update
  % sudo apt install -y -V apt-transport-https
  % sudo apt install -y -V wget
  % wget https://packages.groonga.org/debian/groonga-apt-source-latest-buster.deb
  % wget https://repo.mysql.com/mysql-apt-config_0.8.17-1_all.deb
  % sudo apt install -y -V ./groonga-apt-source-latest-buster.deb
  % sudo env DEBIAN_FRONTEND=noninteractive MYSQL_SERVER_VERSION=mysql-5.7 apt install -y ./mysql-apt-config_0.8.17-1_all.deb
  % sudo apt update
  % sudo apt install -y -V mysql-community-5.7-mroonga

If you want to use `MeCab <https://taku910.github.io/mecab/>`_ as a tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo apt install -y -V groonga-tokenizer-mecab

buster (with the Oracle MySQL 8.0 package)
------------------------------------------

Install::

  % sudo apt update
  % sudo apt install -y -V apt-transport-https
  % sudo apt install -y -V wget
  % wget https://packages.groonga.org/debian/groonga-apt-source-latest-buster.deb
  % wget https://repo.mysql.com/mysql-apt-config_0.8.17-1_all.deb
  % sudo apt install -y -V ./groonga-apt-source-latest-buster.deb
  % sudo env DEBIAN_FRONTEND=noninteractive MYSQL_SERVER_VERSION=mysql-8.0 apt install -y ./mysql-apt-config_0.8.17-1_all.deb
  % sudo apt update
  % sudo apt install -y -V mysql-community-8.0-mroonga

If you want to use `MeCab <https://taku910.github.io/mecab/>`_ as a tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo apt install -y -V groonga-tokenizer-mecab
