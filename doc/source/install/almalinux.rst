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

  % sudo dnf install -y --enablerepo=epel groonga-tokenizer-mecab

.. _almalinux-8-percona-8-0:

AlmaLinux 8 (with Percona Server 8.0 package)
---------------------------------------------

You can use Percona Server packages version 8.0 on AlmaLinux 8
since Mroonga 11.10 release.

Install::

  % sudo dnf install -y https://packages.groonga.org/almalinux/8/groonga-release-latest.noarch.rpm
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

.. _almalinux-8-mariadb-10-3:

AlmaLinux 8 (with MariaDB 10.3 package)
---------------------------------------

You can use MariaDB's MariaDB packages version 10.3 on AlmaLinux 8 since
Mroonga 11.10 release.

Create ``/etc/yum.repos.d/MariaDB.repo`` with the following content::

  [mariadb]
  name = MariaDB
  baseurl = http://yum.mariadb.org/10.3/centos8-amd64
  gpgkey=https://yum.mariadb.org/RPM-GPG-KEY-MariaDB
  gpgcheck=1

Install::

  % sudo dnf install -y https://packages.groonga.org/almalinux/8/groonga-release-latest.noarch.rpm
  % sudo dnf module -y disable mariadb
  % sudo dnf module -y disable mysql
  % sudo dnf install -y --enablerepo=powertools mariadb-server
  % sudo systemctl start mariadb
  % sudo dnf install -y --enablerepo=powertools mariadb-10.3-mroonga
  (% sudo mysqladmin -u root password 'new-password')

If you want to use `MeCab <https://taku910.github.io/mecab/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo dnf install -y --enablerepo=epel groonga-tokenizer-mecab

.. _almalinux-8-mariadb-10-4:

AlmaLinux 8 (with MariaDB 10.4 package)
---------------------------------------

You can use MariaDB's MariaDB packages version 10.4 on AlmaLinux 8 since
Mroonga 11.10 release.

Create ``/etc/yum.repos.d/MariaDB.repo`` with the following content::

  [mariadb]
  name = MariaDB
  baseurl = http://yum.mariadb.org/10.4/centos8-amd64
  gpgkey=https://yum.mariadb.org/RPM-GPG-KEY-MariaDB
  gpgcheck=1

Install::

  % sudo dnf install -y https://packages.groonga.org/almalinux/8/groonga-release-latest.noarch.rpm
  % sudo dnf module -y disable mariadb
  % sudo dnf module -y disable mysql
  % sudo dnf install -y --enablerepo=powertools mariadb-server
  % sudo systemctl start mariadb
  % sudo dnf install -y --enablerepo=powertools mariadb-10.4-mroonga
  (% sudo mysqladmin -u root password 'new-password')

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
  baseurl = http://yum.mariadb.org/10.5/centos8-amd64
  gpgkey=https://yum.mariadb.org/RPM-GPG-KEY-MariaDB
  gpgcheck=1

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

  % sudo dnf install -y --enablerepo=epel groonga-tokenizer-mecab

.. _almalinux-8-mariadb-10-6:

AlmaLinux 8 (with MariaDB 10.6 package)
---------------------------------------

You can use MariaDB's MariaDB packages version 10.6 on AlmaLinux 8 since
Mroonga 11.10 release.

Create ``/etc/yum.repos.d/MariaDB.repo`` with the following content::

  [mariadb]
  name = MariaDB
  baseurl = https://yum.mariadb.org/10.6/rhel8-amd64
  gpgkey=https://yum.mariadb.org/RPM-GPG-KEY-MariaDB
  gpgcheck=1

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

  % sudo dnf install -y --enablerepo=epel groonga-tokenizer-mecab

.. _almalinux-8-mariadb-10-7:

AlmaLinux 8 (with MariaDB 10.7 package)
---------------------------------------

You can use MariaDB's MariaDB packages version 10.7 on AlmaLinux 8 since
Mroonga 12.02 release.

Create ``/etc/yum.repos.d/MariaDB.repo`` with the following content::

  [mariadb]
  name = MariaDB
  baseurl = http://yum.mariadb.org/10.7/rhel8-amd64
  gpgkey=https://yum.mariadb.org/RPM-GPG-KEY-MariaDB
  gpgcheck=1

Install::

  % sudo dnf install -y https://packages.groonga.org/almalinux/8/groonga-release-latest.noarch.rpm
  % sudo dnf module -y disable mariadb
  % sudo dnf module -y disable mysql
  % sudo dnf install -y --enablerepo=powertools mariadb-server
  % sudo systemctl start mariadb
  % sudo dnf install -y --enablerepo=powertools mariadb-10.7-mroonga
  (% sudo mysqladmin -u root password 'new-password')

If you want to use `MeCab <https://taku910.github.io/mecab/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo dnf install -y --enablerepo=epel groonga-tokenizer-mecab

.. _almalinux-8-mariadb-10-8:

AlmaLinux 8 (with MariaDB 10.8 package)
---------------------------------------

You can use MariaDB's MariaDB packages version 10.8 on AlmaLinux 8 since
Mroonga 12.06 release.

Create ``/etc/yum.repos.d/MariaDB.repo`` with the following content::

  [mariadb]
  name = MariaDB
  baseurl = http://yum.mariadb.org/10.8/rhel8-amd64
  gpgkey=https://yum.mariadb.org/RPM-GPG-KEY-MariaDB
  gpgcheck=1

Install::

  % sudo dnf install -y https://packages.groonga.org/almalinux/8/groonga-release-latest.noarch.rpm
  % sudo dnf module -y disable mariadb
  % sudo dnf module -y disable mysql
  % sudo dnf install -y --enablerepo=powertools mariadb-server
  % sudo systemctl start mariadb
  % sudo dnf install -y --enablerepo=powertools mariadb-10.8-mroonga
  (% sudo mysqladmin -u root password 'new-password')

If you want to use `MeCab <https://taku910.github.io/mecab/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo dnf install -y --enablerepo=epel groonga-tokenizer-mecab
