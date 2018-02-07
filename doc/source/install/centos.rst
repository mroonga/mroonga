.. highlightlang:: none

CentOS
======

This section describes how to install Mroonga related RPM packages on
CentOS. You can install them by ``yum``.

.. include:: 32bit-note.inc

.. _centos-6-official:

CentOS 6 (with the official MySQL package)
------------------------------------------

You can use CentOS's SCL MySQL packages (version 5.5.x) on CentOS 6
since Mroonga 4.01 release.

Install::

  % sudo yum install -y centos-release-scl
  % sudo yum install -y https://packages.groonga.org/centos/groonga-release-1.3.0-1.noarch.rpm
  % sudo yum install -y mysql55-mysql-server
  % sudo /sbin/service mysql55-mysqld start
  % sudo yum install -y --enablerepo=epel mysql55-mroonga
  (% sudo scl enable mysql55 "mysqladmin -u root password 'new-password'")

If you want to use `MeCab <http://mecab.sourceforge.net/>`_ as a tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo yum install -y --enablerepo=epel groonga-tokenizer-mecab

.. _centos-6-oracle-56:

CentOS 6 (with the Oracle MySQL 5.6 package)
--------------------------------------------

You can use Oracle's MySQL packages version 5.6 on CentOS 6 since
Mroonga 4.04 release.

Install::

  % sudo yum install -y https://packages.groonga.org/centos/groonga-release-1.3.0-1.noarch.rpm
  % sudo yum install -y http://repo.mysql.com/mysql-community-release-el6-7.noarch.rpm
  % sudo yum install -y --enablerepo=epel mysql-community-mroonga
  (% sudo /sbin/service mysqld start)
  (% sudo mysqladmin -u root password 'new-password')

If you want to use `MeCab <http://mecab.sourceforge.net/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo yum install -y --enablerepo=epel groonga-tokenizer-mecab

.. _centos-6-oracle-57:

CentOS 6 (with the Oracle MySQL 5.7 package)
--------------------------------------------

You can use Oracle's MySQL packages version 5.7 on CentOS 6 since
Mroonga 5.09 release.

Install::

  % sudo yum install -y https://packages.groonga.org/centos/groonga-release-1.3.0-1.noarch.rpm
  % sudo yum install -y http://repo.mysql.com/mysql-community-release-el6-7.noarch.rpm
  % sudo yum install -y yum-utils
  % sudo yum-config-manager --disable mysql56-community
  % sudo yum-config-manager --enable mysql57-community
  % sudo yum install -y --enablerepo=epel mysql57-community-mroonga
  (% sudo /sbin/service mysqld start)
  (% tmp_password=$(sudo grep 'A temporary password' /var/log/mysqld.log | sed -e 's/^.*: //'))
  (% sudo mysqladmin -u root --password="${tmp_password}" password)

If you want to use `MeCab <http://mecab.sourceforge.net/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo yum install -y --enablerepo=epel groonga-tokenizer-mecab

.. _centos-6-percona-56:

CentOS 6 (with Percona Server 5.6 package)
------------------------------------------

You can use Percona Server packages version 5.6 on CentOS 6
since Mroonga 5.02 release.

Install::

  % sudo yum install -y https://packages.groonga.org/centos/groonga-release-1.3.0-1.noarch.rpm
  $ sudo yum install -y http://www.percona.com/downloads/percona-release/redhat/0.1-4/percona-release-0.1-4.noarch.rpm
  % sudo yum install -y Percona-Server-server-56
  % sudo /sbin/service mysql start
  % sudo yum install -y --enablerepo=epel percona-server-56-mroonga
  (% sudo mysqladmin -u root password 'new-password')

If you want to use `MeCab <http://mecab.sourceforge.net/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo yum install -y groonga-tokenizer-mecab

.. _centos-6-percona-57:

CentOS 6 (with Percona Server 5.7 package)
------------------------------------------

You can use Percona Server packages version 5.7 on CentOS 6
since Mroonga 6.02 release.

Install::

  % sudo yum install -y https://packages.groonga.org/centos/groonga-release-1.3.0-1.noarch.rpm
  $ sudo yum install -y http://www.percona.com/downloads/percona-release/redhat/0.1-4/percona-release-0.1-4.noarch.rpm
  % sudo yum install -y --enablerepo=epel percona-server-57-mroonga
  (% sudo /sbin/service mysql start)
  (% tmp_password=$(sudo grep 'A temporary password' /var/log/mysqld.log | sed -e 's/^.*: //'))
  (% sudo mysqladmin -u root --password="${tmp_password}" password)

If you want to use `MeCab <http://mecab.sourceforge.net/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo yum install -y --enablerepo=epel groonga-tokenizer-mecab


.. _centos-6-mariadb-10-1:

CentOS 6 (with MariaDB 10.1 package)
------------------------------------

You can use MariaDB's MariaDB packages version 10.1 on CentOS 6 since
Mroonga 7.06 release.

Create ``/etc/yum.repos.d/MariaDB.repo``.

For 32-bit version::

  [mariadb]
  name = MariaDB
  baseurl = http://yum.mariadb.org/10.1/centos6-x86
  gpgkey=https://yum.mariadb.org/RPM-GPG-KEY-MariaDB
  gpgcheck=1

For 64-bit version::

  [mariadb]
  name = MariaDB
  baseurl = http://yum.mariadb.org/10.1/centos6-amd64
  gpgkey=https://yum.mariadb.org/RPM-GPG-KEY-MariaDB
  gpgcheck=1

Install::

  % sudo yum install -y https://packages.groonga.org/centos/groonga-release-1.3.0-1.noarch.rpm
  % sudo yum install -y MariaDB-server
  % sudo systemctl start mariadb
  % sudo yum install -y --enablerepo=epel mariadb-10.1-mroonga
  (% sudo mysqladmin -u root password 'new-password')

If you want to use `MeCab <http://mecab.sourceforge.net/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo yum install -y --enablerepo=epel groonga-tokenizer-mecab

.. _centos-6-mariadb-10-2:

CentOS 6 (with MariaDB 10.2 package)
------------------------------------

You can use MariaDB's MariaDB packages version 10.2 on CentOS 6 since
Mroonga 7.06 release.

Create ``/etc/yum.repos.d/MariaDB.repo``.

For 32-bit version::

  [mariadb]
  name = MariaDB
  baseurl = http://yum.mariadb.org/10.2/centos6-x86
  gpgkey=https://yum.mariadb.org/RPM-GPG-KEY-MariaDB
  gpgcheck=1

For 64-bit version::

  [mariadb]
  name = MariaDB
  baseurl = http://yum.mariadb.org/10.2/centos6-amd64
  gpgkey=https://yum.mariadb.org/RPM-GPG-KEY-MariaDB
  gpgcheck=1

Install::

  % sudo yum install -y https://packages.groonga.org/centos/groonga-release-1.3.0-1.noarch.rpm
  % sudo yum install -y MariaDB-server
  % sudo systemctl start mariadb
  % sudo yum install -y --enablerepo=epel mariadb-10.2-mroonga
  (% sudo mysqladmin -u root password 'new-password')

If you want to use `MeCab <http://mecab.sourceforge.net/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo yum install -y --enablerepo=epel groonga-tokenizer-mecab

.. _centos-6-mariadb-10-3:

CentOS 6 (with MariaDB 10.3 package)
------------------------------------

You can use MariaDB's MariaDB packages version 10.3 on CentOS 6 since
Mroonga 8.00 release.

Create ``/etc/yum.repos.d/MariaDB.repo``.

For 32-bit version::

  [mariadb]
  name = MariaDB
  baseurl = http://yum.mariadb.org/10.3/centos6-x86
  gpgkey=https://yum.mariadb.org/RPM-GPG-KEY-MariaDB
  gpgcheck=1

For 64-bit version::

  [mariadb]
  name = MariaDB
  baseurl = http://yum.mariadb.org/10.3/centos6-amd64
  gpgkey=https://yum.mariadb.org/RPM-GPG-KEY-MariaDB
  gpgcheck=1

Install::

  % sudo yum install -y https://packages.groonga.org/centos/groonga-release-1.3.0-1.noarch.rpm
  % sudo yum install -y MariaDB-server
  % sudo systemctl start mariadb
  % sudo yum install -y --enablerepo=epel mariadb-10.3-mroonga
  (% sudo mysqladmin -u root password 'new-password')

If you want to use `MeCab <http://mecab.sourceforge.net/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo yum install -y --enablerepo=epel groonga-tokenizer-mecab

.. _centos-7-official:

CentOS 7 (with the official MariaDB package)
--------------------------------------------

You can use CentOS's MariaDB packages (version 5.5.x) on CentOS 7.

Install::

  % sudo yum install -y https://packages.groonga.org/centos/groonga-release-1.3.0-1.noarch.rpm
  % sudo yum install -y mariadb-server
  % sudo systemctl start mariadb
  % sudo yum install -y --enablerepo=epel mariadb-mroonga
  (% sudo mysqladmin -u root password 'new-password')

If you want to use `MeCab <http://mecab.sourceforge.net/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo yum install -y --enablerepo=epel groonga-tokenizer-mecab

.. _centos-7-oracle-56:

CentOS 7 (with the Oracle MySQL 5.6 package)
--------------------------------------------

You can use Oracle's MySQL packages version 5.6 on CentOS 7.

Install::

  % sudo yum install -y https://packages.groonga.org/centos/groonga-release-1.3.0-1.noarch.rpm
  % sudo yum install -y http://repo.mysql.com/mysql-community-release-el7-7.noarch.rpm
  % sudo yum install -y --enablerepo=epel mysql-community-mroonga
  (% sudo systemctl start mysqld)
  (% sudo mysqladmin -u root password 'new-password')

If you want to use `MeCab <http://mecab.sourceforge.net/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo yum install -y --enablerepo=epel groonga-tokenizer-mecab

.. _centos-7-oracle-57:

CentOS 7 (with the Oracle MySQL 5.7 package)
--------------------------------------------

You can use Oracle's MySQL packages version 5.7 on CentOS 7 since
Mroonga 5.09 release.

Install::

  % sudo yum install -y https://packages.groonga.org/centos/groonga-release-1.3.0-1.noarch.rpm
  % sudo yum install -y http://repo.mysql.com/mysql-community-release-el7-7.noarch.rpm
  % sudo yum install -y yum-utils
  % sudo yum-config-manager --disable mysql56-community
  % sudo yum-config-manager --enable mysql57-community
  % sudo yum install -y --enablerepo=epel mysql57-community-mroonga
  (% sudo systemctl start mysqld)
  (% tmp_password=$(sudo grep 'A temporary password' /var/log/mysqld.log | sed -e 's/^.*: //'))
  (% sudo mysqladmin -u root --password="${tmp_password}" password)

If you want to use `MeCab <http://mecab.sourceforge.net/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo yum install -y --enablerepo=epel groonga-tokenizer-mecab

.. _centos-7-percona-56:

CentOS 7 (with Percona Server 5.6 package)
------------------------------------------

You can use Percona Server packages version 5.6 on CentOS 7
since Mroonga 5.02 release.

Install::

  % sudo yum install -y https://packages.groonga.org/centos/groonga-release-1.3.0-1.noarch.rpm
  % sudo yum install -y http://www.percona.com/downloads/percona-release/redhat/0.1-4/percona-release-0.1-4.noarch.rpm
  % sudo yum install -y --enablerepo=epel percona-server-56-mroonga
  (% sudo systemctl start mysqld)
  (% sudo mysqladmin -u root password 'new-password')

If you want to use `MeCab <http://mecab.sourceforge.net/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo yum install -y --enablerepo=epel groonga-tokenizer-mecab

.. _centos-7-percona-57:

CentOS 7 (with Percona Server 5.7 package)
------------------------------------------

You can use Percona Server packages version 5.7 on CentOS 7
since Mroonga 6.02 release.

Install::

  % sudo yum install -y https://packages.groonga.org/centos/groonga-release-1.3.0-1.noarch.rpm
  % sudo yum install -y http://www.percona.com/downloads/percona-release/redhat/0.1-4/percona-release-0.1-4.noarch.rpm
  % sudo yum install -y --enablerepo=epel percona-server-57-mroonga
  (% sudo systemctl start mysqld)
  (% tmp_password=$(sudo grep 'A temporary password' /var/log/mysqld.log | sed -e 's/^.*: //'))
  (% sudo mysqladmin -u root --password="${tmp_password}" password)

If you want to use `MeCab <http://mecab.sourceforge.net/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo yum install -y --enablerepo=epel groonga-tokenizer-mecab

.. _centos-7-mariadb-10-1:

CentOS 7 (with MariaDB 10.1 package)
------------------------------------

You can use MariaDB's MariaDB packages version 10.1 on CentOS 7 since
Mroonga 7.06 release.

Create ``/etc/yum.repos.d/MariaDB.repo`` with the following content::

  [mariadb]
  name = MariaDB
  baseurl = http://yum.mariadb.org/10.1/centos7-amd64
  gpgkey=https://yum.mariadb.org/RPM-GPG-KEY-MariaDB
  gpgcheck=1

Install::

  % sudo yum install -y https://packages.groonga.org/centos/groonga-release-1.3.0-1.noarch.rpm
  % sudo yum install -y MariaDB-server
  % sudo systemctl start mariadb
  % sudo yum install -y --enablerepo=epel mariadb-10.1-mroonga
  (% sudo mysqladmin -u root password 'new-password')

If you want to use `MeCab <http://mecab.sourceforge.net/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo yum install -y --enablerepo=epel groonga-tokenizer-mecab

.. _centos-7-mariadb-10-2:

CentOS 7 (with MariaDB 10.2 package)
------------------------------------

You can use MariaDB's MariaDB packages version 10.2 on CentOS 7 since
Mroonga 7.06 release.

Create ``/etc/yum.repos.d/MariaDB.repo`` with the following content::

  [mariadb]
  name = MariaDB
  baseurl = http://yum.mariadb.org/10.2/centos7-amd64
  gpgkey=https://yum.mariadb.org/RPM-GPG-KEY-MariaDB
  gpgcheck=1

Install::

  % sudo yum install -y https://packages.groonga.org/centos/groonga-release-1.3.0-1.noarch.rpm
  % sudo yum install -y MariaDB-server
  % sudo systemctl start mariadb
  % sudo yum install -y --enablerepo=epel mariadb-10.2-mroonga
  (% sudo mysqladmin -u root password 'new-password')

If you want to use `MeCab <http://mecab.sourceforge.net/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo yum install -y --enablerepo=epel groonga-tokenizer-mecab

.. _centos-7-mariadb-10-3:

CentOS 7 (with MariaDB 10.3 package)
------------------------------------

You can use MariaDB's MariaDB packages version 10.3 on CentOS 7 since
Mroonga 8.00 release.

Create ``/etc/yum.repos.d/MariaDB.repo`` with the following content::

  [mariadb]
  name = MariaDB
  baseurl = http://yum.mariadb.org/10.3/centos7-amd64
  gpgkey=https://yum.mariadb.org/RPM-GPG-KEY-MariaDB
  gpgcheck=1

Install::

  % sudo yum install -y https://packages.groonga.org/centos/groonga-release-1.3.0-1.noarch.rpm
  % sudo yum install -y MariaDB-server
  % sudo systemctl start mariadb
  % sudo yum install -y --enablerepo=epel mariadb-10.3-mroonga
  (% sudo mysqladmin -u root password 'new-password')

If you want to use `MeCab <http://mecab.sourceforge.net/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo yum install -y --enablerepo=epel groonga-tokenizer-mecab
