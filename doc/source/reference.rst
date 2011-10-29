.. highlightlang:: none

Reference
=========

List of available SQL commands
------------------------------

You can see many SQL examples in the following directory of groonga storage engine's source tree. ::

 test/sql/t

List of files ::

  auto_increment.test        insert.test
  binlog.test                insert_wrapper.test
  btree.test                 last_insert_grn_id.test
  count_performance.test     log_level.test
  create_table.test          order_limit_performance.test
  create_table_wrapper.test  replace.test
  delete.test                select_all.test
  delete_wrapper.test        select_pkey.test
  drop_database.test         select_secondary_key.test
  drop_table.test            show_create_table.test
  flush_logs.test            show_table_status.test
  fulltext.test              tinyint.test
  fulltext_wrapper.test      update.test
  hash.test                  update_wrapper.test
  information_schema.test

All SQL statements written there are currently available ones.

(Or we can say that any SQL statements that are not written there are not supported.)

List of server variables
------------------------

No server variable is added for now.

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
