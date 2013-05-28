.. highlightlang:: none

SQL commands
============

This section describes avaiable SQL commands and unavailable SQL commands.

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
* ``CREATE TABLE IF NOT EXISTS table_name (...)``
* ``CREATE TABLE \`table-name-with-hyphen\` (...)``
* ``CREATE TABLE table_name (... FULLTEXT INDEX index_name (column_name) COMMENT 'parser "TokenMecab"')``
* ``CREATE TABLE table_name (... FULLTEXT INDEX index_name (column_name) COMMENT 'normalizer "NormalizerMySQLUnicodeCIExceptKanaCIKanaWithVoicedSoundMark"')``
* ``CREATE TABLE table_name (... FULLTEXT INDEX index_name (column_name) COMMENT 'parser "TokenDelimit", normalizer "NormalizerMySQLUnicodeCIExceptKanaCIKanaWithVoicedSoundMark"')``
* ``CREATE TABLE table_name (... FULLTEXT INDEX index_name (column_name))``
* ``CREATE TABLE table_name (... FULLTEXT INDEX index_name (column_name, ...))``
* ``CREATE TABLE table_name (... INDEX (column_name))``
* ``CREATE TABLE table_name (... INDEX (column_name, ...))``
* ``CREATE TABLE table_name (... INDEX USING BTREE (column_name))``
* ``CREATE TABLE table_name (... PRIMARY KEY (column_name))``
* ``CREATE TABLE table_name (...) CHARSET ASCII``
* ``CREATE TABLE table_name (...) CHARSET BINARY``
* ``CREATE TABLE table_name (...) CHARSET CP932``
* ``CREATE TABLE table_name (...) CHARSET EUCJPMS``
* ``CREATE TABLE table_name (...) CHARSET KOI8R``
* ``CREATE TABLE table_name (...) CHARSET LATIN1``
* ``CREATE TABLE table_name (...) CHARSET SJIS``
* ``CREATE TABLE table_name (...) CHARSET UJIS``
* ``CREATE TABLE table_name (...) CHARSET UTF8``
* ``CREATE TABLE table_name (...) CHARSET UTF8MB4``
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
* ``SELECT * FROM table_name WHERE MATCH(column_name) AGAINST("*W word ..." IN BOOLEAN MODE)``
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

* ``CREATE TABLE (...) CHARSET not_listed_charset_above``
* ``INSERT INTO (geometry_column_name) VALUES (GeomFromText('LineString(...)'))``
* ``INSERT INTO (...) VALUES (null)``
* ``START TRANSACTION``

Basically, The character set which groonga supports (EUC-JP/UTF-8/SJIS/LATIN1/KOI8R) is also supported in mroonga.

I found an unlisted SQL command
-------------------------------

If you find unlisted SQL command and confirm whether the SQL command works well or not, we want to list it. Please mail it to `us <http://groonga.org/docs/community.html>`_ or send a patch against `the source file <https://github.com/mroonga/mroonga/blob/master/doc/source/reference.rst>`_ by pull request system on GitHub.
