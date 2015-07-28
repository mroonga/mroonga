.. highlightlang:: none

Others
======

This section describes how to install Mroonga from source code. If
there is no package no your environment, you need to build Mroonga
from source code.

Dependencies
------------

Mroonga needs some tools, libraries and MySQL for build. You can use
MariaDB instead of MySQL.

Tools
^^^^^

Here are required tools:

  * ``wget``, ``curl`` or Web browser for downloading source archive
  * ``tar`` and ``gzip`` for extracting source archive
  * shell
    (many shells such as ``dash``, ``bash`` and ``zsh`` will work)
  * C compiler and C++ compiler
    (``gcc`` and ``g++`` are supported but other compilers may work)
  * ``make`` (GNU make is supported but other make like BSD make will work)
  * `pkg-config
    <http://www.freedesktop.org/wiki/Software/pkg-config>`_ for
    detecting libraries

You must get them ready.

You can use `CMake <http://www.cmake.org/>`_ instead of shell but this
document doesn't describe about building with CMake.

Here are optional tools:

  * `sudo <http://www.gratisoft.us/sudo/>`_ for installing built
    Mroonga.

Libraries
^^^^^^^^^

Here are required libraries:

  * `Groonga <http://groonga.org/>`_. (If you use package, install
    development package such as ``libgroonga-dev`` for deb or
    ``groonga-devel`` for RPM.)
  * `groonga-normalizer-mysql
    <https://github.com/groonga/groonga-normalizer-mysql>`_.

Here are optional libraries:

  * `MeCab <http://mecab.sourceforge.net/>`_:
    Japanese morphological analysis system

    .. note:: If you want to use indexes of tokenizing of each
              morpheme for full text search, install MeCab before
              installing Groonga.

MySQL
^^^^^

Mroonga needs not only installed MySQL but also MySQL source and build
directory. You can't use MySQL package. It doesn't provide MySQL
source and build directory. You need MySQL source and build directory!

If you use MariaDB instead of MySQL, you need MariaDB source.

Download the latest MySQL 5.6 source code, then build and install it.

.. seealso:: `Download MySQL Community Server <http://dev.mysql.com/downloads/mysql/>`_

Here we assume that you use mysql-5.6.21 and its source code is
extracted in the following directory::

  /usr/local/src/mysql-5.6.21

Then build in the following directory::

  /usr/local/build/mysql-5.6.21

Here are command lines to build and install MySQL::

  % cd /usr/local/build/mysql-5.6.21
  % cmake /usr/local/src/mysql-5.6.21
  % make
  % sudo make install

And we assume that MySQL is installed in the following directory::

  /usr/local/mysql

Build from source
-----------------

Mroonga uses GNU build system. So the following is the simplest build
steps::

  % wget http://packages.groonga.org/source/mroonga/mroonga-5.05.tar.gz
  % tar xvzf mroonga-5.05.tar.gz
  % cd mroonga-5.05
  % ./configure \
      --with-mysql-source=/usr/local/src/mysql-5.6.21 \
      --with-mysql-build=/usr/local/build/mysql-5.6.21 \
      --with-mysql-config=/usr/local/mysql/bin/mysql_config
  % make
  % sudo make install
  % /usr/local/mysql/bin/mysql -u root < /usr/local/share/mroonga/install.sql

You need to specify the following on ``configure``:

  * The location of MySQL source code with ``--with-mysql-source``.
  * The location of MySQL build directory with ``--with-mysql-build``.
  * The path of ``mysql_config`` command with ``--with-mysql-config``.

You can confirm Mroonga is installed successfully by ``SHOW ENGINES``
SQL. If you can find ``Mroonga`` row, Mroonga is installed
successfully::

  mysql> SHOW ENGINES;
  +------------+---------+------------------------------------------------------------+--------------+------+------------+
  | Engine     | Support | Comment                                                    | Transactions | XA   | Savepoints |
  +------------+---------+------------------------------------------------------------+--------------+------+------------+
  | Mroonga    | YES     | Fulltext search, column base                               | NO           | NO   | NO         |
  | MRG_MYISAM | YES     | Collection of identical MyISAM tables                      | NO           | NO   | NO         |
  | CSV        | YES     | CSV storage engine                                         | NO           | NO   | NO         |
  | MyISAM     | DEFAULT | Default engine as of MySQL 3.23 with great performance     | NO           | NO   | NO         |
  | InnoDB     | YES     | Supports transactions, row-level locking, and foreign keys | YES          | YES  | YES        |
  | MEMORY     | YES     | Hash based, stored in memory, useful for temporary tables  | NO           | NO   | NO         |
  +------------+---------+------------------------------------------------------------+--------------+------+------------+
  6 rows in set (0.00 sec)

The following describes details about each step.

.. _source-configure:

``configure``
^^^^^^^^^^^^^

First, you need to run ``configure``. Here are important ``configure``
parameters:

``--with-mysql-source=PATH``
++++++++++++++++++++++++++++

Specifies the location of MySQL source code.

This is required parameter::

  % ./configure \
      --with-mysql-source=/usr/local/src/mysql-5.6.21 \
      --with-mysql-config=/usr/local/mysql/bin/mysql_config

``--with-mysql-build=PATH``
+++++++++++++++++++++++++++

Specifies the location where you build MySQL source code.

If you build MySQL in MySQL source code directory, you don't need to
specify this parameter. If you build MySQL in other directory, you
need to specify this parameter.

Here is an example when you build MySQL in
``/usr/local/build/mysql-5.6.21``::

  % ./configure \
      --with-mysql-source=/usr/local/src/mysql-5.6.21 \
      --with-mysql-build=/usr/local/build/mysql-5.6.21 \
      --with-mysql-config=/usr/local/mysql/bin/mysql_config

``--with-mysql-config=PATH``
++++++++++++++++++++++++++++

Specifies the path of ``mysql_config`` command.

If ``mysql_config`` command can be found by ``PATH``, you don't
need to specify this parameter. For example, if ``mysql_config``
command exists at ``/usr/bin/mysql_config``, you don't need to specify
this parameter::

  % ./configure \
      --with-mysql-source=/usr/local/src/mysql-5.6.21

``--with-default-parser=PARSER``
++++++++++++++++++++++++++++++++

Specifies the default parser for full text. You can custom it in
my.cnf.

The default is ``TokenBigram``.

Here is an example to use ``TokenMecab`` as the default parser::

  % ./configure \
      --with-mysql-source=/usr/local/src/mysql-5.6.21 \
      --with-mysql-config=/usr/local/mysql/bin/mysql_config \
      --with-default-parser=TokenMecab

``--prefix=PATH``
+++++++++++++++++

Specifies the install base directory. Mroonga related files are
installed under ``${PATH}/`` directory except
``ha_mroonga.so``. ``ha_mroonga.so`` is a MySQL plugin file. It is
installed the plugin directory of MySQL.

The default is ``/usr/local``. In this case, ``install.sql`` that is
used for installing Mroonga is installed to
``/usr/local/share/mroonga/install.sql``.

Here is an example that installs Mroonga into ``~/local`` for an user
use instead of system wide use::

  % ./configure \
      --prefix=$HOME/local \
      --with-mysql-source=$HOME/local/src/mysql-5.6.21 \
      --with-mysql-config=$HOME/local/mysql/bin/mysql_config

``PKG_CONFIG_PATH=PATH``
++++++++++++++++++++++++

This is not a ``configure`` parameter but we describe it for users who
doesn't install Groonga into the standard location.

If Groonga is not installed in the standard location like
``/usr/lib``, you need to specify its location by
``PKG_CONFIG_PATH``. For example, if Groonga is installed with
``--prefix=$HOME/local``, use the following command line::

  ./configure \
    PKG_CONFIG_PATH=$HOME/local/lib/pkgconfig \
    --with-mysql-source=/usr/local/src/mysql-5.6.21 \
    --with-mysql-config=/usr/local/mysql/bin/mysql_config

``make``
^^^^^^^^

``configure`` is succeeded, you can build Mroonga by ``make``::

  % make

If you have multi cores CPU, you can make faster by using ``-j``
option. If you have 4 cores CPU, it's good for using ``-j4`` option::

  % make -j4

If you get some errors by ``make``, please report them to us:
:doc:`/contribution/report`

``make install``
^^^^^^^^^^^^^^^^

Now, you can install built Mroonga!::

  % sudo make install

If you have write permission for ``${PREFIX}`` and the plugin
directory of MySQL, you don't need to use
``sudo``. e.g. ``--prefix=$HOME/local`` case. In this case, use ``make
install``::

  % make install

``mysql -u root < install.sql``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

You need to run some SQLs to register Mroonga to MySQL such as
``INSTALL PLUGIN`` and ``CREATE FUNCTION``. They are written in
``${PREFIX}/share/mroonga/install.sql``.

Here is an example when you specify ``--prefix=$HOME/local`` to
``configure``::

  % mysql -u < $HOME/local/share/mroonga/install.sql
