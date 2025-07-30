Wrapper mode
============

Here we explain how to use wrapper mode of Mroonga

.. versionchanged:: 12.02

   Dropped support wrapper mode with MySQL 8.0 or later.

How to use wrapper mode
-----------------------

In wrapper mode, Mroonga works in wrapping an existing storage engine. To specify the wrapped storage engine, we use SQL comment like ``COMMENT = 'engine "InnoDB"'`` for now.

.. note::

   For now, a primary key is mandatory in wrapper mode. That is not the case with storage mode.

.. note::

   Wrapper mode supports the followings, that are not supported in storage mode for now.

   * null value
   * transaction (if storage engine supports. Note that rollback causes mismatch of indexes, it may affects search results, so recreate index of Mroonga in such a case.)

How to use full text search
---------------------------

After confirming the installation, let's create a table. The important point is to specify Mroonga by ``ENGINE = Mroonga``. ::

  mysql> CREATE TABLE diaries (
      ->   id INT PRIMARY KEY AUTO_INCREMENT,
      ->   content VARCHAR(255),
      ->   FULLTEXT INDEX (content)
      -> ) ENGINE = Mroonga COMMENT = 'engine "InnoDB"' DEFAULT CHARSET utf8;
  Query OK, 0 rows affected (0.52 sec)

We put data by INSERT. ::

  mysql> INSERT INTO diaries (content) VALUES ("It'll be fine tomorrow.");
  Query OK, 1 row affected (0.01 sec)

  mysql> INSERT INTO diaries (content) VALUES ("It'll rain tomorrow");
  Query OK, 1 row affected (0.00 sec)

Try full text search. ::

  mysql> SELECT * FROM diaries WHERE MATCH(content) AGAINST("+fine" IN BOOLEAN MODE);
  +----+-----------------------------------------+
  | id | content                                 |
  +----+-----------------------------------------+
  |  1 | It'll be fine tomorrow. |
  +----+-----------------------------------------+
  1 row in set (0.00 sec)

Yes, full text search works.

How to get search score
-----------------------

We often want to display more relevant results first in full text search. We use search score in such case.

We can get search score by MySQL's standard way [#score]_, i.e. we use MATCH...AGAINST in one of columns in SELECT or ORDER BY.

Let's try. ::

  mysql> INSERT INTO diaries (content) VALUES ("It's fine today. It'll be fine tomorrow as well.");
  Query OK, 1 row affected (0.00 sec)

  mysql> INSERT INTO diaries (content) VALUES ("It's fine today. But it'll rain tomorrow.");
  Query OK, 1 row affected (0.00 sec)

  mysql> SELECT *, MATCH (content) AGAINST ("+fine" IN BOOLEAN MODE) FROM diaries WHERE MATCH (content) AGAINST ("+fine" IN BOOLEAN MODE) ORDER BY MATCH (content) AGAINST ("+fine" IN BOOLEAN MODE) DESC;
  +----+--------------------------------------------------+---------------------------------------------------+
  | id | content                                          | MATCH (content) AGAINST ("+fine" IN BOOLEAN MODE) |
  +----+--------------------------------------------------+---------------------------------------------------+
  |  3 | It's fine today. It'll be fine tomorrow as well. |                                                 2 |
  |  1 | It'll be fine tomorrow.                          |                                                 1 |
  |  4 | It's fine today. But it'll rain tomorrow.        |                                                 1 |
  +----+--------------------------------------------------+---------------------------------------------------+
  3 rows in set (0.00 sec)

The result having the search word ``fine`` more, i.e. ``id = 3`` message having the higher search score, is displayed first. And you also get search score by using MATCH AGAINST in SELECT phrase.

You can use ``AS`` to change the attribute name. ::

  mysql> SELECT *, MATCH (content) AGAINST ("+fine" IN BOOLEAN MODE) AS score FROM diaries WHERE MATCH (content) AGAINST ("+fine" IN BOOLEAN MODE) ORDER BY MATCH (content) AGAINST ("+fine" IN BOOLEAN MODE) DESC;
  +----+--------------------------------------------------+-------+
  | id | content                                          | score |
  +----+--------------------------------------------------+-------+
  |  3 | It's fine today. It'll be fine tomorrow as well. |     2 |
  |  1 | It'll be fine tomorrow.                          |     1 |
  |  4 | It's fine today. But it'll rain tomorrow.        |     1 |
  +----+--------------------------------------------------+-------+
  3 rows in set (0.00 sec)

How to specify the parser for full text search
----------------------------------------------

MySQL has the following syntax to specify the parser [#parser]_ for full text search. ::

  FULLTEXT INDEX (content) WITH PARSER parser_name

To use this syntax, you need to register all parsers in MySQL beforehand. On the other hand, Groonga can dynamically add a tokenizer, that is a parser in MySQL. So if use this syntax in Mroonga, tokenizers that are added in Groonga dynamically cannot be supported. We think that this limitation decreases the convenience, and we choose our own syntax using COMMENT like the following. ::

  FULLTEXT INDEX (content) COMMENT 'tokenizer "TokenMecab"'

.. note::

   ``COMMENT`` in ``FULLTEXT INDEX`` is only supported MySQL 5.5 or later. If you use MySQL 5.1, use ``mroonga_default_parser`` variable described below.

You can specify one of following values as the tokenizer.

.. include:: tokenizer-list.inc


You can specify the default tokenizer by passing ``--with-default-tokenizer`` option in ``configure`` when you build Mroonga. ::

  ./configure --with-default-tokenizer TokenMecab ...

Or you can set ``mroonga_default_tokenizer`` variable in my.cnf or by SQL. If you specify it in my.cnf, the change will not be lost after restarting MySQL, but you need to restart MySQL to make it effective. On the other hand, if you set it in SQL, the change is effective immediately, but it will be lost when you restart MySQL.

my.cnf::

  [mysqld]
  mroonga_default_tokenizer=TokenMecab

SQL::

  mysql> SET GLOBAL mroonga_default_tokenizer = TokenMecab;
  Query OK, 0 rows affected (0.00 sec)

How to specify the normalizer
-----------------------------

Mroonga uses normalizer corresponding to the encoding of document.
It is used when tokenizing text and storing table key.

It is used ``NormalizerMySQLGeneralCI`` normalizer when the encoding is
``utf8_general_ci`` or ``utf8mb4_general_ci``.

It is used ``NormalizerMySQLUnicodeCI`` normalizer when the encoding is
``utf8_unicode_ci`` or ``utf8mb4_unicode_ci``.

It isn't used normalizer when the encoding is ``utf8_bin``.

Here is an example that uses ``NormalizerMySQLUnicodeCI`` normalizer by specifying ``utf8_unicode_ci``.::

  mysql> SET NAMES utf8;
  Query OK, 0 rows affected (0.00 sec)

  mysql> CREATE TABLE diaries (
      ->   day DATE PRIMARY KEY,
      ->   content VARCHAR(64) NOT NULL,
      ->   FULLTEXT INDEX (content)
      -> ) Engine=Mroonga DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
  Query OK, 0 rows affected (0.18 sec)

  mysql> INSERT INTO diaries VALUES ("2013-04-23", "ブラックコーヒーを飲んだ。");
  Query OK, 1 row affected (0.00 sec)

  mysql> SELECT * FROM diaries
      ->        WHERE MATCH (content) AGAINST ("+ふらつく" IN BOOLEAN MODE);
  +------------+-----------------------------------------+
  | day        | content                                 |
  +------------+-----------------------------------------+
  | 2013-04-23 | ブラックコーヒーを飲んだ。 |
  +------------+-----------------------------------------+
  1 row in set (0.00 sec)

  mysql> SELECT * FROM diaries
      ->        WHERE MATCH (content) AGAINST ("+ﾌﾞﾗｯｸ" IN BOOLEAN MODE);
  +------------+-----------------------------------------+
  | day        | content                                 |
  +------------+-----------------------------------------+
  | 2013-04-23 | ブラックコーヒーを飲んだ。 |
  +------------+-----------------------------------------+
  1 row in set (0.00 sec)

Mroonga has the following syntax to specify Groonga's normalizer::

  FULLTEXT INDEX (content) COMMENT 'normalizer "NormalizerAuto"'

See `Groonga's documentation about normalizer <https://groonga.org/docs/reference/normalizers.html>`_ for more details.

Here is an example that uses ``NormalizerAuto`` normalizer::

  mysql> SET NAMES utf8;
  Query OK, 0 rows affected (0.00 sec)

  mysql> CREATE TABLE diaries (
      ->   day DATE PRIMARY KEY,
      ->   content VARCHAR(64) NOT NULL,
      ->   FULLTEXT INDEX (content) COMMENT 'normalizer "NormalizerAuto"'
      -> ) Engine=Mroonga DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
  Query OK, 0 rows affected (0.19 sec)

  mysql> INSERT INTO diaries VALUES ("2013-04-23", "ブラックコーヒーを飲んだ。");
  Query OK, 1 row affected (0.00 sec)

  mysql> SELECT * FROM diaries
      ->        WHERE MATCH (content) AGAINST ("+ふらつく" IN BOOLEAN MODE);
  Empty set (0.00 sec)

  mysql> SELECT * FROM diaries
      ->        WHERE MATCH (content) AGAINST ("+ﾌﾞﾗｯｸ" IN BOOLEAN MODE);
  +------------+-----------------------------------------+
  | day        | content                                 |
  +------------+-----------------------------------------+
  | 2013-04-23 | ブラックコーヒーを飲んだ。 |
  +------------+-----------------------------------------+
  1 row in set (0.00 sec)

How to get snippet (Keyword in context)
---------------------------------------

Mroonga provides functionality to get keyword in context.
It is implemented as :doc:`/reference/udf/mroonga_snippet` UDF.

Logging
-------

Mroonga outputs the logs by default.

Log files are located in MySQL's data directory with the filename  ``groonga.log``.

Here is the example of the log. ::

  2010-10-07 17:32:39.209379|n|b1858f80|mroonga 1.10 started.
  2010-10-07 17:32:44.934048|d|46953940|hash get not found (key=test)
  2010-10-07 17:32:44.936113|d|46953940|hash put (key=test)

The default log level is NOTICE, i.e. we have important information only and we don't have debug information etc.).

You can get the log level by ``mroonga_log_level`` system variable, that is a global variable. You can also modify it dynamically by using SET phrase. ::

  mysql> SHOW VARIABLES LIKE 'mroonga_log_level';
  +-------------------+--------+
  | Variable_name     | Value  |
  +-------------------+--------+
  | mroonga_log_level | NOTICE |
  +-------------------+--------+
  1 row in set (0.00 sec)

  mysql> SET GLOBAL mroonga_log_level=DUMP;
  Query OK, 0 rows affected (0.00 sec)

  mysql> SHOW VARIABLES LIKE 'mroonga_log_level';
  +-------------------+-------+
  | Variable_name     | Value |
  +-------------------+-------+
  | mroonga_log_level | DUMP  |
  +-------------------+-------+
  1 row in set (0.00 sec)

Available log levels are the followings.

* NONE
* EMERG
* ALERT
* CRIT
* ERROR
* WARNING
* NOTICE
* INFO
* DEBUG
* DUMP

See :ref:`server-variable-mroonga-log-level` about details.

You can reopen the log file by FLUSH LOGS. If you want to rotate the log file without stopping MySQL server, you can do in the following procedure.

1. change the file name of ``groonga.log`` (by using OS's mv command etc.).
2. invoke "FLUSH LOGS" in MySQL server (by mysql command or mysqladmin command).

Next step
---------

Now, you can use Mroonga as wrapper mode! If you want Mroonga to be
faster, see also :doc:`/reference/optimizations`.

.. rubric:: Footnotes

.. [#score] `MySQL 5.1 Reference Manual :: 11 Functions and Operations :: 11.7 Full-Text Search Functions <http://dev.mysql.com/doc/refman/5.1/ja/fulltext-search.html>`_
.. [#parser] In Groonga, we call it a 'tokenizer'.
