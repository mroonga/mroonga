.. highlightlang:: none

Debian GNU/Linux
================

This section describes how to install Mroonga related deb packages on
Debian GNU/Linux. You can install them by ``apt``.

.. include:: 32bit-note.inc

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

