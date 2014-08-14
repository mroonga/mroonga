.. highlightlang:: none

Fedora
======

This section describes how to install Mroonga related RPM packages on
Fedora. You can install them by ``yum``.

.. include:: 32bit-note.inc

Fedora 20
---------

Since Fedora 19, MariaDB is the default implementation of MySQL.

So there are two selections for Mroonga. One is using with MariaDB, the other is using with MySQL (community-mysql).

Install Mroonga for MySQL (community-mysql)::

  % sudo rpm -ivh http://packages.groonga.org/fedora/groonga-release-1.1.0-1.noarch.rpm
  % sudo yum makecache
  % sudo yum install -y mysql-mroonga

Install Mroonga for MariaDB::

  % sudo rpm -ivh http://packages.groonga.org/fedora/groonga-release-1.1.0-1.noarch.rpm
  % sudo yum makecache
  % sudo yum install -y mariadb-mroonga

.. note::

   MariaDB and MySQL (community-mysql) package are exclusive. For example, if you want to use mysql-mroonga, you need to remove conflicted mariadb packages at first.

If you want to use `MeCab <http://mecab.sourceforge.net/>`_ as a tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo yum install -y groonga-tokenizer-mecab
