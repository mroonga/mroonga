.. highlightlang:: none
.. Mroonga Project

Installation Guide
==================

If you use binary packages, MySQL related packages will be also installed when you install Mroonga package.

We distribute packages for both 32-bit and 64-bit but we recommend that you should use 64-bit package for server. You should use 32-bit package just only for test or development. You will get no memory error with 32-bit package even if you just process medium size data.

Here we explain how to install for each environment.

Debian GNU/Linux wheezy
-----------------------

/etc/apt/sources.list.d/groonga.list::

  deb http://packages.groonga.org/debian/ wheezy main
  deb-src http://packages.groonga.org/debian/ wheezy main

Install::

  % sudo apt-get update
  % sudo apt-get install -y --allow-unauthenticated groonga-keyring
  % sudo apt-get update
  % sudo apt-get install -y -V groonga

If you want to use `MeCab <http://mecab.sourceforge.net/>`_ as a tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo apt-get install -y -V groonga-tokenizer-mecab

If you want to use MySQL compatible COLLATION such as 'utf8_general_ci', 'utf8mb4_general_ci', 'utf8_unicode_ci' or 'utf8mb4_unicode_ci', please install groonga-normalizer-mysql package.

Install groonga-normalizer-mysql package::

  % sudo apt-get install -y -V groonga-normalizer-mysql

.. note::

   If you don't specify MySQL compatible case insensitive COLLATION, MySQL incompatible COLLATION of Groonga is used.
   MySQL compatible case insensitive COLLATION are 'utf8_general_ci', 'utf8mb4_general_ci', 'utf8_unicode_ci' and 'utf8mb4_unicode_ci'.

Debian GNU/Linux jessie
-----------------------

/etc/apt/sources.list.d/groonga.list::

  deb http://packages.groonga.org/debian/ jessie main
  deb-src http://packages.groonga.org/debian/ jessie main

Install::

  % sudo apt-get update
  % sudo apt-get install -y --allow-unauthenticated groonga-keyring
  % sudo apt-get update
  % sudo apt-get install -y -V groonga

If you want to use `MeCab <http://mecab.sourceforge.net/>`_ as a tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo apt-get install -y -V groonga-tokenizer-mecab

If you want to use MySQL compatible COLLATION such as 'utf8_general_ci', 'utf8mb4_general_ci', 'utf8_unicode_ci' or 'utf8mb4_unicode_ci', please install groonga-normalizer-mysql package.

Install groonga-normalizer-mysql package::

  % sudo apt-get install -y -V groonga-normalizer-mysql

.. note::

   If you don't specify MySQL compatible case insensitive COLLATION, MySQL incompatible COLLATION of Groonga is used.
   MySQL compatible case insensitive COLLATION are 'utf8_general_ci', 'utf8mb4_general_ci', 'utf8_unicode_ci' and 'utf8mb4_unicode_ci'.

Debian GNU/Linux sid
--------------------

/etc/apt/sources.list.d/groonga.list::

  deb http://packages.groonga.org/debian/ unstable main
  deb-src http://packages.groonga.org/debian/ unstable main

Install::

  % sudo apt-get update
  % sudo apt-get install -y --allow-unauthenticated groonga-keyring
  % sudo apt-get update
  % sudo apt-get install -y -V groonga

If you want to use `MeCab <http://mecab.sourceforge.net/>`_ as a tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo apt-get install -y -V groonga-tokenizer-mecab

If you want to use MySQL compatible COLLATION such as 'utf8_general_ci', 'utf8mb4_general_ci', 'utf8_unicode_ci' or 'utf8mb4_unicode_ci', please install groonga-normalizer-mysql package.

Install groonga-normalizer-mysql package::

  % sudo apt-get install -y -V groonga-normalizer-mysql

.. note::

   'utf8_unicode_ci' or 'utf8mb4_unicode_ci' will be supported in the future release.

.. note::

   If you don't specify MySQL compatible case insensitive COLLATION, MySQL incompatible COLLATION of Groonga is used.
   MySQL compatible case insensitive COLLATION are 'utf8_general_ci', 'utf8mb4_general_ci', 'utf8_unicode_ci' and 'utf8mb4_unicode_ci'.

Ubuntu
------

The Mroonga APT repository for Ubuntu uses PPA (Personal Package
Archive) on Launchpad. You can install Mroonga by APT from the PPA.

Here are supported Ubuntu versions:

  * 12.04 LTS Precise Pangolin
  * 13.10 Saucy Salamander
  * 14.04 LTS Trusty Tahr

Enable the universe repository and the security update repository to
install Mroonga::

  % sudo apt-get install -y -V software-properties-common lsb-release
  % sudo add-apt-repository -y universe
  % sudo add-apt-repository "deb http://security.ubuntu.com/ubuntu $(lsb_release --short --codename)-security main restricted"

Add the ``ppa:groonga/ppa`` PPA to your system::

  % sudo add-apt-repository -y ppa:groonga/ppa
  % sudo apt-get update

Install::

  % sudo apt-get install -y -V mysql-server-mroonga

If you want to use `MeCab <http://mecab.sourceforge.net/>`_ as a tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo apt-get install -y -V groonga-tokenizer-mecab

If you want to use MySQL compatible COLLATION such as 'utf8_general_ci', 'utf8mb4_general_ci', 'utf8_unicode_ci' or 'utf8mb4_unicode_ci', please install groonga-normalizer-mysql package.

Install groonga-normalizer-mysql package::

  % sudo apt-get install -y -V groonga-normalizer-mysql

.. note::

   If you don't specify MySQL compatible case insensitive COLLATION, MySQL incompatible COLLATION of Groonga is used.
   MySQL compatible case insensitive COLLATION are 'utf8_general_ci', 'utf8mb4_general_ci', 'utf8_unicode_ci' and 'utf8mb4_unicode_ci'.

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

If you want to use MySQL compatible COLLATION such as 'utf8_general_ci', 'utf8mb4_general_ci', 'utf8_unicode_ci' or 'utf8mb4_unicode_ci', please install groonga-normalizer-mysql package.

Install groonga-normalizer-mysql package::

  % sudo yum install -y install groonga-normalizer-mysql

.. note::

   If you don't specify MySQL compatible case insensitive COLLATION, MySQL incompatible COLLATION of Groonga is used.
   MySQL compatible case insensitive COLLATION are 'utf8_general_ci', 'utf8mb4_general_ci', 'utf8_unicode_ci' and 'utf8mb4_unicode_ci'.


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
  (% sudo scl enable mysql55 mysqladmin -u root password 'new-password')

If you want to use `MeCab <http://mecab.sourceforge.net/>`_ as a tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo yum install -y groonga-tokenizer-mecab

If you want to use MySQL compatible COLLATION such as 'utf8_general_ci', 'utf8mb4_general_ci', 'utf8_unicode_ci' or 'utf8mb4_unicode_ci', please install groonga-normalizer-mysql package.

Install groonga-normalizer-mysql package::

  % sudo yum install -y install groonga-normalizer-mysql

.. note::

   If you don't specify MySQL compatible case insensitive COLLATION, MySQL incompatible COLLATION of Groonga is used.
   MySQL compatible case insensitive COLLATION are 'utf8_general_ci', 'utf8mb4_general_ci', 'utf8_unicode_ci' and 'utf8mb4_unicode_ci'.


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

If you want to use MySQL compatible COLLATION such as 'utf8_general_ci', 'utf8mb4_general_ci', 'utf8_unicode_ci' or 'utf8mb4_unicode_ci', please install groonga-normalizer-mysql package.

Install groonga-normalizer-mysql package::

  % sudo yum install -y install groonga-normalizer-mysql

.. note::

   If you don't specify MySQL compatible case insensitive COLLATION, MySQL incompatible COLLATION of Groonga is used.
   MySQL compatible case insensitive COLLATION are 'utf8_general_ci', 'utf8mb4_general_ci', 'utf8_unicode_ci' and 'utf8mb4_unicode_ci'.

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

Install groonga-tokenizer-mecab package::

  % sudo yum install -y groonga-tokenizer-mecab

If you want to use MySQL compatible COLLATION such as 'utf8_general_ci', 'utf8mb4_general_ci', 'utf8_unicode_ci' or 'utf8mb4_unicode_ci', please install groonga-normalizer-mysql package.

Install groonga-normalizer-mysql package::

  % sudo yum install -y install groonga-normalizer-mysql

.. note::

   If you don't specify MySQL compatible case insensitive COLLATION, MySQL incompatible COLLATION of Groonga is used.
   MySQL compatible case insensitive COLLATION are 'utf8_general_ci', 'utf8mb4_general_ci', 'utf8_unicode_ci' and 'utf8mb4_unicode_ci'.

Mac OS X
--------

Install::

  % brew install https://raw.github.com/mroonga/homebrew/master/mroonga.rb --use-homebrew-mysql

If you want to use `MeCab <http://mecab.sourceforge.net/>`_ as a tokenizer, install with ``--with-mecab`` option.

Install with MeCab support::

  % brew install https://raw.github.com/mroonga/homebrew/master/mroonga.rb --use-homebrew-mysql --with-mecab

.. seealso:: `mroonga/homebrew on GitHub <https://github.com/mroonga/homebrew>`_


Windows
-------

Mroonga binary for Windows is provided with MariaDB binary because
`some changes
<https://github.com/mroonga/mroonga/tree/master/packages/source/patches>`_
are needed for building mroonga for Windows.

Installer
^^^^^^^^^

.. caution::

   The following MSI files don't work yet. Please use zip files below
   or help us to creating MSI files that work well.

Download MSI file and execute it. You need to choose a MSI for your
environment.

Choose ``win32`` version for 32-bit environment, ``winx64`` version
for 64-bit environment:

  * `mariadb-10.0.10-with-mroonga-4.02-win32.msi <https://github.com/mroonga/mroonga/releases/download/v4.02/mariadb-10.0.10-with-mroonga-4.02-win32.msi>`_
  * `mariadb-10.0.10-with-mroonga-4.02-winx64.msi <https://github.com/mroonga/mroonga/releases/download/v4.02/mariadb-10.0.10-with-mroonga-4.02-winx64.msi>`_

Zip
^^^

Download zip file and extract it. You need to choose a zip for your
environment.

Choose ``win32`` version for 32-bit environment, ``winx64`` version
for 64-bit environment:

  * `mariadb-10.0.11-with-mroonga-4.03-win32.zip <https://github.com/mroonga/mroonga/releases/download/v4.03/mariadb-10.0.11-with-mroonga-4.03-win32.zip>`_
  * `mariadb-10.0.11-with-mroonga-4.03-winx64.zip <https://github.com/mroonga/mroonga/releases/download/v4.03/mariadb-10.0.11-with-mroonga-4.03-winx64.zip>`_

Install Mroonga
^^^^^^^^^^^^^^^

Zip packages are pre-configured for easy to use, so no need to execute "INSTALL PLUGIN" and install UDF.

Just start mysqld by following command.::

  > mysqld.exe --defautls-file=.\MY-PREFERRED-INI.ini --console

Each zip package contains ini files (my-small.ini, my-medium.ini, my-large.ini and so on), choose preferred ini file which meets on your demand.

Next connect to MariaDB by following command.::

  > mysql.exe
  MariaDB [(none)]> SHOW ENGINES;
  +--------------------+---------+------------------------------------------------------------+--------------+------+------------+
  | Engine             | Support | Comment                                                    | Transactions | XA   | Savepoints |
  +--------------------+---------+------------------------------------------------------------+--------------+------+------------+
  | CSV                | YES     | CSV storage engine                                         | NO           | NO   | NO         |
  | PERFORMANCE_SCHEMA | YES     | Performance Schema                                         | NO           | NO   | NO         |
  | MEMORY             | YES     | Hash based, stored in memory, useful for temporary tables  | NO           | NO   | NO         |
  | MyISAM             | YES     | MyISAM storage engine                                      | NO           | NO   | NO         |
  | MRG_MyISAM         | YES     | Collection of identical MyISAM tables                      | NO           | NO   | NO         |
  | InnoDB             | DEFAULT | Supports transactions, row-level locking, and foreign keys | YES          | YES  | YES        |
  | mroonga            | YES     | CJK-ready fulltext search, column store                    | NO           | NO   | NO         |
  | Aria               | YES     | Crash-safe tables with MyISAM heritage                     | NO           | NO   | NO         |
  +--------------------+---------+------------------------------------------------------------+--------------+------+------------+
  8 rows in set (0.00 sec)


Install from the source code
----------------------------

Here we explain how to install from the source code. If your environment is not listed above, you need to do so.

Japanese morphological analysis system (MeCab)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If you want to use indexes of tokenizing of each morpheme for full text search, install `MeCab <http://mecab.sourceforge.net/>`_ before installing Groonga.

Download
^^^^^^^^

To install the released version, download the tarball from `packages.groonga.org <http://packages.groonga.org/source/mroonga>`_ .

To install the latest source code, clone the code from `GitHub <https://github.com/mroonga/mroonga/>`_ and invoke `./autogen.sh` (GNU Autotools are required). This way is aimed at skilled developpers. If not, we recommend using the tarball. ::

 % git clone https://github.com/mroonga/mroonga.git
 % cd mroonga
 % ./autogen.sh

Requirements
^^^^^^^^^^^^

MySQL and Groonga should be already installed.

And MySQL's source code is also required to build Mroonga.

Install MySQL
^^^^^^^^^^^^^

Download the latest MySQL 5.6 source code, then build and install it.

http://dev.mysql.com/downloads/mysql/

Here we assume that we use mysql-5.6.17 and its source code is extracted in the following directory. ::

 /usr/local/src/mysql-5.6.17

And we assume that MySQL is installed in the following directory. ::

 /usr/local/mysql

Install Groonga
^^^^^^^^^^^^^^^

Build and install the latest Groonga.

http://groonga.org/docs/

Here we assume that libgroonga is installed in the standard location like /usr/lib etc.

Build Mroonga
^^^^^^^^^^^^^

Run configure script by specifying the location of MySQL source code with ``--with-mysql-source`` and the path of mysql_config command with ``--with-mysql-config``. ::

 ./configure \
   --with-mysql-source=/usr/local/src/mysql-5.6.17 \
   --with-mysql-config=/usr/local/mysql/bin/mysql_config

If Groonga is not installed in the standard location like /usr/lib, you need to specify its location by PKG_CONFIG_PATH. For example, if Groonga is installed with ``--prefix=$HOME/local``, do like the following ::

 ./configure \
   PKG_CONFIG_PATH=$HOME/local/lib/pkgconfig \
   --with-mysql-source=/usr/local/src/mysql-5.6.17 \
   --with-mysql-config=/usr/local/mysql/bin/mysql_config

Then invoke "make". ::

 make

Install Mroonga
^^^^^^^^^^^^^^^

By invoking "make install", ha_mroonga.so will be installed in MySQL's plugin directory. ::

 make install

Then start mysqld, connect to it by mysql client, and install it by "INSTALL PLUGIN" command. ::

 mysql> INSTALL PLUGIN mroonga SONAME 'ha_mroonga.so';

If "mroonga" is displayed in "SHOW ENGINES" command result like below, Mroonga is well installed. ::

 mysql> SHOW ENGINES;
 +------------+---------+------------------------------------------------------------+--------------+------+------------+
 | Engine     | Support | Comment                                                    | Transactions | XA   | Savepoints |
 +------------+---------+------------------------------------------------------------+--------------+------+------------+
 | mroonga    | YES     | Fulltext search, column base                               | NO           | NO   | NO         |
 | MRG_MYISAM | YES     | Collection of identical MyISAM tables                      | NO           | NO   | NO         |
 | CSV        | YES     | CSV storage engine                                         | NO           | NO   | NO         |
 | MyISAM     | DEFAULT | Default engine as of MySQL 3.23 with great performance     | NO           | NO   | NO         |
 | InnoDB     | YES     | Supports transactions, row-level locking, and foreign keys | YES          | YES  | YES        |
 | MEMORY     | YES     | Hash based, stored in memory, useful for temporary tables  | NO           | NO   | NO         |
 +------------+---------+------------------------------------------------------------+--------------+------+------------+
 6 rows in set (0.00 sec)

Next install UDF (User-Defined Function).

To get the record ID assigned by Groonga in INSERT, install last_insert_grn_id function.

Invoke CREATE FUNCTION like the following. ::

 mysql> CREATE FUNCTION last_insert_grn_id RETURNS INTEGER SONAME 'ha_mroonga.so';

To enable snippet (keyword in context) UDF, install mroonga_snippet function.

Invoke CREATE FUNCTION like the following. ::

 mysql> CREATE FUNCTION mroonga_snippet RETURNS STRING SONAME 'ha_mroonga.so';

To enable invoking Groonga query from Mroonga, install mroonga_command function.

Invoke CREATE FUNCTION like the following. ::

 mysql> CREATE FUNCTION mroonga_command RETURNS STRING SONAME 'ha_mroonga.so';

To escape an user input query properly, install mroonga_escape function.

Invoke CREATE FUNCTION like the following. ::

 mysql> CREATE FUNCTION mroonga_escape RETURNS STRING SONAME 'ha_mroonga.so';

Install from the source code with MariaDB
-----------------------------------------

Here we explain how to install from the source code with MariaDB. If your environment is not listed above, you need to do so.

Japanese morphological analysis system (MeCab)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If you want to use indexes of tokenizing of each morpheme for full text search, install `MeCab <http://mecab.sourceforge.net/>`_ before installing Groonga.

Download
^^^^^^^^

Download Mroonga tarball from `packages.groonga.org <http://packages.groonga.org/source/mroonga>`_ .

Download MariaDB tarball from `downloads.mariadb.org <https://downloads.mariadb.org/>`_ .

Requirements
^^^^^^^^^^^^

Groonga should be already installed.

Install Groonga
^^^^^^^^^^^^^^^

Build and install the latest Groonga.

http://groonga.org/docs/

Here we assume that libgroonga is installed in the standard location like /usr/lib etc.

Build Mroonga with MariaDB
^^^^^^^^^^^^^^^^^^^^^^^^^^

Uncompress MariaDB tarball. ::

 tar xvfz mariadb-10.0.11.tar.gz

Uncompress Mroonga tarball then move into storage directory. ::

 tar xvfz mroonga-4.03.tar.gz
 mv mroonga-4.03 mariadb-10.0.11/storage/mroonga/

Run "cmake". ::

 cd mariadb-10.0.11
 cmake .

Then invoke "make". ::

 make

Install Mroonga with MariaDB
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

By invoking "make install", MariaDB and Mroonga will be installed in "/usr/local/mysql" directory. ::

 make install

Then start mysqld, connect to it by MariaDB client, and install it by "INSTALL PLUGIN" command. ::

 mysql> INSTALL PLUGIN mroonga SONAME 'ha_mroonga.so';

If "mroonga" is displayed in "SHOW ENGINES" command result like below, Mroonga is well installed. ::

 mysql> SHOW ENGINES;
 +--------------------+---------+------------------------------------------------------------+--------------+------+------------+
 | Engine             | Support | Comment                                                    | Transactions | XA   | Savepoints |
 +--------------------+---------+------------------------------------------------------------+--------------+------+------------+
 | CSV                | YES     | CSV storage engine                                         | NO           | NO   | NO         |
 | PERFORMANCE_SCHEMA | YES     | Performance Schema                                         | NO           | NO   | NO         |
 | MEMORY             | YES     | Hash based, stored in memory, useful for temporary tables  | NO           | NO   | NO         |
 | MyISAM             | DEFAULT | MyISAM storage engine                                      | NO           | NO   | NO         |
 | MRG_MyISAM         | YES     | Collection of identical MyISAM tables                      | NO           | NO   | NO         |
 | InnoDB             | NO      | Supports transactions, row-level locking, and foreign keys | NULL         | NULL | NULL       |
 | mroonga            | YES     | CJK-ready fulltext search, column store                    | NO           | NO   | NO         |
 | Aria               | YES     | Crash-safe tables with MyISAM heritage                     | NO           | NO   | NO         |
 +--------------------+---------+------------------------------------------------------------+--------------+------+------------+
 8 rows in set (0.01 sec)

Next install UDF (User-Defined Function).

To get the record ID assigned by Groonga in INSERT, install last_insert_grn_id function.

Invoke CREATE FUNCTION like the following. ::

 mysql> CREATE FUNCTION last_insert_grn_id RETURNS INTEGER SONAME 'ha_mroonga.so';

To enable snippet (keyword in context) UDF, install mroonga_snippet function.

Invoke CREATE FUNCTION like the following. ::

 mysql> CREATE FUNCTION mroonga_snippet RETURNS STRING SONAME 'ha_mroonga.so';

To enable invoking Groonga query from Mroonga, install mroonga_command function.

Invoke CREATE FUNCTION like the following. ::

 mysql> CREATE FUNCTION mroonga_command RETURNS STRING SONAME 'ha_mroonga.so';

To escape an user input query properly, install mroonga_escape function.

Invoke CREATE FUNCTION like the following. ::

 mysql> CREATE FUNCTION mroonga_escape RETURNS STRING SONAME 'ha_mroonga.so';

Upgrade Guide
-------------

There is a case that incompatible change is introduced at new release.
It is announced by release announce if new release contains such a incompatible change.

Here is the list of recommended way of upgrading Mroonga from old release.

See following URL about upgrade sequence if you use previous version.

If you upgrade prior to 1.20, refer to :ref:`release-1-20`

If you upgrade from 1.20, refer to :ref:`release-2-00`

If you upgrade from 2.00 or 2.01, refer to :ref:`release-2-02`

If you upgrade from 2.00 or later and using multiple column indexes on storage mode, refer to :ref:`release-2-03`

If you upgrade from 2.04 or later and using SET column or ENUM that has the number of elements < 256 in Storage mode, refer to :ref:`release-2-05`

If you upgrade from 2.05 or later and using multiple column indexes against VARCHAR or CHAR, refer to :ref:`release-2-08`

If you upgrade from 2.08 or later and using TIMESTAMP column, please recreate database. If you upgrade from 2.08 or later and using CHAR(N) as primary key, please recreate index. Refer to :ref:`release-2-09` for each case:

