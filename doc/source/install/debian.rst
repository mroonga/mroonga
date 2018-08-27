.. highlightlang:: none

Debian GNU/Linux
================

This section describes how to install Mroonga related deb packages on
Debian GNU/Linux. You can install them by ``apt``.

.. include:: 32bit-note.inc

stretch (MariaDB)
-----------------

/etc/apt/sources.list.d/groonga.list::

  deb https://packages.groonga.org/debian/ stretch main
  deb-src https://packages.groonga.org/debian/ stretch main

Install::

  % sudo apt install -y -V apt-transport-https
  % sudo apt update --allow-insecure-repositories
  % sudo apt install -y -V --allow-unauthenticated groonga-keyring
  % sudo apt update
  % sudo apt install -y -V mariadb-server-10.1-mroonga

If you want to use `MeCab <http://mecab.sourceforge.net/>`_ as a tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo apt install -y -V groonga-tokenizer-mecab

