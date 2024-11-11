Windows
=======

This section describes how to install Mroonga on Windows. You can
install Mroogna by extracting a zip package.

Mroonga binary for Windows is provided with MariaDB binary because
`some changes
<https://github.com/mroonga/mroonga/tree/main/packages/source/patches>`_
are needed for building Mroonga for Windows.

Install
-------

Download and extract the zip file
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Download zip file and extract it. You need to choose a zip for your
environment:

  * |mroonga_mariadb_windows_package_link|

Run ``mariadb-install-db.exe``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Execution Example::

  > mariadb-install-db.exe --datadir=C:\EXAMPLE\data --service=MariaDB --password=PASSWORD

* ``--datadir=C:\EXAMPLE\data``

  * Data directory of the new database

* ``--service=MariaDB``

  * Name of the Windows service

  * Specify if registering as a Windows service

* ``--password=PASSWORD``

  * Password of the root user

(``mariadb-install-db.exe`` is included in the zip file.)

Start server command
^^^^^^^^^^^^^^^^^^^^

Zip packages are pre-configured for easy to use, so no need to execute
``INSTALL PLUGIN`` and ``CREATE FUNCTION``.

Just start mysqld by following command::

  > mysqld.exe --defaults-file=.\MY-PREFERRED-INI.ini --console

Each zip package contains ini files (my-small.ini, my-medium.ini,
my-large.ini and so on), choose preferred ini file which meets on your
demand.

Next connect to MariaDB by following command::

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
  | Mroonga            | YES     | CJK-ready fulltext search, column store                    | NO           | NO   | NO         |
  | Aria               | YES     | Crash-safe tables with MyISAM heritage                     | NO           | NO   | NO         |
  +--------------------+---------+------------------------------------------------------------+--------------+------+------------+
  8 rows in set (0.00 sec)

Build from source with MariaDB
------------------------------

You need to use Mroonga bundled MariaDB source provided by the Mroonga
project.

You can find it in
`<https://packages.groonga.org/source/mroonga/>`_. Mroonga bundled
MariaDB source has
``mariadb-${MARIADB_VERSION}-with-mroonga-${MROONGA_VERSION}.zip``
file name.

You can build the source code with `the standard MariaDB build process
<https://mariadb.com/kb/en/mariadb/documentation/getting-started/compiling-mariadb-from-source/Building_MariaDB_on_Windows/>`_.

You need to register Mroonga after building MariaDB. Use SQL at
``${MARIADB_BUILD_DIR}\storage\mroonga\data\install.sql`` to register
Mroonga.
