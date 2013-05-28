.. highlightlang:: none

Server variables
================

Here are the explanations of server variables that are introduced by mroonga.

mroonga_default_parser
----------------------

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
--------------------------

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
----------------

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

.. _mroonga_log_level:

mroonga_log_level
-----------------

The output level of mroonga log file. The default value is ``NOTICE``.

Here is the list of ``mroonga_log_level`` which you can use.

+-----------+--------------------------------------------------------------------------------+
| Log level | Description                                                                    |
+===========+================================================================================+
| NONE      | No logging output.                                                             |
+-----------+--------------------------------------------------------------------------------+
| EMERG     | Logging emergency messages such as database corruption.                        |
+-----------+--------------------------------------------------------------------------------+
| ALERT     | Logging alert messages such as internal error.                                 |
+-----------+--------------------------------------------------------------------------------+
| CRIT      | Logging critical messasge such as deadlock.                                    |
+-----------+--------------------------------------------------------------------------------+
| ERROR     | Logging error messages such as API error which mroonga use.                    |
+-----------+--------------------------------------------------------------------------------+
| WARNING   | Logging warning messages such as invalid argument.                             |
+-----------+--------------------------------------------------------------------------------+
| NOTICE    | Logging notice messages such as configuration or status changed.               |
+-----------+--------------------------------------------------------------------------------+
| INFO      | Logging informative messages such as file system operation.                    |
+-----------+--------------------------------------------------------------------------------+
| DEBUG     | Logging debug messages.                                                        |
|           | Recommend to use for mroonga developer or bug reporter.                        |
+-----------+--------------------------------------------------------------------------------+
| DUMP      | Logging dump messages.                                                         |
+-----------+--------------------------------------------------------------------------------+

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
---------------

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
-----------------

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
---------------------------

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
----------------------------------

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
