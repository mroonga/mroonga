.. highlightlang:: none

CentOS
======

This section describes how to install Mroonga related RPM packages on
CentOS. You can install them by ``yum``.

.. include:: 32bit-note.inc

.. _centos-5:

CentOS 5
--------

You use CentOS's MySQL packages (version 5.5.x) on CentOS 5 since
Mroonga 3.09 release.

Install::

  % sudo rpm -ivh http://packages.groonga.org/centos/groonga-release-1.1.0-1.noarch.rpm
  % sudo yum makecache
  % sudo yum install -y mysql55-mysql-server
  % sudo /sbin/service mysql55-mysqld start
  % sudo yum install -y mysql55-mroonga
  (% sudo mysqladmin -u root password 'new-password')

If you want to use `MeCab <http://mecab.sourceforge.net/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo yum install -y groonga-tokenizer-mecab

.. _centos-6-official:

CentOS 6 (with the official MySQL package)
------------------------------------------

You can use CentOS's SCL MySQL packages (version 5.5.x) on CentOS 6
since Mroonga 4.01 release.

Install::

  % sudo yum install -y centos-release-SCL
  % sudo yum install -y http://packages.groonga.org/centos/groonga-release-1.1.0-1.noarch.rpm
  % sudo yum makecache
  % sudo yum install -y mysql55-mysql-server
  % sudo /sbin/service mysql55-mysqld start
  % sudo yum install -y mysql55-mroonga
  (% sudo scl enable mysql55 "mysqladmin -u root password 'new-password'")

If you want to use `MeCab <http://mecab.sourceforge.net/>`_ as a tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo yum install -y groonga-tokenizer-mecab

.. _centos-6-oracle-56:

CentOS 6 (with the Oracle MySQL 5.6 package)
--------------------------------------------

You can use Oracle's MySQL packages version 5.6 on CentOS 6 since
Mroonga 4.04 release.

Install::

  % sudo yum install -y http://packages.groonga.org/centos/groonga-release-1.1.0-1.noarch.rpm
  % sudo yum install -y http://repo.mysql.com/mysql-community-release-el6-7.noarch.rpm
  % sudo yum makecache
  % sudo yum install -y mysql-community-server
  % sudo /sbin/service mysqld start
  % sudo yum install -y mysql-community-mroonga
  (% sudo mysqladmin -u root password 'new-password')

If you want to use `MeCab <http://mecab.sourceforge.net/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo yum install -y groonga-tokenizer-mecab

.. _centos-6-oracle-57:

CentOS 6 (with the Oracle MySQL 5.7 package)
--------------------------------------------

You can use Oracle's MySQL packages version 5.7 on CentOS 6 since
Mroonga 5.09 release.

Install::

  % sudo yum install -y http://packages.groonga.org/centos/groonga-release-1.1.0-1.noarch.rpm
  % sudo yum install -y http://repo.mysql.com/mysql-community-release-el6-7.noarch.rpm
  % sudo yum install -y yum-utils
  % sudo yum-config-manager --disable mysql56-community
  % sudo yum-config-manager --enable mysql57-community
  % sudo yum makecache
  % sudo yum install -y mysql57-community-mroonga
  (% sudo /sbin/service mysqld start)
  (% tmp_password=$(sudo grep 'A temporary password' /var/log/mysqld.log | sed -e 's/^.*: //'))
  (% sudo mysqladmin -u root --password="${tmp_password}" password)

If you want to use `MeCab <http://mecab.sourceforge.net/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo yum install -y groonga-tokenizer-mecab

.. _centos-6-percona:

CentOS 6 (with Percona Server package)
--------------------------------------

You can use Percona Server packages (version 5.6.x) on CentOS 6
since Mroonga 5.02 release.

Install::

  % sudo yum install -y http://packages.groonga.org/centos/groonga-release-1.1.0-1.noarch.rpm
  $ sudo yum install -y http://www.percona.com/downloads/percona-release/redhat/0.1-3/percona-release-0.1-3.noarch.rpm
  % sudo yum makecache
  % sudo yum install -y Percona-Server-server-56
  % sudo /sbin/service mysql start
  % sudo yum install -y percona-server-56-mroonga
  (% sudo mysqladmin -u root password 'new-password')

If you want to use `MeCab <http://mecab.sourceforge.net/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo yum install -y groonga-tokenizer-mecab

.. _centos-7-official:

CentOS 7 (with the official MariaDB package)
--------------------------------------------

You can use CentOS's MariaDB packages (version 5.5.x) on CentOS 7.

Install::

  % sudo yum install -y http://packages.groonga.org/centos/groonga-release-1.1.0-1.noarch.rpm
  % sudo yum makecache
  % sudo yum install -y mariadb-server
  % sudo systemctl start mariadb
  % sudo yum install -y mariadb-mroonga
  (% sudo mysqladmin -u root password 'new-password')

If you want to use `MeCab <http://mecab.sourceforge.net/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo yum install -y groonga-tokenizer-mecab

.. _centos-7-oracle-56:

CentOS 7 (with the Oracle MySQL 5.6 package)
--------------------------------------------

You can use Oracle's MySQL packages version 5.6 on CentOS 7.

Install::

  % sudo yum install -y http://packages.groonga.org/centos/groonga-release-1.1.0-1.noarch.rpm
  % sudo yum install -y http://repo.mysql.com/mysql-community-release-el7-7.noarch.rpm
  % sudo yum makecache
  % sudo yum install -y mysql-community-server
  % sudo systemctl start mysqld
  % sudo yum install -y mysql-community-mroonga
  (% sudo mysqladmin -u root password 'new-password')

If you want to use `MeCab <http://mecab.sourceforge.net/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo yum install -y groonga-tokenizer-mecab

.. _centos-7-oracle-57:

CentOS 7 (with the Oracle MySQL 5.7 package)
--------------------------------------------

You can use Oracle's MySQL packages version 5.7 on CentOS 7 since
Mroonga 5.09 release.

Install::

  % sudo yum install -y http://packages.groonga.org/centos/groonga-release-1.1.0-1.noarch.rpm
  % sudo yum install -y http://repo.mysql.com/mysql-community-release-el7-7.noarch.rpm
  % sudo yum install -y yum-utils
  % sudo yum-config-manager --disable mysql56-community
  % sudo yum-config-manager --enable mysql57-community
  % sudo yum makecache
  % sudo yum install -y mysql57-community-mroonga
  (% sudo systemctl start mysqld)
  (% tmp_password=$(sudo grep 'A temporary password' /var/log/mysqld.log | sed -e 's/^.*: //'))
  (% sudo mysqladmin -u root --password="${tmp_password}" password)

If you want to use `MeCab <http://mecab.sourceforge.net/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo yum install -y groonga-tokenizer-mecab

.. _centos-7-percona:

CentOS 7 (with Percona Server package)
--------------------------------------

You can use Percona Server packages (version 5.6.x) on CentOS 7
since Mroonga 5.02 release.

Install::

  % sudo yum install -y http://packages.groonga.org/centos/groonga-release-1.1.0-1.noarch.rpm
  $ sudo yum install -y http://www.percona.com/downloads/percona-release/redhat/0.1-3/percona-release-0.1-3.noarch.rpm
  % sudo yum makecache
  % sudo yum install -y Percona-Server-server-56
  % sudo /sbin/service mysqld start
  % sudo yum install -y percona-server-56-mroonga
  (% sudo mysqladmin -u root password 'new-password')

If you want to use `MeCab <http://mecab.sourceforge.net/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo yum install -y groonga-tokenizer-mecab
