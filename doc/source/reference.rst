.. highlightlang:: none

Reference
=========

List of available SQL commands
------------------------------

You can see many SQL examples in the following directory of groonga storage engine's source tree. ::

  test/sql/suite/groonga_storage/t
  test/sql/suite/groonga_wrapper/t

All SQL statements written there are currently available ones.

(Or we can say that any SQL statements that are not written there are not supported.)

List of server variables
------------------------

Here are the explanations of server variables that are introduced by groonga storage engine.

groonga_default_parser
^^^^^^^^^^^^^^^^^^^^^^

The default parser of the full text search.
The default value can be specified by ``--with-default-parser=PARSER`` configure argument, whose default value is ``TokenBigram``.

groonga_libgroonga_version
^^^^^^^^^^^^^^^^^^^^^^^^^^

The version string of the groonga library.

groonga_log_file
^^^^^^^^^^^^^^^^

The path of the log file of the groonga storage engine. The default value is ``groonga.log``.

groonga_log_level
^^^^^^^^^^^^^^^^^

The log level of the groonga storage engine. The default value is ``NOTICE``.

groonga_version
^^^^^^^^^^^^^^^

The version string of the groonga storage engine.

List of status variables
------------------------

Here are the explanations of status variables that are introduced by groonga storage engine.

groonga_count_skip
^^^^^^^^^^^^^^^^^^

This value is increased when 'fast line count feature' is used.
You can use this value to check if the feature is working when you enable it.

groonga_fast_order_limit
^^^^^^^^^^^^^^^^^^^^^^^^

This value is increased when 'fast ORDER BY LIMIT feature' is used.
You can use this value to check if the feature is working when you enable it.
