.. highlightlang:: none

Optimizations
=============

Mroonga implemented some optimizations to return response faster.

Some optimizations can work only on :doc:`/tutorial/storage`.

.. _optimization-fetching-only-needed-columns:

Fetching only needed columns
----------------------------

This optimization can work only on :doc:`/tutorial/storage`.

Groonga uses column store architecture. It means that Groonga doesn't
need to fetch values of all columns for fetching a column value in a
row. Groonga can fetch values of only needed columns.

InnoDB and MyISAM use row store architecture. They need to fetch
values of all columns for fetching a column value in a row.

If you specify only columns you needed in ``SELECT``, Mroonga just
fetches only values of these columns. Mroonga doesn't fetch values of
other columns.

Mroonga can work faster by reducing operations and I/O.

It's this optimization.

Here is a sample table definition to describe this optimization::

  CREATE TABLE t1 (
    c1 INT PRIMARY KEY AUTO_INCREMENT,
    c2 INT,
    c3 INT,
    ...
    c11 VARCHAR(20),
    c12 VARCHAR(20),
    ...
    c20 DATETIME
  ) ENGINE=Mroonga DEFAULT CHARSET=utf8;

Here is a ``SELECT`` to describe this optimization::

  SELECT c1, c2, c11 FROM t1 WHERE c2 = XX AND c12 = "XXX";

In this case, Mroonga fetches values from only ``c1``, ``c2``, ``c11``
and ``c12``. Mroonga doesn't fetch values from ``c3``, ``c4``, ...,
``c10``, ``c13``, ..., ``c19`` and ``c20``.

.. _optimization-raw-count:

Row count
---------

This optimization can work only on :doc:`/tutorial/storage`.

MySQL requires all column values from storage engine for processing
``COUNT(*)`` even if ``COUNT(*)`` doesn't need them.

Mroonga doesn't fetch any column values for the case.

Mroonga can work faster by reducing operations and I/O.

It's this optimization.

Here is a ``SELECT`` to describe this optimization::

  SELECT COUNT(*) FROM t1 WHERE MATCH(c2) AGAINST("+keyword" IN BOOLEAN MODE);

The ``SELECT`` fetches only ``COUNT(*)`` and condition in ``WHERE``
can be processed only by index. In this case, Mroonga uses this
optimization.

You can confirm whether this optimization is used or not by looking
:ref:`status-variable-mroonga-count-skip` status variable::

  mysql> SHOW STATUS LIKE 'Mroonga_count_skip';
  +--------------------+-------+
  | Variable_name      | Value |
  +--------------------+-------+
  | Mroonga_count_skip | 1     |
  +--------------------+-------+
  1 row in set (0.00 sec)

:ref:`status-variable-mroonga-count-skip` status variable is
incremented when Mroonga uses this optimization.

You can disable this optimization by setting
:ref:`server-variable-mroonga-enable-optimization` to ``false``.

.. _optimization-order-by-limit:

``ORDER BY LIMIT``
------------------

This optimization can work on both :doc:`/tutorial/storage` and
:doc:`/tutorial/wrapper`.

MySQL can process ``ORDER BY`` and ``LIMIT`` with low cost if you can
get sorted records by index even if the number of matched records is
very big.

MySQL can do the process for ``MATCH() AGAINST(IN NATURAL LANGUAGE
MODE)``. But MySQL can't do the process for ``MATCH() AGAINST(IN
BOOLEAN MODE)``.

It means that MySQL might take long time for ``MATCH() AGAINST(IN
BOOLEAN MODE)`` that matches with many records.

Mroonga processes ``ORDER BY`` and ``LIMIT`` by Groonga and returns
only target records to MySQL. It's very faster for query that matches with
many records.

It's this optimization.

Here is a ``SELECT`` to describe this optimization::

  SELECT *
    FROM t1
   WHERE MATCH(c2) AGAINST("+keyword" IN BOOLEAN MODE)
   ORDER BY c1 LIMIT 1;

The ``SELECT`` runs full text search and sorts by Groonga and returns
only one record to MySQL.

You can confirm whether this optimization is used or not by looking
:ref:`status-variable-mroonga-fast-order-limit` status variable::

  mysql> SHOW STATUS LIKE 'Mroonga_fast_order_limit';
  +--------------------------+-------+
  | Variable_name            | Value |
  +--------------------------+-------+
  | Mroonga_fast_order_limit | 1     |
  +--------------------------+-------+
  1 row in set (0.00 sec)

:ref:`status-variable-mroonga-fast-order-limit` status variable is
incremented when Mroonga uses this optimization.

This optimization is used only when all the following conditions are
true:

* :doc:`/tutorial/storage`: ``WHERE`` clause has one ``MATCH AGAINST``
  and zero or more arithmetic operations such as ``column < 100``.
* :doc:`/tutorial/wrapper`: ``WHERE`` clause has only ``MATCH AGAINST``.
* No ``JOIN``
* No ``GROUP BY``
* No ``SQL_CALC_FOUND_ROWS``
* With ``LIMIT``
* :doc:`/tutorial/storage`: ``ORDER BY`` clause has only columns or
  ``MATCH AGAINST`` that is used in ``WHERE`` clause.
* :doc:`/tutorial/wrapper`: ``ORDER BY`` clause has only primary
  column or ``MATCH AGAINST`` that is used in ``WHERE`` clause.
