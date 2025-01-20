AlmaLinux
=========

This section describes how to install Mroonga related RPM packages on
AlmaLinux. You can install them by ``dnf``.

.. _almalinux-8-oracle-8-0:

AlmaLinux 8 (with the Oracle MySQL 8.0 package)
-----------------------------------------------

You can use Oracle's MySQL packages version 8.0 on AlmaLinux 8 since
Mroonga 11.10 release.

.. note::

   There are already known issues about MySQL 8.0.

   * :doc:`/tutorial/wrapper` Wrapper mode is not supported yet
   * :doc:`/tutorial/storage`  Storage mode does not support the following feature.

     * The feature of relevant to the optimization.

Install::

  % sudo dnf -y module disable mysql
  % sudo dnf install -y https://packages.groonga.org/almalinux/8/groonga-release-latest.noarch.rpm
  % sudo dnf install -y https://repo.mysql.com/mysql80-community-release-el8.rpm
  % sudo dnf install --disablerepo=AppStream -y --enablerepo=epel,powertools mysql-community-8.0-mroonga
  (% sudo systemctl start mysqld)
  (% tmp_password=$(sudo grep 'A temporary password' /var/log/mysqld.log | sed -e 's/^.*: //'))
  (% sudo mysqladmin -u root --password="${tmp_password}" password)

If you want to use `MeCab <https://taku910.github.io/mecab/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo dnf -y module enable mysql
  % sudo dnf install -y --enablerepo=epel groonga-tokenizer-mecab
  % sudo dnf -y module disable mysql

.. _almalinux-8-percona-8-0:

AlmaLinux 8 (with Percona Server 8.0 package)
---------------------------------------------

You can use Percona Server packages version 8.0 on AlmaLinux 8
since Mroonga 11.10 release.

Install::

  % sudo dnf install -y https://packages.groonga.org/almalinux/8/groonga-release-latest.noarch.rpm
  % sudo dnf install -y https://repo.percona.com/yum/percona-release-latest.noarch.rpm
  % sudo percona-release setup ps80
  % sudo dnf install -y --enablerepo=epel,powertools percona-server-8.0-mroonga
  (% sudo systemctl start mysqld)
  (% tmp_password=$(sudo grep 'A temporary password' /var/log/mysqld.log | sed -e 's/^.*: //'))
  (% sudo mysqladmin -u root --password="${tmp_password}" password)

If you want to use `MeCab <https://taku910.github.io/mecab/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo dnf install -y --enablerepo=epel groonga-tokenizer-mecab

.. _almalinux-8-mariadb-10-5:

AlmaLinux 8 (with MariaDB 10.5 package)
---------------------------------------

You can use MariaDB's MariaDB packages version 10.5 on AlmaLinux 8 since
Mroonga 11.10 release.

Create ``/etc/yum.repos.d/MariaDB.repo`` with the following content::

  [mariadb]
  name = MariaDB
  baseurl = https://rpm.mariadb.org/10.5/rhel/$releasever/$basearch
  gpgkey = https://rpm.mariadb.org/RPM-GPG-KEY-MariaDB
  gpgcheck = 1

Install::

  % sudo dnf install -y https://packages.groonga.org/almalinux/8/groonga-release-latest.noarch.rpm
  % sudo dnf module -y disable mariadb
  % sudo dnf module -y disable mysql
  % sudo dnf install -y --enablerepo=powertools mariadb-server
  % sudo systemctl start mariadb
  % sudo dnf install -y --enablerepo=powertools mariadb-10.5-mroonga
  (% sudo mysqladmin -u root password 'new-password')

If you want to use `MeCab <https://taku910.github.io/mecab/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo dnf module -y enable mysql
  % sudo dnf install -y --enablerepo=epel groonga-tokenizer-mecab
  % sudo dnf module -y disable mysql

.. _almalinux-8-mariadb-10-6:

AlmaLinux 8 (with MariaDB 10.6 package)
---------------------------------------

You can use MariaDB's MariaDB packages version 10.6 on AlmaLinux 8 since
Mroonga 11.10 release.

Create ``/etc/yum.repos.d/MariaDB.repo`` with the following content::

  [mariadb]
  name = MariaDB
  baseurl = https://rpm.mariadb.org/10.6/rhel/$releasever/$basearch
  gpgkey = https://rpm.mariadb.org/RPM-GPG-KEY-MariaDB
  gpgcheck = 1

Install::

  % sudo dnf install -y https://packages.groonga.org/almalinux/8/groonga-release-latest.noarch.rpm
  % sudo dnf module -y disable mariadb
  % sudo dnf module -y disable mysql
  % sudo dnf install -y --enablerepo=powertools mariadb-server
  % sudo systemctl start mariadb
  % sudo dnf install -y --enablerepo=powertools mariadb-10.6-mroonga
  (% sudo mysqladmin -u root password 'new-password')

If you want to use `MeCab <https://taku910.github.io/mecab/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo dnf module -y enable mysql
  % sudo dnf install -y --enablerepo=epel groonga-tokenizer-mecab
  % sudo dnf module -y disable mysql

.. _almalinux-8-mariadb-10-11:

AlmaLinux 8 (with MariaDB 10.11 package)
----------------------------------------

You can use MariaDB's MariaDB packages version 10.11 on AlmaLinux 8 since
Mroonga 13.01 release.

Create ``/etc/yum.repos.d/MariaDB.repo`` with the following content::

  [mariadb]
  name = MariaDB
  baseurl = https://rpm.mariadb.org/10.11/rhel/$releasever/$basearch
  gpgkey = https://rpm.mariadb.org/RPM-GPG-KEY-MariaDB
  gpgcheck = 1

Install::

  % sudo dnf install -y https://packages.groonga.org/almalinux/8/groonga-release-latest.noarch.rpm
  % sudo dnf module -y disable mariadb
  % sudo dnf module -y disable mysql
  % sudo dnf install -y --enablerepo=powertools mariadb-server
  % sudo systemctl start mariadb
  % sudo dnf install -y --enablerepo=powertools mariadb-10.11-mroonga
  (% sudo mysqladmin -u root password 'new-password')

If you want to use `MeCab <https://taku910.github.io/mecab/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package:

.. code-block:: console

   $ sudo dnf module -y enable mysql
   $ sudo dnf install -y --enablerepo=epel groonga-tokenizer-mecab
   $ sudo dnf module -y disable mysql

.. _almalinux-8-mariadb-11-4:

AlmaLinux 8 (with MariaDB 11.4 package)
---------------------------------------

You can use MariaDB's MariaDB packages version 11.4 on AlmaLinux 8 since
Mroonga 14.07 release.

Create ``/etc/yum.repos.d/MariaDB.repo`` with the following content::

  [mariadb]
  name = MariaDB
  baseurl = https://rpm.mariadb.org/11.4/rhel/$releasever/$basearch
  gpgkey = https://rpm.mariadb.org/RPM-GPG-KEY-MariaDB
  gpgcheck = 1

Install::

  % sudo dnf install -y https://packages.groonga.org/almalinux/8/groonga-release-latest.noarch.rpm
  % sudo dnf module -y disable mariadb
  % sudo dnf module -y disable mysql
  % sudo dnf install -y --enablerepo=powertools mariadb-server
  % sudo systemctl start mariadb
  % sudo dnf install -y --enablerepo=powertools mariadb-11.4-mroonga
  (% sudo mariadb-admin -u root password 'new-password')

If you want to use `MeCab <https://taku910.github.io/mecab/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo dnf module -y enable mysql
  % sudo dnf install -y --enablerepo=epel groonga-tokenizer-mecab
  % sudo dnf module -y disable mysql

.. _almalinux-9-oracle-8-0:

AlmaLinux 9 (with the Oracle MySQL 8.0 package)
-----------------------------------------------

You can use Oracle's MySQL packages version 8.0 on AlmaLinux 9 since
Mroonga 12.12 release.

.. note::

   There are already known issues about MySQL 8.0.

   * :doc:`/tutorial/wrapper` Wrapper mode is not supported yet
   * :doc:`/tutorial/storage`  Storage mode does not support the following feature.

     * The feature of relevant to the optimization.

Install::

  % sudo dnf install -y https://apache.jfrog.io/artifactory/arrow/almalinux/9/apache-arrow-release-latest.rpm
  % sudo dnf install -y https://packages.groonga.org/almalinux/9/groonga-release-latest.noarch.rpm
  % sudo dnf install -y https://repo.mysql.com/mysql80-community-release-el9.rpm
  % sudo dnf install --disablerepo=AppStream -y --enablerepo=epel,crb mysql-community-8.0-mroonga
  (% sudo systemctl start mysqld)
  (% tmp_password=$(sudo grep 'A temporary password' /var/log/mysqld.log | sed -e 's/^.*: //'))
  (% sudo mysqladmin -u root --password="${tmp_password}" password)

If you want to use `MeCab <https://taku910.github.io/mecab/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo dnf install -y --enablerepo=epel groonga-tokenizer-mecab

.. _almalinux-9-percona-8-0:

AlmaLinux 9 (with Percona Server 8.0 package)
---------------------------------------------

You can use Percona Server packages version 8.0 on AlmaLinux 9
since Mroonga 12.12 release.

Install::

  % sudo dnf install -y https://apache.jfrog.io/artifactory/arrow/almalinux/9/apache-arrow-release-latest.rpm
  % sudo dnf install -y https://packages.groonga.org/almalinux/9/groonga-release-latest.noarch.rpm
  % sudo dnf install -y https://repo.percona.com/yum/percona-release-latest.noarch.rpm
  % sudo percona-release setup ps80
  % sudo dnf install -y --enablerepo=epel percona-server-8.0-mroonga
  (% sudo systemctl start mysqld)
  (% tmp_password=$(sudo grep 'A temporary password' /var/log/mysqld.log | sed -e 's/^.*: //'))
  (% sudo mysqladmin -u root --password="${tmp_password}" password)

If you want to use `MeCab <https://taku910.github.io/mecab/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo dnf install -y --enablerepo=epel groonga-tokenizer-mecab

.. _almalinux-9-mariadb-10-5:

AlmaLinux 9 (with MariaDB 10.5 package)
---------------------------------------

You can use MariaDB's MariaDB packages version 10.5 on AlmaLinux 9 since
Mroonga 12.12 release.

Create ``/etc/yum.repos.d/MariaDB.repo`` with the following content::

  [mariadb]
  name = MariaDB
  baseurl = https://rpm.mariadb.org/10.5/rhel/$releasever/$basearch
  gpgkey = https://rpm.mariadb.org/RPM-GPG-KEY-MariaDB
  gpgcheck = 1

Install::

  % sudo dnf install -y https://apache.jfrog.io/artifactory/arrow/almalinux/9/apache-arrow-release-latest.rpm
  % sudo dnf install -y https://packages.groonga.org/almalinux/9/groonga-release-latest.noarch.rpm
  % sudo dnf install -y mariadb-server
  % sudo systemctl start mariadb
  % sudo dnf install -y mariadb-10.5-mroonga
  (% sudo mysqladmin -u root password 'new-password')

If you want to use `MeCab <https://taku910.github.io/mecab/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo dnf install -y --enablerepo=epel groonga-tokenizer-mecab

.. _almalinux-9-mariadb-10-6:

AlmaLinux 9 (with MariaDB 10.6 package)
---------------------------------------

You can use MariaDB's MariaDB packages version 10.6 on AlmaLinux 9 since
Mroonga 12.12 release.

Create ``/etc/yum.repos.d/MariaDB.repo`` with the following content::

  [mariadb]
  name = MariaDB
  baseurl = https://rpm.mariadb.org/10.6/rhel/$releasever/$basearch
  gpgkey = https://rpm.mariadb.org/RPM-GPG-KEY-MariaDB
  gpgcheck = 1

Install::

  % sudo dnf install -y https://apache.jfrog.io/artifactory/arrow/almalinux/9/apache-arrow-release-latest.rpm
  % sudo dnf install -y https://packages.groonga.org/almalinux/9/groonga-release-latest.noarch.rpm
  % sudo dnf install -y mariadb-server
  % sudo systemctl start mariadb
  % sudo dnf install -y mariadb-10.6-mroonga
  (% sudo mysqladmin -u root password 'new-password')

If you want to use `MeCab <https://taku910.github.io/mecab/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo dnf install -y --enablerepo=epel groonga-tokenizer-mecab

.. _almalinux-9-mariadb-10-11:

AlmaLinux 9 (with MariaDB 10.11 package)
----------------------------------------

You can use MariaDB's MariaDB packages version 10.11 on AlmaLinux 9 since
Mroonga 13.01 release.

Create ``/etc/yum.repos.d/MariaDB.repo`` with the following content::

  [mariadb]
  name = MariaDB
  baseurl = https://rpm.mariadb.org/10.11/rhel/$releasever/$basearch
  gpgkey = https://rpm.mariadb.org/RPM-GPG-KEY-MariaDB
  gpgcheck = 1

Install::

  % sudo dnf install -y https://apache.jfrog.io/artifactory/arrow/almalinux/9/apache-arrow-release-latest.rpm
  % sudo dnf install -y https://packages.groonga.org/almalinux/9/groonga-release-latest.noarch.rpm
  % sudo dnf install -y mariadb-server
  % sudo systemctl start mariadb
  % sudo dnf install -y mariadb-10.11-mroonga
  (% sudo mysqladmin -u root password 'new-password')

If you want to use `MeCab <https://taku910.github.io/mecab/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo dnf install -y --enablerepo=epel groonga-tokenizer-mecab

.. _almalinux-9-mariadb-11-4:

AlmaLinux 9 (with MariaDB 11.4 package)
---------------------------------------

You can use MariaDB's MariaDB packages version 11.4 on AlmaLinux 9 since
Mroonga 14.07 release.

Create ``/etc/yum.repos.d/MariaDB.repo`` with the following content::

  [mariadb]
  name = MariaDB
  baseurl = https://rpm.mariadb.org/11.4/rhel/$releasever/$basearch
  gpgkey = https://rpm.mariadb.org/RPM-GPG-KEY-MariaDB
  gpgcheck = 1

Install::

  % sudo dnf install -y https://apache.jfrog.io/artifactory/arrow/almalinux/9/apache-arrow-release-latest.rpm
  % sudo dnf install -y https://packages.groonga.org/almalinux/9/groonga-release-latest.noarch.rpm
  % sudo dnf install -y mariadb-server
  % sudo systemctl start mariadb
  % sudo dnf install -y mariadb-11.4-mroonga
  (% sudo mariadb-admin -u root password 'new-password')

If you want to use `MeCab <https://taku910.github.io/mecab/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo dnf install -y --enablerepo=epel groonga-tokenizer-mecab
