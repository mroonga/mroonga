CentOS
======

This section describes how to install Mroonga related RPM packages on
CentOS. You can install them by ``yum``.

.. include:: 32bit-note.inc

.. _centos-7-oracle-5-7:

CentOS 7 (with the Oracle MySQL 5.7 package)
--------------------------------------------

You can use Oracle's MySQL packages version 5.7 on CentOS 7 since
Mroonga 5.09 release.

Install::

  % sudo yum install -y https://packages.groonga.org/centos/groonga-release-latest.noarch.rpm
  % sudo yum install -y https://repo.mysql.com/mysql-community-release-el7.rpm
  % sudo yum install -y yum-utils
  % sudo yum-config-manager --disable mysql80-community
  % sudo yum-config-manager --enable mysql57-community
  % sudo yum install -y --enablerepo=epel mysql-community-5.7-mroonga
  (% sudo systemctl start mysqld)
  (% tmp_password=$(sudo grep 'A temporary password' /var/log/mysqld.log | sed -e 's/^.*: //'))
  (% sudo mysqladmin -u root --password="${tmp_password}" password)

If you want to use `MeCab <https://taku910.github.io/mecab/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo yum install -y --enablerepo=epel groonga-tokenizer-mecab

.. _centos-7-oracle-8-0:

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
  % sudo yum install -y https://repo.mysql.com/mysql80-community-release-el7.rpm
  % sudo yum install -y --enablerepo=epel mysql-community-8.0-mroonga
  (% sudo systemctl start mysqld)
  (% tmp_password=$(sudo grep 'A temporary password' /var/log/mysqld.log | sed -e 's/^.*: //'))
  (% sudo mysqladmin -u root --password="${tmp_password}" password)

If you want to use `MeCab <https://taku910.github.io/mecab/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo yum install -y --enablerepo=epel groonga-tokenizer-mecab

.. _centos-7-percona-5-7:

CentOS 7 (with Percona Server 5.7 package)
------------------------------------------

You can use Percona Server packages version 5.7 on CentOS 7
since Mroonga 6.02 release.

Install::

  % sudo yum install -y https://packages.groonga.org/centos/groonga-release-latest.noarch.rpm
  % sudo yum install -y https://repo.percona.com/yum/percona-release-latest.noarch.rpm
  % sudo yum install -y --enablerepo=epel percona-server-5.7-mroonga
  (% sudo systemctl start mysqld)
  (% tmp_password=$(sudo grep 'A temporary password' /var/log/mysqld.log | sed -e 's/^.*: //'))
  (% sudo mysqladmin -u root --password="${tmp_password}" password)

If you want to use `MeCab <https://taku910.github.io/mecab/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo yum install -y --enablerepo=epel groonga-tokenizer-mecab

.. _centos-7-percona-8-0:

CentOS 7 (with Percona Server 8.0 package)
------------------------------------------

You can use Percona Server packages version 8.0 on CentOS 7
since Mroonga 10.06 release.

Install::

  % sudo yum install -y https://packages.groonga.org/centos/groonga-release-latest.noarch.rpm
  % sudo yum install -y https://repo.percona.com/yum/percona-release-latest.noarch.rpm
  % sudo yum install -y --enablerepo=epel percona-server-8.0-mroonga
  (% sudo systemctl start mysqld)
  (% tmp_password=$(sudo grep 'A temporary password' /var/log/mysqld.log | sed -e 's/^.*: //'))
  (% sudo mysqladmin -u root --password="${tmp_password}" password)

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

.. _centos-7-mariadb-10-5:

CentOS 7 (with MariaDB 10.5 package)
------------------------------------

You can use MariaDB's MariaDB packages version 10.5 on CentOS 7 since
Mroonga 10.06 release.

Create ``/etc/yum.repos.d/MariaDB.repo`` with the following content::

  [mariadb]
  name = MariaDB
  baseurl = http://yum.mariadb.org/10.5/centos7-amd64
  gpgkey=https://yum.mariadb.org/RPM-GPG-KEY-MariaDB
  gpgcheck=1

Install::

  % sudo yum install -y https://packages.groonga.org/centos/groonga-release-latest.noarch.rpm
  % sudo yum install -y MariaDB-server
  % sudo systemctl start mariadb
  % sudo yum install -y --enablerepo=epel mariadb-10.5-mroonga
  (% sudo mysqladmin -u root password 'new-password')

If you want to use `MeCab <https://taku910.github.io/mecab/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo yum install -y --enablerepo=epel groonga-tokenizer-mecab

.. _centos-7-mariadb-10-6:

CentOS 7 (with MariaDB 10.6 package)
------------------------------------

You can use MariaDB's MariaDB packages version 10.6 on CentOS 7 since
Mroonga 11.09 release.

Create ``/etc/yum.repos.d/MariaDB.repo`` with the following content::

  [mariadb]
  name = MariaDB
  baseurl = https://yum.mariadb.org/10.6/centos7-amd64
  gpgkey=https://yum.mariadb.org/RPM-GPG-KEY-MariaDB
  gpgcheck=1

Install::

  % sudo yum install -y https://packages.groonga.org/centos/groonga-release-latest.noarch.rpm
  % sudo yum install -y MariaDB-server
  % sudo systemctl start mariadb
  % sudo yum install -y --enablerepo=epel mariadb-10.6-mroonga
  (% sudo mysqladmin -u root password 'new-password')

If you want to use `MeCab <https://taku910.github.io/mecab/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo yum install -y --enablerepo=epel groonga-tokenizer-mecab
