.. highlightlang:: none

CentOS
======

This section describes how to install Mroonga related RPM packages on
CentOS. You can install them by ``yum``.

.. include:: 32bit-note.inc

.. _centos-7-official:

CentOS 7 (with the official MariaDB package)
--------------------------------------------

You can use CentOS's MariaDB packages (version 5.5.x) on CentOS 7.

Install::

  % sudo yum install -y https://packages.groonga.org/centos/groonga-release-latest.noarch.rpm
  % sudo yum install -y mariadb-server
  % sudo systemctl start mariadb
  % sudo yum install -y --enablerepo=epel mariadb-mroonga
  (% sudo mysqladmin -u root password 'new-password')

If you want to use `MeCab <https://taku910.github.io/mecab/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo yum install -y --enablerepo=epel groonga-tokenizer-mecab

.. _centos-7-oracle-57:

CentOS 7 (with the Oracle MySQL 5.7 package)
--------------------------------------------

You can use Oracle's MySQL packages version 5.7 on CentOS 7 since
Mroonga 5.09 release.

Install::

  % sudo yum install -y https://packages.groonga.org/centos/groonga-release-latest.noarch.rpm
  % sudo yum install -y http://repo.mysql.com/mysql-community-release-el7-7.noarch.rpm
  % sudo yum install -y yum-utils
  % sudo yum-config-manager --disable mysql56-community
  % sudo yum-config-manager --enable mysql57-community
  % sudo yum install -y --enablerepo=epel mysql57-community-mroonga
  (% sudo systemctl start mysqld)
  (% tmp_password=$(sudo grep 'A temporary password' /var/log/mysqld.log | sed -e 's/^.*: //'))
  (% sudo mysqladmin -u root --password="${tmp_password}" password)

If you want to use `MeCab <https://taku910.github.io/mecab/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo yum install -y --enablerepo=epel groonga-tokenizer-mecab

.. _centos-7-oracle-80:

CentOS 7 (with the Oracle MySQL 8.0 package)
--------------------------------------------

You can use Oracle's MySQL packages version 8.0 on CentOS 7 since
Mroonga 9.04 release.

.. note::

   There are already known issues about MySQL 8.0.

   * :doc:`/tutorial/wrapper` Wrapper mode is not supported yet
   * :doc:`/tutorial/storage` `JSON` data type is not supported yet

Install::

  % sudo yum install -y https://packages.groonga.org/centos/groonga-release-latest.noarch.rpm
  % sudo yum install -y http://repo.mysql.com/mysql80-community-release-el7.rpm
  % sudo yum install -y --enablerepo=epel mysql80-community-mroonga
  (% sudo systemctl start mysqld)
  (% tmp_password=$(sudo grep 'A temporary password' /var/log/mysqld.log | sed -e 's/^.*: //'))
  (% sudo mysqladmin -u root --password="${tmp_password}" password)

If you want to use `MeCab <https://taku910.github.io/mecab/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo yum install -y --enablerepo=epel groonga-tokenizer-mecab

.. _centos-7-percona-56:

CentOS 7 (with Percona Server 5.6 package)
------------------------------------------

You can use Percona Server packages version 5.6 on CentOS 7
since Mroonga 5.02 release.

Install::

  % sudo yum install -y https://packages.groonga.org/centos/groonga-release-latest.noarch.rpm
  % sudo yum install -y http://repo.percona.com/release/percona-release-latest.noarch.rpm
  % sudo yum install -y --enablerepo=epel percona-server-56-mroonga
  (% sudo systemctl start mysqld)
  (% sudo mysqladmin -u root password 'new-password')

If you want to use `MeCab <https://taku910.github.io/mecab/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo yum install -y --enablerepo=epel groonga-tokenizer-mecab

.. _centos-7-percona-57:

CentOS 7 (with Percona Server 5.7 package)
------------------------------------------

You can use Percona Server packages version 5.7 on CentOS 7
since Mroonga 6.02 release.

Install::

  % sudo yum install -y https://packages.groonga.org/centos/groonga-release-latest.noarch.rpm
  % sudo yum install -y http://repo.percona.com/release/percona-release-latest.noarch.rpm
  % sudo yum install -y --enablerepo=epel percona-server-57-mroonga
  (% sudo systemctl start mysqld)
  (% tmp_password=$(sudo grep 'A temporary password' /var/log/mysqld.log | sed -e 's/^.*: //'))
  (% sudo mysqladmin -u root --password="${tmp_password}" password)

If you want to use `MeCab <https://taku910.github.io/mecab/>`_ as a
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

  % sudo yum install -y https://packages.groonga.org/centos/groonga-release-latest.noarch.rpm
  % sudo yum install -y MariaDB-server
  % sudo systemctl start mariadb
  % sudo yum install -y --enablerepo=epel mariadb-10.1-mroonga
  (% sudo mysqladmin -u root password 'new-password')

If you want to use `MeCab <https://taku910.github.io/mecab/>`_ as a
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

  % sudo yum install -y https://packages.groonga.org/centos/groonga-release-latest.noarch.rpm
  % sudo yum install -y MariaDB-server
  % sudo systemctl start mariadb
  % sudo yum install -y --enablerepo=epel mariadb-10.2-mroonga
  (% sudo mysqladmin -u root password 'new-password')

If you want to use `MeCab <https://taku910.github.io/mecab/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo yum install -y --enablerepo=epel groonga-tokenizer-mecab

.. _centos-7-mariadb-10-3:

CentOS 7 (with MariaDB 10.3 package)
------------------------------------

You can use MariaDB's MariaDB packages version 10.3 on CentOS 7 since
Mroonga 7.11 release.

Create ``/etc/yum.repos.d/MariaDB.repo`` with the following content::

  [mariadb]
  name = MariaDB
  baseurl = http://yum.mariadb.org/10.3/centos7-amd64
  gpgkey=https://yum.mariadb.org/RPM-GPG-KEY-MariaDB
  gpgcheck=1

Install::

  % sudo yum install -y https://packages.groonga.org/centos/groonga-release-latest.noarch.rpm
  % sudo yum install -y MariaDB-server
  % sudo systemctl start mariadb
  % sudo yum install -y --enablerepo=epel mariadb-10.3-mroonga
  (% sudo mysqladmin -u root password 'new-password')

If you want to use `MeCab <https://taku910.github.io/mecab/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo yum install -y --enablerepo=epel groonga-tokenizer-mecab

.. _centos-7-mariadb-10-4:

CentOS 7 (with MariaDB 10.4 package)
------------------------------------

You can use MariaDB's MariaDB packages version 10.4 on CentOS 7 since
Mroonga 9.07 release.

Create ``/etc/yum.repos.d/MariaDB.repo`` with the following content::

  [mariadb]
  name = MariaDB
  baseurl = http://yum.mariadb.org/10.4/centos7-amd64
  gpgkey=https://yum.mariadb.org/RPM-GPG-KEY-MariaDB
  gpgcheck=1

Install::

  % sudo yum install -y https://packages.groonga.org/centos/groonga-release-latest.noarch.rpm
  % sudo yum install -y MariaDB-server
  % sudo systemctl start mariadb
  % sudo yum install -y --enablerepo=epel mariadb-10.4-mroonga
  (% sudo mysqladmin -u root password 'new-password')

If you want to use `MeCab <https://taku910.github.io/mecab/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo yum install -y --enablerepo=epel groonga-tokenizer-mecab

.. _centos-8-oracle-80:

CentOS 8 (with the Oracle MySQL 8.0 package)
--------------------------------------------

You can use Oracle's MySQL packages version 8.0 on CentOS 8 since
Mroonga 9.10 release.

.. note::

   There are already known issues about MySQL 8.0.

   * :doc:`/tutorial/wrapper` Wrapper mode is not supported yet
   * :doc:`/tutorial/storage` `JSON` data type is not supported yet

Install::

  % sudo dnf install -y https://packages.groonga.org/centos/groonga-release-latest.noarch.rpm
  % sudo dnf install -y http://repo.mysql.com/mysql80-community-release-el8.rpm
  % sudo dnf install -y groonga-libs
  % sudo dnf install --disablerepo=AppStream -y --enablerepo=epel mysql80-community-mroonga
  (% sudo systemctl start mysqld)
  (% tmp_password=$(sudo grep 'A temporary password' /var/log/mysqld.log | sed -e 's/^.*: //'))
  (% sudo mysqladmin -u root --password="${tmp_password}" password)

.. _centos-8-mariadb-10-3:

CentOS 8 (with MariaDB 10.3 package)
------------------------------------

You can use MariaDB's MariaDB packages version 10.3 on CentOS 8 since
Mroonga 9.10 release.

Create ``/etc/yum.repos.d/MariaDB.repo`` with the following content::

  [mariadb]
  name = MariaDB
  baseurl = http://yum.mariadb.org/10.3/centos8-amd64
  gpgkey=https://yum.mariadb.org/RPM-GPG-KEY-MariaDB
  gpgcheck=1

Install::

  % sudo dnf install -y https://packages.groonga.org/centos/groonga-release-latest.noarch.rpm
  % sudo dnf install -y boost-program-options
  % sudo dnf install --disablerepo=AppStream -y MariaDB-server
  % sudo systemctl start mariadb
  % sudo dnf install -y --enablerepo=epel mariadb-10.3-mroonga
  (% sudo mysqladmin -u root password 'new-password')

.. _centos-8-mariadb-10-4:

CentOS 8 (with MariaDB 10.4 package)
------------------------------------

You can use MariaDB's MariaDB packages version 10.4 on CentOS 8 since
Mroonga 9.10 release.

Create ``/etc/yum.repos.d/MariaDB.repo`` with the following content::

  [mariadb]
  name = MariaDB
  baseurl = http://yum.mariadb.org/10.4/centos8-amd64
  gpgkey=https://yum.mariadb.org/RPM-GPG-KEY-MariaDB
  gpgcheck=1

Install::

  % sudo dnf install -y https://packages.groonga.org/centos/groonga-release-latest.noarch.rpm
  % sudo dnf install -y boost-program-options
  % sudo dnf install --disablerepo=AppStream -y MariaDB-server
  % sudo systemctl start mariadb
  % sudo dnf install -y --enablerepo=epel mariadb-10.4-mroonga
  (% sudo mysqladmin -u root password 'new-password')
