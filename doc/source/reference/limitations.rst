.. highlightlang:: none

Limitations
===========

There are some limitations in Mroonga storage engine.

Limitations of table
--------------------

A table has the following limitations.
This limitation is derived from Groonga.

* The maximum one key size: 4096Bytes
* The maximum total size of keys: 4GBytes

Keep in mind that these limitations may vary depending on conditions.

Limitations of indexing
-----------------------

A full-text index has the following limitations.
This limitation is derived from groonga and applied to each table.

* The maximum number of records: 268,435,455 (more than 268 million)
* The maximum number of distinct terms: 268,435,455 (more than 268 million)
* The maximum index size: 256GBytes

Keep in mind that these limitations may vary depending on conditions.

Confirm by ``SHOW TABLE STATUS`` or ``SHOW INDEX FROM ...`` whether your table data matches to this limitations.


Limitations about the value of columns
--------------------------------------

There is a limitation about DATE, DATETIME column in storage mode.

mroonga storage engine automatically convert 0 into 1 as the value of month or date.

Thus, the value 0 is treated as the 1st month (January) of the year or
the 1st date of the month.

And more, the value NULL is treated as the value of UNIX time 0 (1970-01-01 00:00:00).

Here is an example to show behavior described above.

.. code-block:: sql
   :linenos:

   CREATE TABLE date_limitation (
     id INT PRIMARY KEY AUTO_INCREMENT,
     input varchar(32) DEFAULT NULL,
     date DATE DEFAULT NULL
   ) ENGINE=mroonga DEFAULT CHARSET=UTF8;
   CREATE TABLE datetime_limitation (
     id INT PRIMARY KEY AUTO_INCREMENT,
     input varchar(32) DEFAULT NULL,
     datetime DATETIME DEFAULT NULL
   ) ENGINE=mroonga DEFAULT CHARSET=UTF8;

   -- Test data for date_limitation
   INSERT INTO date_limitation (input) VALUES ("NULL");
   INSERT INTO date_limitation (input, date) VALUES ("1970-00-00", "1970-00-00");

   -- Test data for datetime_limitation
   INSERT INTO datetime_limitation (input) VALUES ("NULL");
   INSERT INTO datetime_limitation (input, datetime) VALUES ("1970-00-00 00:00:00", "1970-00-00 00:00:00");

Here is the results of execution example::

  mysql> select * from date_limitation;
  +----+------------+------------+
  | id | input      | date       |
  +----+------------+------------+
  |  1 | NULL       | 1970-01-01 |
  |  2 | 1970-00-00 | 1970-01-01 |
  +----+------------+------------+
  2 rows in set (0.00 sec)
  
  mysql> select * from datetime_limitation;
  +----+---------------------+---------------------+
  | id | input               | datetime            |
  +----+---------------------+---------------------+
  |  1 | NULL                | 1970-01-01 00:00:00 |
  |  2 | 1970-00-00 00:00:00 | 1970-01-01 00:00:00 |
  +----+---------------------+---------------------+
  2 rows in set (0.00 sec)
  
