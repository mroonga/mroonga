.. highlightlang:: none

Reference
=========

List of available SQL commands
------------------------------

You can see many SQL examples in the following directory of mroonga's source tree. ::

  test/sql/suite/groonga_storage/t
  test/sql/suite/groonga_wrapper/t

All SQL statements written there are currently available ones.

(Or we can say that any SQL statements that are not written there are not supported.)

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

Here is an example SQL to confirm the using groonga version::

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
