.. highlightlang:: none

Installation Guide
==================

If you use binary packages, MySQL related packages will be also installed when you install mroonga package.

We distribute packages for both 32-bit and 64-bit but we recommend that you should use 64-bit package for server. You should use 32-bit package just only for test or development. You will get no memory error with 32-bit package even if you just process medium size data.

Here we explain how to install for each environment.

Debian GNU/Linux squeeze
------------------------

/etc/apt/sources.list.d/groonga.list::

  deb http://packages.groonga.org/debian/ squeeze main
  deb-src http://packages.groonga.org/debian/ squeeze main

Install ::

  % sudo apt-key adv --recv-keys --keyserver keyserver.ubuntu.com 1C837F31
  % sudo aptitude update
  % sudo aptitude -V -D -y install mysql-server-mroonga

Debian GNU/Linux wheezy
-----------------------

/etc/apt/sources.list.d/groonga.list::

  deb http://packages.groonga.org/debian/ wheezy main
  deb-src http://packages.groonga.org/debian/ wheezy main

Install ::

  % sudo apt-key adv --recv-keys --keyserver keyserver.ubuntu.com 1C837F31
  % sudo aptitude update
  % sudo aptitude -V -D -y install mysql-server-mroonga

Debian GNU/Linux sid
--------------------

/etc/apt/sources.list.d/groonga.list::

  deb http://packages.groonga.org/debian/ unstable main
  deb-src http://packages.groonga.org/debian/ unstable main

Install ::

  % sudo apt-key adv --recv-keys --keyserver keyserver.ubuntu.com 1C837F31
  % sudo aptitude update
  % sudo aptitude -V -D -y install mysql-server-mroonga

Ubuntu 10.04 LTS Lucid Lynx
---------------------------

.. note::

   You need to enable the universe section in Ubuntu's software sources.

/etc/apt/sources.list.d/groonga.list::

  deb http://packages.groonga.org/ubuntu/ lucid universe
  deb-src http://packages.groonga.org/ubuntu/ lucid universe

Install ::

  % sudo apt-key adv --recv-keys --keyserver keyserver.ubuntu.com 1C837F31
  % sudo aptitude update
  % sudo aptitude -V -D -y install mysql-server-mroonga

Ubuntu 11.04 Natty Narwhal
--------------------------

.. note::

   You need to enable the universe section in Ubuntu's software sources.

/etc/apt/sources.list.d/groonga.list::

  deb http://packages.groonga.org/ubuntu/ natty universe
  deb-src http://packages.groonga.org/ubuntu/ natty universe

Install ::

  % sudo apt-key adv --recv-keys --keyserver keyserver.ubuntu.com 1C837F31
  % sudo aptitude update
  % sudo aptitude -V -D -y install mysql-server-mroonga

Ubuntu 11.10 Oneiric Ocelot
---------------------------

.. note::

   You need to enable the universe section in Ubuntu's software sources.

/etc/apt/sources.list.d/groonga.list::

  deb http://packages.groonga.org/ubuntu/ oneiric universe
  deb-src http://packages.groonga.org/ubuntu/ oneiric universe

Install ::

  % sudo apt-key adv --recv-keys --keyserver keyserver.ubuntu.com 1C837F31
  % sudo aptitude update
  % sudo aptitude -V -D -y install mysql-server-mroonga

CentOS 5
--------

CentOS 5's MySQL packages should be removed beforehand if installed.

Remove existing MySQL packages ::

  % sudo yum remove mysql*

Install ::

  % sudo rpm -ivh http://packages.groonga.org/centos/groonga-repository-1.0.0-0.noarch.rpm
  % sudo yum update
  % sudo yum install -y mysql-mroonga

CentOS 6
--------

In CentOS 6, unlike in CentOS 5, we use CentOS's MySQL packages (version 5.1.x). So you don't need to remove CentOS's MySQL packages.

Install ::

  % sudo rpm -ivh http://packages.groonga.org/centos/groonga-repository-1.0.0-0.noarch.rpm
  % sudo yum update
  % sudo yum install -y mysql-mroonga

Fedora 16
---------

Install ::

  % sudo rpm -ivh http://packages.groonga.org/fedora/groonga-repository-1.0.0-0.noarch.rpm
  % sudo yum update
  % sudo yum install -y mysql-mroonga

Install from the source code
------------------------------

Here we explain how to install from the source code. If your environment is not listed above, you need to do so.

Japanese morphological analysis system (MeCab)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If you want to use indexes of tokenizing of each morpheme for full text search, install `MeCab <http://mecab.sourceforge.net/>`_ before installing groonga.

Download
^^^^^^^^

To install the released version, download the tarball from `GitHub's download page <http://github.com/mroonga/mroonga/downloads>`_ .

To install the latest source code, clone the code from `GitHub <https://github.com/mroonga/mroonga/>`_ and invoke `./autogen.sh` (GNU Autotools are required). This way is aimed at skilled developpers. If not, we recommend using the tarball. ::

 % git clone https://github.com/mroonga/mroonga.git
 % mroonga
 % ./autogen.sh

Requirements
^^^^^^^^^^^^

MySQL and groonga should be already installed.

And MySQL's source code is also required to build mroonga.

Install MySQL
^^^^^^^^^^^^^

Download the latest MySQL 5.5 source code, then build and install it.

http://dev.mysql.com/downloads/mysql/

Here we assume that we use mysql-5.5.17 and its source code is extracted in the following directory. ::

 /usr/local/src/mysql-5.5.17

And we assume that MySQL is installed in the following directory. ::

 /usr/local/mysql

Install groonga
^^^^^^^^^^^^^^^

Build and install the latest groonga.

http://groonga.org/docs/

Here we assume that libgroonga is installed in the standard location like /usr/lib etc.

Build mroonga
^^^^^^^^^^^^^

Run configure script by specifying the location of MySQL source code with ``--with-mysql-source`` and the path of mysql_config command with ``--with-mysql-config``. ::

 ./configure \
   --with-mysql-source=/usr/local/src/mysql-5.5.17 \
   --with-mysql-config=/usr/local/mysql/bin/mysql_config

If groonga is not installed in the standard location like /usr/lib, you need to specify its location by PKG_CONFIG_PATH. For example, if groonga is installed with ``--prefix=$HOME/local``, do like the following ::

 ./configure \
   PKG_CONFIG_PATH=$HOME/local/lib/pkgconfig \
   --with-mysql-source=/usr/local/src/mysql-5.5.17 \
   --with-mysql-config=/usr/local/mysql/bin/mysql_config

Then invoke "make". ::

 make

Install mroonga
^^^^^^^^^^^^^^^

By invoking "make install", ha_mroonga.so will be installed in MySQL's plugin directory. ::

 make install

Then start mysqld, connect to it by mysql client, and install it by "INSTALL PLUGIN" command. ::

 mysql> INSTALL PLUGIN groonga SONAME 'ha_mroonga.so';

If "groonga" is displayed in "SHOW ENGINES" command result like below, mroonga is well installed. ::

 mysql> SHOW ENGINES;
 +------------+---------+------------------------------------------------------------+--------------+------+------------+
 | Engine     | Support | Comment                                                    | Transactions | XA   | Savepoints |
 +------------+---------+------------------------------------------------------------+--------------+------+------------+
 | groonga    | YES     | Fulltext search, column base                               | NO           | NO   | NO         |
 | MRG_MYISAM | YES     | Collection of identical MyISAM tables                      | NO           | NO   | NO         |
 | CSV        | YES     | CSV storage engine                                         | NO           | NO   | NO         |
 | MyISAM     | DEFAULT | Default engine as of MySQL 3.23 with great performance     | NO           | NO   | NO         |
 | InnoDB     | YES     | Supports transactions, row-level locking, and foreign keys | YES          | YES  | YES        |
 | MEMORY     | YES     | Hash based, stored in memory, useful for temporary tables  | NO           | NO   | NO         |
 +------------+---------+------------------------------------------------------------+--------------+------+------------+
 6 rows in set (0.00 sec)

Next install UDF (User-Defined Function).

To get the record ID assigned by groonga in INSERT, install last_insert_grn_id function.

Invoke CREATE FUNCTION like the following. ::

 mysql> CREATE FUNCTION last_insert_grn_id RETURNS INTEGER soname 'ha_mroonga.so';
