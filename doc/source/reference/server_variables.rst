.. highlightlang:: none

Server variables
================

Here are the explanations of server variables that are introduced by Mroonga.

.. _server-variable-mroonga-action-on-fulltext-query-error:

``mroonga_action_on_fulltext_query_error``
------------------------------------------

The default behavior of fulltext query error.

The default value of ``mroonga_action_on_fulltext_query_error`` is ``ERROR_AND_LOG``.
This is the conventional behavior which is equal to old version of mroonga.

Here is the list of ``mroonga_action_on_fulltext_query_error`` which you can use.

.. list-table::
  :header-rows: 1

  * - Value
    - Description
  * - ERROR
    - Report an error. Logging is disabled.
  * - ERROR_AND_LOG
    - Report an error. Logging is enabled. (This is the default)
  * - IGNORE
    - Just ignore an error. Logging is disabled.
  * - IGNORE_AND_LOG
    - Ignore an error, but logging is enabled. (Similar to InnoDB behavior)

Here is an example SQL to confirm the value of ``mroonga_action_on_fulltext_query_error``::

  mysql> SHOW VARIABLES LIKE 'mroonga_action_on_fulltext_query_error';
  +----------------------------------------+---------------+
  | Variable_name                          | Value         |
  +----------------------------------------+---------------+
  | mroonga_action_on_fulltext_query_error | ERROR_AND_LOG |
  +----------------------------------------+---------------+
  1 row in set (0.00 sec)

.. _server-variable-mroonga-boolean-mode-syntax-flags:

``mroonga_boolean_mode_syntax_flags``
-------------------------------------

The flags to custom syntax in ``MATCH () AGAINST ('...' IN BOOLEAN
MODE)``.

This variable is system and session variable.

Here are available flags:

.. list-table::
  :header-rows: 1

  * - Flag
    - Description
  * - ``DEFAULT``
    - Equals to ``SYNTAX_QUERY,ALLOW_LEADING_NOT``.
  * - ``SYNTAX_QUERY``
    - Uses `query syntax in Groonga
      <http://groonga.org/docs/reference/grn_expr/query_syntax.html>`_. Query
      syntax in Groonga is a compatible syntax with MySQL's BOOLEAM
      MODE syntax.

      If neither ``SYNTAX_QUERY`` nor ``SYNTAX_SCRIPT`` aren't
      specified, ``SYNTAX_QUERY`` is used.
  * - ``SYNTAX_SCRIPT``
    - Uses `script syntax in Groonga
      <http://groonga.org/docs/reference/grn_expr/script_syntax.html>`_.

      It's JavaScript like syntax. You can use full Groonga features
      with this syntax.

      If both ``SYNTAX_QUERY`` and ``SYNTAX_SCRIPT`` are specified,
      ``SYNTAX_SCRIPT`` is used.
  * - ``ALLOW_COLUMN``
    - Allows ``COLUMN:...`` syntax in query syntax. It's not
      compatible with MySQL's BOOLEAM MODE syntax.

      You can use multiple indexes in one ``MATCH () AGAINST ()`` with
      this syntax. MySQL can use only one index in a query. You can
      avoid the restriction by this syntax.

      You can use not only full-text search operation but also other
      more operations such as equal operation and prefix search
      operation with this syntax.

      See `query syntax in Groonga
      <http://groonga.org/docs/reference/grn_expr/query_syntax.html>`_
      for details.
  * - ``ALLOW_UPDATE``
    -  Allows updating value by ``COLUMN:=NEW_VALUE`` syntax
       in query syntax.
  * - ``ALLOW_LEADING_NOT``
    - Allows ``-NOT_INCLUDED_KEYWORD ...`` syntax in query syntax.

The default flags is ``DEFAULT``. It is MySQL's BOOLEAN MODE
compatible syntax.

You can combine flags by separated by comma such as
``SYNTAX_QUERY,ALLOW_LEADING_NOT``.

Here is an example SQL to use script syntax in Groonga::

  mysql> SET mroonga_boolean_mode_syntax_flags = "SYNTAX_SCRIPT";

.. _server-variable-mroonga-database-path-prefix:

``mroonga_database_path_prefix``
--------------------------------

TODO:

.. _server-variable-mroonga-default-parser:

``mroonga_default_parser``
--------------------------

.. deprecated:: 5.04
   Use :ref:`server-variable-mroonga-default-tokenizer` instead.

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

.. _server-variable-mroonga-default-tokenizer:

``mroonga_default_tokenizer``
-----------------------------

.. versionadded:: 5.04

The default tokenizer of the full text search.
The default value can be specified by ``--with-default-tokenizer=TOKENIZER`` configure argument, whose default value is ``TokenBigram``.

Here is an example to use ``TokenBigramSplitSymbolAlphaDigit`` as a fulltext index tokenizer. It is used by ``body_index`` fulltext index.

.. code-block:: sql
   :linenos:

   SET GLOBAL mroonga_default_tokenizer=TokenBigramSplitSymbolAlphaDigit;
   CREATE TABLE diaries (
     id INT PRIMARY KEY AUTO_INCREMENT,
     body TEXT,
     FULLTEXT INDEX body_index (body)
   ) DEFAULT CHARSET UTF8;


.. _server-variable-mroonga-default-wrapper-engine:

``mroonga_default_wrapper_engine``
----------------------------------

TODO:

.. _server-variable-mroonga-dry-write:

``mroonga_dry_write``
---------------------

Whether really write data to Groonga database or not. The
default value is ``OFF`` that means data are really written
to Groonga database. Usually we don't need to change the
value of this variable. This variable is useful for
benchmark because we can measure processing time MySQL and
Mroonga. It doesn't include Groonga's processing time.

Here is an example SQL to disable writing data to Groonga
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

.. _server-variable-mroonga-enable-optimization:

``mroonga_enable_optimization``
-------------------------------

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

.. _server-variable-mroonga-libgroonga-support-lz4:

``mroonga_libgroonga_support_lz4``
----------------------------------

The status of libgroonga supports LZ4.

Here is an example SQL to confirm the status of libgroonga supports LZ4::

  mysql> SHOW GLOBAL VARIABLES LIKE 'mroonga_libgroonga_support_lz4';
  +--------------------------------+-------+
  | Variable_name                  | Value |
  +--------------------------------+-------+
  | mroonga_libgroonga_support_lz4 | ON    |
  +--------------------------------+-------+

.. _server-variable-mroonga-libgroonga-support-zlib:

``mroonga_libgroonga_support_zlib``
-----------------------------------

The status of libgroonga supports zlib.

Here is an example SQL to confirm the status of libgroonga supports zlib::

  mysql> SHOW GLOBAL VARIABLES LIKE 'mroonga_libgroonga_support_zlib';
  +---------------------------------+-------+
  | Variable_name                   | Value |
  +---------------------------------+-------+
  | mroonga_libgroonga_support_zlib | ON    |
  +---------------------------------+-------+

.. _server-variable-mroonga-libgroonga-support-zstd:

``mroonga_libgroonga_support_zstd``
-----------------------------------

The status of libgroonga supports Zstandard.

Here is an example SQL to confirm the status of libgroonga supports Zstandard::

  mysql> SHOW GLOBAL VARIABLES LIKE 'mroonga_libgroonga_support_zstd';
  +---------------------------------+-------+
  | Variable_name                   | Value |
  +---------------------------------+-------+
  | mroonga_libgroonga_support_zstd | ON    |
  +---------------------------------+-------+

.. _serer-variable-mroonga-libgroonga-version:


``mroonga_libgroonga_version``
------------------------------

The version string of the groonga library.

Here is an example SQL to confirm the using groonga library version::

  mysql> SHOW VARIABLES LIKE 'mroonga_libgroonga_version';
  +----------------------------+------------------+
  | Variable_name              | Value            |
  +----------------------------+------------------+
  | mroonga_libgroonga_version | 1.2.8-9-gbf05b82 |
  +----------------------------+------------------+
  1 row in set (0.00 sec)

.. _server-variable-mroonga-lock-timeout:

``mroonga_lock_timeout``
------------------------

TODO:

.. _serer-variable-mroonga-log-file:

``mroonga_log_file``
--------------------

The path of the log file of Mroonga. The default value is ``groonga.log``.

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

.. _server-variable-mroonga-log-level:

``mroonga_log_level``
---------------------

The output level of Mroonga log file. The default value is ``NOTICE``.

Here is the list of ``mroonga_log_level`` which you can use.

.. list-table::
  :header-rows: 1

  * - Log level
    - Description
  * - ``NONE``
    - No logging output.
  * - ``EMERG``
    - Logging emergency messages such as database corruption.
  * - ``ALERT``
    - Logging alert messages such as internal error.
  * - ``CRIT``
    - Logging critical message such as deadlock.
  * - ``ERROR``
    - Logging error messages such as API error which Mroonga use.
  * - ``WARNING``
    - Logging warning messages such as invalid argument.
  * - ``NOTICE``
    - Logging notice messages such as configuration or status changed.
  * - ``INFO``
    - Logging informative messages such as file system operation.
  * - ``DEBUG``
    - Logging debug messages.

      Recommend to use for Mroonga developer or bug reporter.
  * - ``DUMP``
    - Logging dump messages.

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

.. _server-variable-mroonga-query-log-file:

``mroonga_query_log_file``
--------------------------

The path of the query log file of Mroonga. The default value is empty.

If this value is empty, the query log is not stored to file.
If this value is not empty, query log is stored to the specified file.

Here is an example transcript to change query log file to ``/tmp/mroonga_query.log``::

  mysql> SHOW VARIABLES LIKE 'mroonga_query_log_file';
  +------------------------+-------+
  | Variable_name          | Value |
  +------------------------+-------+
  | mroonga_query_log_file |       |
  +------------------------+-------+
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

.. _server-variable-mroonga-match-escalation-threshold:

``mroonga_match_escalation_threshold``
--------------------------------------

The threshold to determin whether match method is escalated. See
`search specification for Groonga
<http://groonga.org/docs/spec/search.html>`_ about match method
escalation.

The default value is the same as Groonga's default value. It's 0 for
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


.. _server-variable-mroonga-max-n-records-for-estimate:

``mroonga_max_n_records_for_estimate``
--------------------------------------

.. versionadded:: 5.02

TODO:

.. _server-variable-mroonga-enable-operations-recording:

``mroonga_enable_operations_recording``
---------------------------------------

Whether recording operations for recover is enabled or not.
The default value is ``ON`` that means operations are recording
to Groonga database.
It needs to reopen the database with ``FLUSH TABLES`` in order
to reflect the variable is changed.

Here is an example SQL to disable operations recording::


  mysql> SET GLOBAL mroonga_enable_operations_recording = false;
  Query OK, 0 rows affected (0.00 sec)

  mysql> FLUSH TABLES;
  Query OK, 0 rows affected (0.00 sec)

  mysql> SHOW GLOBAL VARIABLES LIKE 'mroonga_enable_operations_recording';
  +-------------------------------------+-------+
  | Variable_name                       | Value |
  +-------------------------------------+-------+
  | mroonga_enable_operations_recording | OFF   |
  +-------------------------------------+-------+

.. _server-variable-mroonga-vector-column-delimiter:

``mroonga_vector_column_delimiter``
-----------------------------------

The delimiter when outputting a vector column.  The default value is a white space.

Here is an example SQL to change the delimiter to a semicolon from a white space::


  mysql> SHOW VARIABLES LIKE 'mroonga_vector_column_delimiter';
  +---------------------------------+-------+
  | Variable_name                   | Value |
  +---------------------------------+-------+
  | mroonga_vector_column_delimiter |       |
  +---------------------------------+-------+
  1 row in set (0.00 sec)


  mysql> SET GLOBAL mroonga_vector_column_delimiter = ';';
  Query OK, 0 rows affected (0.00 sec)

  mysql> SHOW GLOBAL VARIABLES LIKE 'mroonga_vector_column_delimiter';
  +---------------------------------+-------+
  | Variable_name                   | Value |
  +---------------------------------+-------+
  | mroonga_vector_column_delimiter | ;     |
  +---------------------------------+-------+

.. _server-variable-mroonga-version:

``mroonga_version``
-------------------

The version string of Mroonga.

Here is an example SQL to confirm the running mroonga version::

  mysql> SHOW VARIABLES LIKE 'mroonga_version';
  +-----------------+-------+
  | Variable_name   | Value |
  +-----------------+-------+
  | mroonga_version | 1.10  |
  +-----------------+-------+
  1 row in set (0.00 sec)

.. _server-variable-mroonga-condition-push-down-type:

``mroonga_condition_push_down_type``
------------------------------------

.. versionadded:: 7.10

It controls how to enable condition push down support.

The default value is ``ONE_FULL_TEXT_SEARCH``. It means that condition
push down is enabled only when ``WHERE`` clause has one ``MATCH
AGAINST`` condition.

Here are available values:

.. list-table::
  :header-rows: 1

  * - Value
    - Description
  * - NONE
    - Never use condition push down.
  * - ALL
    - Always use condition push down. It's experimental for now.
  * - ONE_FULL_TEXT_SEARCH
    - Use condition push down only when one ``MATCH AGAINST``
      condition exists in ``WHERE``.

      It's the default.

Here is an example SQL to confirm the current value::

  mysql> SHOW VARIABLES LIKE 'mroonga_condition_push_down_type';
  +----------------------------------+----------------------+
  | Variable_name                    | Value                |
  +----------------------------------+----------------------+
  | mroonga_condition_push_down_type | ONE_FULL_TEXT_SEARCH |
  +----------------------------------+----------------------+
  1 row in set (0.00 sec)
