.. highlightlang:: none

Debian GNU/Linux
================

This section describes how to install Mroonga related deb packages on
Debian GNU/Linux. You can install them by ``apt``.

.. include:: 32bit-note.inc

jessie
------

/etc/apt/sources.list.d/groonga.list::

  deb http://packages.groonga.org/debian/ jessie main
  deb-src http://packages.groonga.org/debian/ jessie main

Install::

  % sudo apt-get update
  % sudo apt-get install -y --allow-unauthenticated groonga-keyring
  % sudo apt-get update
  % sudo apt-get install -y -V mysql-server-mroonga

If you want to use `MeCab <http://mecab.sourceforge.net/>`_ as a tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo apt-get install -y -V groonga-tokenizer-mecab

