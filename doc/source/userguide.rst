.. highlightlang:: none

User's guide
============

Please read the following to see how to install : :doc:`install`

How to check the installation
-----------------------------

The way to start or stop MySQL server is just same as the normal MySQL.

After invoking the MySQL server, connect to it by mysql command. If you set password, you need to add '-p' option. ::

 shell> mysql -uroot test

By using SHOW ENGINES command, you can check if mroonga is installed. ::

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

If you see "mroonga" storage engine like above, the installation is well done.

If not installed, invoke INSTALL PLUGIN command like below. ::

 mysql> INSTALL PLUGIN mroonga SONAME 'ha_mroonga.so';

Running modes
-------------

Mroonga can run in the following two modes.

* storage mode
* wrapper mode

With the storage mode, we use groonga for both of the full text search function and the data storage.
Since all functions of storage engine are realised with groonga, aggregations are fast, that is one of groonga's advantages, and you can manage the database directly by ``groonga`` command.

The structure of the storage mode is the following. You use it instead of existing storage engines like MyISAM or InnoDB

.. figure:: /images/storage-mode.png
   :alt: storage mode
   :align: center

With the wrapper mode, groonga is used for full text search function only, and another existing storage engine like InnoDB is used for storing data.
By using wrapper mode, you combine InnoDB that is well-use as the storage engine and mroonga that is a proven full text search engine, and you can use it as the stable database having the fast full text search function.

The structure of the wrapper mode is the following. Full text search related operations are done by mroonga, and other operations are done by existing storage engines like MyISAM, InnoDB etc.
Mroonga is located between SQL Handler that processes SQL and an existing storage engine, thus all data goes through mroonga
With this way, full text search indexing etc. are done transparently.

.. figure:: /images/wrapper-mode.png
   :alt: wrapper mode
   :align: center

Usage of each mode
------------------

Please see the following pages for the usage of each mode.

.. toctree::
   :maxdepth: 2

   userguide/storage.rst
   userguide/wrapper.rst
