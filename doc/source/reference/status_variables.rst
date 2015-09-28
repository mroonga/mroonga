.. highlightlang:: none

Status variables
================

Here are the explanations of status variables that are introduced by Mroonga.

.. _status-variable-mroonga-count-skip:

``Mroonga_count_skip``
----------------------

This value is increased when 'fast line count feature' is used.
You can use this value to check if the feature is working when you enable it.

Here is an example how to check it::

  mysql> SHOW STATUS LIKE 'Mroonga_count_skip';
  +--------------------+-------+
  | Variable_name      | Value |
  +--------------------+-------+
  | Mroonga_count_skip | 0     |
  +--------------------+-------+
  1 row in set (0.00 sec)

.. _status-variable-mroonga-fast-order-limit:

``Mroonga_fast_order_limit``
----------------------------

This value is increased when 'fast ORDER BY LIMIT feature' is used.
You can use this value to check if the feature is working when you enable it.

Here is an example how to check it::

  mysql> SHOW STATUS LIKE 'Mroonga_fast_order_limit';
  +--------------------------+-------+
  | Variable_name            | Value |
  +--------------------------+-------+
  | Mroonga_fast_order_limit | 0     |
  +--------------------------+-------+
  1 row in set (0.00 sec)

