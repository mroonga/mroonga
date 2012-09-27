.. highlightlang:: none

Reference
=========

List of available SQL commands
------------------------------

* ``ALTER TABLE table_name ADD COLUMN column_name TEXT``
* ``ALTER TABLE table_name ADD FULLTEXT INDEX index_name (column_name)``
* ``ALTER TABLE table_name ADD SPATIAL KEY index_name (geometry_column_name)``
* ``ALTER TABLE table_name ENGINE = mroonga``
* ``ALTER TABLE table_name RENAME new_table_name``
* ``COMMIT``
* ``CREATE FULLTEXT INDEX index_name ON table_name(column_name)``
* ``CREATE FULLTEXT INDEX index_name ON table_name(column_name, ...)``
* ``CREATE TABLE \`table-name-with-hyphen\` (...)``
* ``CREATE TABLE table_name (... FULLTEXT INDEX index_name (column_name) COMMENT 'parser "TokenMecab"')``
* ``CREATE TABLE table_name (... FULLTEXT INDEX index_name (column_name))``
* ``CREATE TABLE table_name (... FULLTEXT INDEX index_name (column_name, ...))``
* ``CREATE TABLE table_name (... INDEX (column_name))``
* ``CREATE TABLE table_name (... INDEX (column_name, ...))``
* ``CREATE TABLE table_name (... INDEX USING BTREE (column_name))``
* ``CREATE TABLE table_name (... PRIMARY KEY (column_name))``
* ``CREATE TABLE table_name (...) CAHRSET UTF8``
* ``CREATE TABLE table_name (...) ENGINE=mroonga COMMENT = 'ENGINE "InnoDB"'``
* ``CREATE TABLE table_name (...) ENGINE=mroonga``
* ``CREATE TABLE table_name (\`_id\` INT)``
* ``CREATE TABLE table_name (\`_id\` INT, KEY(_id) USING HASH)``
* ``CREATE TABLE table_name (column_name BIGINT UNSIGNED)``
* ``CREATE TABLE table_name (column_name BIGINT)``
* ``CREATE TABLE table_name (column_name BINARY(...))``
* ``CREATE TABLE table_name (column_name BIT)``
* ``CREATE TABLE table_name (column_name BLOB)``
* ``CREATE TABLE table_name (column_name CHAR(...))``
* ``CREATE TABLE table_name (column_name DATE)``
* ``CREATE TABLE table_name (column_name DATETIME)``
* ``CREATE TABLE table_name (column_name DECIMAL)``
* ``CREATE TABLE table_name (column_name DOUBLE)``
* ``CREATE TABLE table_name (column_name ENUM(...))``
* ``CREATE TABLE table_name (column_name FLOAT)``
* ``CREATE TABLE table_name (column_name GEOMETRY NOT NULL)``
* ``CREATE TABLE table_name (column_name INT UNSIGNED)``
* ``CREATE TABLE table_name (column_name INT)``
* ``CREATE TABLE table_name (column_name LONGBLOB)``
* ``CREATE TABLE table_name (column_name LONGTEXT)``
* ``CREATE TABLE table_name (column_name MEDIUMBLOB)``
* ``CREATE TABLE table_name (column_name MEDIUMINT UNSIGNED)``
* ``CREATE TABLE table_name (column_name MEDIUMINT)``
* ``CREATE TABLE table_name (column_name MEDIUMTEXT)``
* ``CREATE TABLE table_name (column_name SET(...))``
* ``CREATE TABLE table_name (column_name SMALLINT UNSIGNED)``
* ``CREATE TABLE table_name (column_name SMALLINT)``
* ``CREATE TABLE table_name (column_name TEXT)``
* ``CREATE TABLE table_name (column_name TIME)``
* ``CREATE TABLE table_name (column_name TIMESTAMP)``
* ``CREATE TABLE table_name (column_name TINYBLOB)``
* ``CREATE TABLE table_name (column_name TINYINT UNSIGNED)``
* ``CREATE TABLE table_name (column_name TINYINT)``
* ``CREATE TABLE table_name (column_name TINYTEXT)``
* ``CREATE TABLE table_name (column_name VARBINARY(...))``
* ``CREATE TABLE table_name (column_name VARCHAR(...))``
* ``CREATE TABLE table_name (column_name YEAR)``
* ``CREATE TABLE table_name (column_name INT AUTO_INCREMENT)``
* ``CREATE TABLE table_name (column_name INT PRIMARY KEY)``
* ``CREATE TABLE table_name (column_name DATE PRIMARY KEY)``
* ``CREATE TABLE table_name (column_name DATETIME PRIMARY KEY)``
* ``CREATE TABLE table_name (column_name DATETIME(fractional_seconds_precision) PRIMARY KEY)``
* ``CREATE TABLE table_name (column_name DECIMAL PRIMARY KEY)``
* ``CREATE TABLE table_name (column_name DECIMAL(maximum_number_digits, fractional_seconds_precision) PRIMARY KEY)``
* ``CREATE TABLE table_name (column_name TIME PRIMARY KEY)``
* ``CREATE TABLE table_name (column_name TIME(fractional_seconds_precision) PRIMARY KEY)``
* ``CREATE TABLE table_name (column_name TIMESTAMP PRIMARY KEY)``
* ``CREATE TABLE table_name (column_name TIMESTAMP(fractional_seconds_precision) PRIMARY KEY)``
* ``CREATE TABLE table_name (column_name YEAR PRIMARY KEY)``
* ``CREATE TABLE table_name (column_name1 INT, column_name2 ..., KEY (column_name1, column_name2))``
* ``CREATE TABLE table_name (column_name1 DOUBLE, column_name2 ..., KEY (column_name1, column_name2))``
* ``CREATE TABLE table_name (column_name1 FLOAT, column_name2 ..., KEY (column_name1, column_name2))``
* ``CREATE TABLE table_name (column_name1 CHAR(...), column_name2 ..., KEY (column_name1, column_name2))``
* ``CREATE TABLE table_name (column_name1 VARCHAR(...), column_name2 ..., KEY (column_name1, column_name2))``
* ``CREATE TEMPORARY TABLE table_name (...)``
* ``DELETE FROM table_name WHERE ...``
* ``DROP INDEX column_name ON table_name``
* ``DROP TABLE IF EXISTS table_name, ...``
* ``DROP TABLE \`table-name-with-hyphen\`, ...``
* ``DROP TABLE table_name, ...``
* ``DROP TEMPORARY TABLE table_name``
* ``FLUSH LOGS``
* ``FLUSH TABLES``
* ``INSERT INTO (geometry_column_name) VALUES (GeomFromText('POINT(...)'))``
* ``INSERT INTO ... VALUES ...``
* ``INSERT INTO ... VALUES ... ON DUPLICATE KEY UPDATE ...`` (for ``PRIMARY KEY``)
* ``INSERT INTO ... VALUES ... ON DUPLICATE KEY UPDATE ...`` (for ``UNIQUE KEY``)
* ``INSERT INTO table_name (column_name, ...) SELECT ... FROM other_table_name``
* ``REPLACE INTO table_name SELECT ... FROM other_table_name ...``
* ``SELECT * FROM information_schema.plugins WHERE plugin_name = "mroonga"``
* ``SELECT * FROM table_name``
* ``SELECT * FROM table_name FORCE INDEX(index_name) WHERE ...``
* ``SELECT * FROM table_name ORDER BY column_name ASC LIMIT ...``
* ``SELECT * FROM table_name ORDER BY column_name ASC``
* ``SELECT * FROM table_name ORDER BY column_name DESC LIMIT ...``
* ``SELECT * FROM table_name ORDER BY column_name DESC``
* ``SELECT * FROM table_name ORDER BY column_name``
* ``SELECT * FROM table_name WHERE MATCH(column_name) AGAINST("*D+ word ..." IN BOOLEAN MODE)``
* ``SELECT * FROM table_name WHERE MATCH(column_name) AGAINST("*D- word ..." IN BOOLEAN MODE)``
* ``SELECT * FROM table_name WHERE MATCH(column_name) AGAINST("*DOR word ..." IN BOOLEAN MODE)``
* ``SELECT * FROM table_name WHERE MATCH(column_name) AGAINST("+word ..." IN BOOLEAN MODE)``
* ``SELECT * FROM table_name WHERE MATCH(column_name) AGAINST("..." IN BOOLEAN MODE)``
* ``SELECT * FROM table_name WHERE MATCH(column_name, ...) AGAINST("..." IN BOOLEAN MODE)``
* ``SELECT * FROM table_name WHERE MBRContains(GeomFromText('LineString(...)'), geometry_column_name)``
* ``SELECT * FROM table_name WHERE NOT MATCH(column_name) AGAINST("..." IN BOOLEAN MODE)``
* ``SELECT * FROM table_name WHERE column_name < ...``
* ``SELECT * FROM table_name WHERE column_name <= ...``
* ``SELECT * FROM table_name WHERE column_name = ...``
* ``SELECT * FROM table_name WHERE column_name > ...``
* ``SELECT * FROM table_name WHERE column_name >= ...``
* ``SELECT * FROM table_name WHERE column_name BETWEEN ... AND ...``
* ``SELECT * FROM table_name WHERE column_name IN (SELECT sub_column_name FROM sub_table_name WHERE MATCH(sub_text_column_name) AGAINST("..."))``
* ``SELECT *, MATCH(column_name) AGAINST("..." IN BOOLEAN MODE) FROM table_name WHERE MATCH(column_name) AGAINST("..." IN BOOLEAN MODE)``
* ``SELECT FOUND_ROWS()``
* ``SELECT SQL_CALC_FOUND_ROWS * FROM table_name WHERE MATCH(...) AGAINST("..." IN BOOLEAN MODE) ORDER BY column_name LIMIT start,n_records``
* ``SELECT last_insert_grn_id()``
* ``SET binlog_format="MIXED"``
* ``SET binlog_format="ROW"``
* ``SET binlog_format="STATEMENT"``
* ``TRUNCATE TABLE table_name``
* ``UPDATE table_name SET column_name = ...``

List of unavailable SQL commands
--------------------------------

* ``CREATE TABLE (...) CAHRSET not_utf8``
* ``INSERT INTO (geometry_column_name) VALUES (GeomFromText('LineString(...)'))``
* ``INSERT INTO (...) VALUES (null)``
* ``START TRANSACTION``

I found an unlisted SQL command
-------------------------------

If you find unlisted SQL command and confirm whether the SQL command works well or not, we want to list it. Please mail it to `us <http://groonga.org/docs/community.html>`_ or send a patch against `the source file <https://github.com/mroonga/mroonga/blob/master/doc/source/reference.rst>`_ by pull request system on GitHub.

List of server variables
------------------------

Here are the explanations of server variables that are introduced by mroonga.

mroonga_default_parser
^^^^^^^^^^^^^^^^^^^^^^

The default parser of the full text search.
The default value can be specified by ``--with-default-parser=PARSER`` configure argument, whose default value is ``TokenBigram``.

Here is an example to use ``TokenBigramSplitSymbolAlphaDigit`` as a fulltext search parser. It is used by ``body_index`` fulltext index.

.. code-block:: sql
   :linenos:

   SET GLOBAL mroonga_default_parser=TokenBigramSplitSymbolAlphaDigit;
   CREATE TABLE diaries (
     id INT PRIMARY KEY AUTO_INCREMENT,
     body TEXT,
     FULLTEXT INDEX body_index (body)
   ) DEFAULT CHARSET UTF8;


mroonga_libgroonga_version
^^^^^^^^^^^^^^^^^^^^^^^^^^

The version string of the groonga library.

Here is an example SQL to confirm the using groonga library version::

  mysql> SHOW VARIABLES LIKE 'mroonga_libgroonga_version';
  +----------------------------+------------------+
  | Variable_name              | Value            |
  +----------------------------+------------------+
  | mroonga_libgroonga_version | 1.2.8-9-gbf05b82 |
  +----------------------------+------------------+
  1 row in set (0.00 sec)

mroonga_log_file
^^^^^^^^^^^^^^^^

The path of the log file of mroonga. The default value is ``groonga.log``.

Here is an example transcript to change log file to ``/tmp/mroonga.log``::

  mysql> SHOW VARIABLES LIKE 'mroonga_log_file';
  +------------------+-------------+
  | Variable_name    | Value       |
  +------------------+-------------+
  | mroonga_log_file | groonga.log |
  +------------------+-------------+
  1 row in set (0.00 sec)

  mysql> SET GLOBAL mroonga_log_file = "/tmp/mroonga.log";
  Query OK, 0 rows affected (0.00 sec)

  mysql> SHOW VARIABLES LIKE 'mroonga_log_file';
  +------------------+------------------+
  | Variable_name    | Value            |
  +------------------+------------------+
  | mroonga_log_file | /tmp/mroonga.log |
  +------------------+------------------+
  1 row in set (0.00 sec)

mroonga_log_level
^^^^^^^^^^^^^^^^^

The output level of mroonga log file. The default value is ``NOTICE``.

Here is an example transcript to change log level to ``DEBUG`` that logs many messages for debugging::

  mysql> SHOW VARIABLES LIKE 'mroonga_log_level';
  +-------------------+--------+
  | Variable_name     | Value  |
  +-------------------+--------+
  | mroonga_log_level | NOTICE |
  +-------------------+--------+
  1 row in set (0.00 sec)

  mysql> SET GLOBAL mroonga_log_level = "debug";
  Query OK, 0 rows affected (0.00 sec)

  mysql> SHOW VARIABLES LIKE 'mroonga_log_level';
  +-------------------+-------+
  | Variable_name     | Value |
  +-------------------+-------+
  | mroonga_log_level | DEBUG |
  +-------------------+-------+
  1 row in set (0.00 sec)

mroonga_version
^^^^^^^^^^^^^^^

The version string of mroonga.

Here is an example SQL to confirm the running mroonga version::

  mysql> SHOW VARIABLES LIKE 'mroonga_version';
  +-----------------+-------+
  | Variable_name   | Value |
  +-----------------+-------+
  | mroonga_version | 1.10  |
  +-----------------+-------+
  1 row in set (0.00 sec)

mroonga_dry_write
^^^^^^^^^^^^^^^^^

Whether really write data to groonga database or not. The
default value is ``OFF`` that means data are really written
to groonga database. Usually we don't need to change the
value of this variable. This variable is useful for
benchmark because we can measure processing time MySQL and
mroonga. It doesn't include groonga's processing time.

Here is an example SQL to disable writing data to groonga
database::

  mysql> SHOW VARIABLES LIKE 'mroonga_dry_write';
  +-------------------+-------+
  | Variable_name     | Value |
  +-------------------+-------+
  | mroonga_dry_write | OFF   |
  +-------------------+-------+
  1 row in set (0.00 sec)

  mysql> SET mroonga_dry_write = true;
  Query OK, 0 rows affected (0.00 sec)

  mysql> SHOW VARIABLES LIKE 'mroonga_dry_write';
  +-------------------+-------+
  | Variable_name     | Value |
  +-------------------+-------+
  | mroonga_dry_write | ON    |
  +-------------------+-------+
  1 row in set (0.00 sec)

mroonga_enable_optimization
^^^^^^^^^^^^^^^^^^^^^^^^^^^

Whether enable optimization or not. The default value is
``ON`` that means optimization is enabled. Usually we don't
need to change the value of this variable. This variable is
useful for benchmark.

Here is an example SQL to disable optimization::

  mysql> SHOW VARIABLES LIKE 'mroonga_enable_optimization';
  +-----------------------------+-------+
  | Variable_name               | Value |
  +-----------------------------+-------+
  | mroonga_enable_optimization | ON    |
  +-----------------------------+-------+
  1 row in set (0.00 sec)

  mysql> SET mroonga_enable_optimization = false;
  Query OK, 0 rows affected (0.00 sec)

  mysql> SHOW VARIABLES LIKE 'mroonga_enable_optimization';
  +-----------------------------+-------+
  | Variable_name               | Value |
  +-----------------------------+-------+
  | mroonga_enable_optimization | OFF   |
  +-----------------------------+-------+
  1 row in set (0.00 sec)

.. _mroonga_match_escalation_threshold:

mroonga_match_escalation_threshold
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The threshold to determin whether match method is escalated. See
`search specification for groonga
<http://groonga.org/docs/spec/search.html>`_ about match method
escalation.

The default value is the same as groonga's default value. It's 0 for
the default installation. The dafault value can be configured in
my.cnf or by ``SET GLOBAL mroonga_match_escalation_threshold =
THRESHOLD;``. Because this variable's scope is both global and
session.

Here is an example to use -1 as a threshold to determin whether match
method is escalated. -1 means that never escalated.

.. code-block:: sql
   :linenos:

   SET GLOBAL mroonga_match_escalation_threshold = -1;

Here is an another example to show behavior change by the variable
value.

.. code-block:: sql
   :linenos:

   CREATE TABLE diaries (
     id INT PRIMARY KEY AUTO_INCREMENT,
     title TEXT,
     tags TEXT,
     FULLTEXT INDEX tags_index (tags) COMMENT 'parser "TokenDelimit"'
   ) ENGINE=mroonga DEFAULT CHARSET=UTF8;

   -- Test data
   INSERT INTO diaries (title, tags) VALUES ("Hello groonga!", "groonga install");
   INSERT INTO diaries (title, tags) VALUES ("Hello mroonga!", "mroonga install");

   -- Matches all records that have "install" tag.
   SELECT * FROM diaries WHERE MATCH (tags) AGAINST ("install" IN BOOLEAN MODE);
   -- id	title	tags
   -- 1	Hello groonga!	groonga install
   -- 2	Hello mroonga!	mroonga install

   -- Matches no records by "gr" tag search because no "gr" tag is used.
   -- But matches a record that has "groonga" tag because search
   -- method is escalated and prefix search with "gr" is used.
   -- The default threshold is 0. It means that no records are matched then
   -- search method is escalated.
   SELECT * FROM diaries WHERE MATCH (tags) AGAINST ("gr" IN BOOLEAN MODE);
   -- id	title	tags
   -- 1	Hello groonga!	groonga install

   -- Disables escalation.
   SET mroonga_match_escalation_threshold = -1;
   -- No records are matched.
   SELECT * FROM diaries WHERE MATCH (tags) AGAINST ("gr" IN BOOLEAN MODE);
   -- id	title	tags

   -- Enables escalation again.
   SET mroonga_match_escalation_threshold = 0;
   -- Matches a record by prefix search with "gr".
   SELECT * FROM diaries WHERE MATCH (tags) AGAINST ("gr" IN BOOLEAN MODE);
   -- id	title	tags
   -- 1	Hello groonga!	groonga install

List of status variables
------------------------

Here are the explanations of status variables that are introduced by mroonga.

mroonga_count_skip
^^^^^^^^^^^^^^^^^^

This value is increased when 'fast line count feature' is used.
You can use this value to check if the feature is working when you enable it.

Here is an example how to check it::

  mysql> SHOW STATUS LIKE 'mroonga_count_skip';
  +--------------------+-------+
  | Variable_name      | Value |
  +--------------------+-------+
  | mroonga_count_skip | 0     |
  +--------------------+-------+
  1 row in set (0.00 sec)

mroonga_fast_order_limit
^^^^^^^^^^^^^^^^^^^^^^^^

This value is increased when 'fast ORDER BY LIMIT feature' is used.
You can use this value to check if the feature is working when you enable it.

Here is an example how to check it::

  mysql> SHOW STATUS LIKE 'mroonga_fast_order_limit';
  +--------------------------+-------+
  | Variable_name            | Value |
  +--------------------------+-------+
  | mroonga_fast_order_limit | 0     |
  +--------------------------+-------+
  1 row in set (0.00 sec)

Limitations
-----------

There are some limitations in mroonga storage engine.

Limitations about the value of columns
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

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
  
