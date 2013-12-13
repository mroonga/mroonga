.. highlightlang:: none

Installation check
==================

It is better that you check Mroonga installation before you use
Mroonga. If Mroonga installation is failed, the SQLs in this tutorial
will fail.

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

