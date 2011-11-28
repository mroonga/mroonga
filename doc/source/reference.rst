.. highlightlang:: none

Reference
=========

List of available SQL commands
------------------------------

* ``ALTER TABLE table_name ADD COLUMN column_name TEXT``
* ``ALTER TABLE table_name ADD FULLTEXT INDEX index_name (column_name)``
* ``ALTER TABLE table_name ADD SPATIAL KEY index_name (geometry_column_name)``
* ``ALTER TABLE table_name ENGINE = groonga``
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
* ``CREATE TABLE table_name (...) ENGINE=groonga COMMENT = 'ENGINE "InnoDB"'``
* ``CREATE TABLE table_name (...) ENGINE=groonga``
* ``CREATE TABLE table_name (\`_id\` INT)``
* ``CREATE TABLE table_name (\`_id\` INT, KEY(_id) USING HASH)``
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
* ``CREATE TABLE table_name (column_name INT PRIMARY KEY AUTO_INCREMENT)``
* ``CREATE TABLE table_name (column_name INT UNSIGNED)``
* ``CREATE TABLE table_name (column_name INT)``
* ``CREATE TABLE table_name (column_name LONGBLOB)``
* ``CREATE TABLE table_name (column_name LONGTEXT)``
* ``CREATE TABLE table_name (column_name MEDIUMBLOB)``
* ``CREATE TABLE table_name (column_name MEDIUMINT)``
* ``CREATE TABLE table_name (column_name MEDIUMTEXT)``
* ``CREATE TABLE table_name (column_name SET(...))``
* ``CREATE TABLE table_name (column_name SMALLINT)``
* ``CREATE TABLE table_name (column_name TEXT)``
* ``CREATE TABLE table_name (column_name TIME)``
* ``CREATE TABLE table_name (column_name TIMESTAMP)``
* ``CREATE TABLE table_name (column_name TINYBLOB)``
* ``CREATE TABLE table_name (column_name TINYINT)``
* ``CREATE TABLE table_name (column_name TINYTEXT)``
* ``CREATE TABLE table_name (column_name VARBINARY(...))``
* ``CREATE TABLE table_name (column_name VARCHAR(...))``
* ``CREATE TABLE table_name (column_name YEAR)``
* ``DELETE FROM table_name WHERE ...``
* ``DROP INDEX column_name ON table_name``
* ``DROP TABLE IF EXISTS table_name, ...``
* ``DROP TABLE \`table-name-with-hyphen\`, ...``
* ``DROP TABLE table_name, ...``
* ``FLUSH LOGS``
* ``FLUSH TABLES``
* ``INSERT INTO (geometry_column_name) VALUES (GeomFromText('POINT(...)'))``
* ``INSERT INTO ... VALUES ...``
* ``INSERT INTO table_name (column_name, ...) SELECT ... FROM other_table_name``
* ``REPLACE INTO table_name SELECT ... FROM other_table_name ...``
* ``SELECT * FROM information_schema.plugins WHERE plugin_name = "groonga"``
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
* ``SELECT * FROM table_name``
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

groonga_default_parser
^^^^^^^^^^^^^^^^^^^^^^

The default parser of the full text search.
The default value can be specified by ``--with-default-parser=PARSER`` configure argument, whose default value is ``TokenBigram``.

Here is an example to use ``TokenBigramSplitSymbolAlphaDigit`` as a fulltext search parser. It is used by ``body_index`` fulltext index.

.. code-block:: sql
   :linenos:

   SET GLOBAL groonga_default_parser=TokenBigramSplitSymbolAlphaDigit;
   CREATE TABLE diaries (
     id INT PRIMARY KEY AUTO_INCREMENT,
     body TEXT,
     FULLTEXT INDEX body_index (body)
   ) DEFAULT CHARSET UTF8;


groonga_libgroonga_version
^^^^^^^^^^^^^^^^^^^^^^^^^^

The version string of the groonga library.

Here is an example SQL to confirm the using groonga library version::

  mysql> SHOW VARIABLES LIKE 'groonga_libgroonga_version';
  +----------------------------+------------------+
  | Variable_name              | Value            |
  +----------------------------+------------------+
  | groonga_libgroonga_version | 1.2.8-9-gbf05b82 |
  +----------------------------+------------------+
  1 row in set (0.00 sec)

groonga_log_file
^^^^^^^^^^^^^^^^

The path of the log file of mroonga. The default value is ``groonga.log``.

Here is an example transcript to change log file to ``/tmp/mroonga.log``::

  mysql> SHOW VARIABLES LIKE 'groonga_log_file';
  +------------------+-------------+
  | Variable_name    | Value       |
  +------------------+-------------+
  | groonga_log_file | groonga.log |
  +------------------+-------------+
  1 row in set (0.00 sec)

  mysql> SET GLOBAL groonga_log_file = "/tmp/mroonga.log";
  Query OK, 0 rows affected (0.00 sec)

  mysql> SHOW VARIABLES LIKE 'groonga_log_file';
  +------------------+------------------+
  | Variable_name    | Value            |
  +------------------+------------------+
  | groonga_log_file | /tmp/mroonga.log |
  +------------------+------------------+
  1 row in set (0.00 sec)


groonga_log_level
^^^^^^^^^^^^^^^^^

The output level of mroonga log file. The default value is ``NOTICE``.

Here is an example transcript to change log level to ``DEBUG`` that logs many messages for debugging::

  mysql> SHOW VARIABLES LIKE 'groonga_log_level';
  +-------------------+--------+
  | Variable_name     | Value  |
  +-------------------+--------+
  | groonga_log_level | NOTICE |
  +-------------------+--------+
  1 row in set (0.00 sec)

  mysql> SET GLOBAL groonga_log_level = "debug";
  Query OK, 0 rows affected (0.00 sec)

  mysql> SHOW VARIABLES LIKE 'groonga_log_level';
  +-------------------+-------+
  | Variable_name     | Value |
  +-------------------+-------+
  | groonga_log_level | DEBUG |
  +-------------------+-------+
  1 row in set (0.00 sec)

groonga_version
^^^^^^^^^^^^^^^

The version string of mroonga.

Here is an example SQL to confirm the running mroonga version::

  mysql> SHOW VARIABLES LIKE 'groonga_version';
  +-----------------+-------+
  | Variable_name   | Value |
  +-----------------+-------+
  | groonga_version | 1.10  |
  +-----------------+-------+
  1 row in set (0.00 sec)

List of status variables
------------------------

Here are the explanations of status variables that are introduced by mroonga.

groonga_count_skip
^^^^^^^^^^^^^^^^^^

This value is increased when 'fast line count feature' is used.
You can use this value to check if the feature is working when you enable it.

Here is an example how to check it::

  mysql> SHOW STATUS LIKE 'groonga_count_skip';
  +--------------------+-------+
  | Variable_name      | Value |
  +--------------------+-------+
  | groonga_count_skip | 0     |
  +--------------------+-------+
  1 row in set (0.00 sec)

groonga_fast_order_limit
^^^^^^^^^^^^^^^^^^^^^^^^

This value is increased when 'fast ORDER BY LIMIT feature' is used.
You can use this value to check if the feature is working when you enable it.

Here is an example how to check it::

  mysql> SHOW STATUS LIKE 'groonga_fast_order_limit';
  +--------------------------+-------+
  | Variable_name            | Value |
  +--------------------------+-------+
  | groonga_fast_order_limit | 0     |
  +--------------------------+-------+
  1 row in set (0.00 sec)
