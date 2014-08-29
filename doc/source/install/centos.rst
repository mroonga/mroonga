.. highlightlang:: none

CentOS
======

This section describes how to install Mroonga related RPM packages on
CentOS. You can install them by ``yum``.

.. include:: 32bit-note.inc

CentOS 5
--------

In CentOS 5, we use CentOS's MySQL packages (version 5.5.x) since Mroonga 3.09 release.

Install::

  % sudo rpm -ivh http://packages.groonga.org/centos/groonga-release-1.1.0-1.noarch.rpm
  % sudo yum makecache
  % sudo yum install -y mysql55-mysql-server
  % sudo /etc/init.d/mysql55-mysqld start
  % sudo yum install -y mysql55-mroonga
  (% sudo scl enable mysql55 mysqladmin -u root password 'new-password')

If you want to use `MeCab <http://mecab.sourceforge.net/>`_ as a tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo yum install -y groonga-tokenizer-mecab

CentOS 5 (Deprecated)
---------------------

The old version of Mroonga had also provided MySQL 5.6 packages as exceptional case. This is obsolete and not recommended way.
The mysql-mroonga package is deprecated on CentOS 5 since Mroonga 3.09.
We recommend to use mysql55-mroonga pakcages on CentOS 5.

CentOS 5's MySQL packages should be removed beforehand if installed.

Remove existing MySQL packages ::

  % sudo yum remove 'mysql*'

Install::

  % sudo rpm -ivh http://packages.groonga.org/centos/groonga-release-1.1.0-1.noarch.rpm
  % sudo yum makecache
  % sudo yum install -y MySQL-server
  % sudo service mysql start
  % sudo yum install -y mysql-mroonga
  (% /usr/bin/mysqladmin -u root password 'new-password')

CentOS 6
--------

In CentOS 6, we use CentOS's SCL MySQL packages (version 5.5.x) since Mroonga 4.01 release.

Install::

  % sudo yum install centos-release-SCL
  % sudo rpm -ivh http://packages.groonga.org/centos/groonga-release-1.1.0-1.noarch.rpm
  % sudo yum makecache
  % sudo yum install -y mysql55-mysql-server
  % sudo /etc/init.d/mysql55-mysqld start
  % sudo yum install -y mysql55-mroonga
  (% sudo scl enable mysql55 "mysqladmin -u root password 'new-password'")

If you want to use `MeCab <http://mecab.sourceforge.net/>`_ as a tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo yum install -y groonga-tokenizer-mecab

CentOS 6 (Deprecated)
---------------------

In CentOS 6, unlike in CentOS 5, we use CentOS's MySQL packages (version 5.1.x). So you don't need to remove CentOS's MySQL packages.

.. note::

   Since Mroonga 4.01 release, mysql-mroonga package is marked as deprecated. Please use mysql55-mroonga package instead.

Install::

  % sudo rpm -ivh http://packages.groonga.org/centos/groonga-release-1.1.0-1.noarch.rpm
  % sudo yum makecache
  % sudo yum install -y mysql-server
  % sudo service mysqld start
  % sudo yum install -y mysql-mroonga
  (% /usr/bin/mysqladmin -u root password 'new-password')

If you want to use `MeCab <http://mecab.sourceforge.net/>`_ as a tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo yum install -y groonga-tokenizer-mecab

CentOS 7
--------

In CentOS 7, we use CentOS's MariaDB packages (version 5.5.x).

Install::

  % sudo rpm -ivh http://packages.groonga.org/centos/groonga-release-1.1.0-1.noarch.rpm
  % sudo yum makecache
  % sudo yum install -y mariadb-server
  % sudo systemctl start mariadb
  % sudo yum install -y mariadb-mroonga
  (% sudo mysqladmin -u root password 'new-password')

If you want to use `MeCab <http://mecab.sourceforge.net/>`_ as a tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo yum install -y groonga-tokenizer-mecab

