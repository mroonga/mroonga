.. highlightlang:: none

Storage mode
============

Here we explain how to use storage mode of mroonga

How to use full text search
---------------------------

After confirming the installation, let's create a table. The important point is to specify mroonga by ``ENGINE = mroonga``. ::

  mysql> CREATE TABLE diaries (
      ->   id INT PRIMARY KEY AUTO_INCREMENT,
      ->   content VARCHAR(255),
      ->   FULLTEXT INDEX (content)
      -> ) ENGINE = mroonga DEFAULT CHARSET utf8;
  Query OK, 0 rows affected (0.10 sec)

We put data by INSERT. ::

  mysql> INSERT INTO diaries (content) VALUES ("明日の天気は晴れでしょう。");
  Query OK, 1 row affected (0.01 sec)

  mysql> INSERT INTO diaries (content) VALUES ("明日の天気は雨でしょう。");
  Query OK, 1 row affected (0.00 sec)

Try full text search. ::

  mysql> SELECT * FROM diaries WHERE MATCH(content) AGAINST("晴れ");
  +----+-----------------------------------------+
  | id | content                                 |
  +----+-----------------------------------------+
  |  1 | 明日の天気は晴れでしょう。 |
  +----+-----------------------------------------+
  1 row in set (0.00 sec)

Yes, full text search works.

How to get search score
-----------------------

.. note::

   In version 1.0.0 or before, mroonga used a special column named ``_score`` to get search score. From version 1.0.0, it follows MySQL's standard way to get search score.

We often want to display more relevant results first in full text search. We use search score in such case.

We can get search score by MySQL's standard way [#score]_, i.e. we use MATCH...AGAINST in one of columns in SELECT or ORDER BY.

Let's try. ::

  mysql> INSERT INTO diaries (content) VALUES ("今日は晴れました。明日も晴れるでしょう。");
  Query OK, 1 row affected (0.00 sec)

  mysql> INSERT INTO diaries (content) VALUES ("今日は晴れましたが、明日は雨でしょう。");
  Query OK, 1 row affected (0.00 sec)

  mysql> SELECT *, MATCH (content) AGAINST ("晴れ") FROM diaries WHERE MATCH (content) AGAINST ("晴れ") ORDER BY MATCH (content) AGAINST ("晴れ") DESC;
  +----+--------------------------------------------------------------+------------------------------------+
  | id | content                                                      | MATCH (content) AGAINST ("晴れ") |
  +----+--------------------------------------------------------------+------------------------------------+
  |  3 | 今日は晴れました。明日も晴れるでしょう。 |                                  2 |
  |  1 | 明日の天気は晴れでしょう。                      |                                  1 |
  |  4 | 今日は晴れましたが、明日は雨でしょう。    |                                  1 |
  +----+--------------------------------------------------------------+------------------------------------+
  3 rows in set (0.00 sec)

The result having the search word ``晴れ`` more, i.e. ``id = 3`` message having the higher search score, is displayed first. And you also get search score by using MATCH AGAINST in SELECT phrase.

You can use ``AS`` to change the attribute name. ::

  mysql> SELECT *, MATCH (content) AGAINST ("晴れ") AS score FROM diaries WHERE MATCH (content) AGAINST ("晴れ") ORDER BY MATCH (content) AGAINST ("晴れ") DESC;
  +----+--------------------------------------------------------------+-------+
  | id | content                                                      | score |
  +----+--------------------------------------------------------------+-------+
  |  3 | 今日は晴れました。明日も晴れるでしょう。 |     2 |
  |  1 | 明日の天気は晴れでしょう。                      |     1 |
  |  4 | 今日は晴れましたが、明日は雨でしょう。    |     1 |
  +----+--------------------------------------------------------------+-------+
  3 rows in set (0.00 sec)

How to specify the parser for full text search
----------------------------------------------

MySQL has the following syntax to specify the parser [#parser]_ for full text search. ::

  FULLTEXT INDEX (content) WITH PARSER parser_name

To use this syntax, you need to register all parsers in MySQL beforehand. On the other hand, groonga can dynamically add a tokeniser, that is a parser in MySQL. So if use this syntax in mroonga, tokenisers that are added in groonga dynamically cannot be supported. We think that this limitation decreases the convenience, and we choose our own syntax using COMMENT like the following. ::

  FULLTEXT INDEX (content) COMMENT 'parser "TokenMecab"'

.. note::

   ``COMMENT`` in ``FULLTEXT INDEX`` is only supported MySQL 5.5 or later. If you use MySQL 5.1, use ``mroonga_default_parser`` variable described below.

You can specify one of following values as the parser.

TokenBigram
  It tokenises in bigram. But continuous alphabets, numbers or symbols are treated as a token. So there can exist tokes with 3 letters or more. It is to reduce noises.

  This is the default value.

TokenMecab
  It tokenises using MeCab. Groonga should be built with MeCab support.

TokenBigramSplitSymbol
  It tokenises in bigram. Unlike TokenBigram, continuous symbols are not treated as a token, but tokenised in bigram.

  When you use TokenBigramSplitSymbol instead of TokenBigram, "!?" can match "!?!?!?" in "Is it really!?!?!?". But when you use TokenBigram, only "!?!?!?" can match as well.

TokenBigramSplitSymbolAlpha
  It tokenise in bigram. In addition to TokenBigramSplitSymbol, continuous alphabets are not treated as a token either, but tokenised in bigram.

  When you use TokenBigramSplitSymbolAlpha instead of TokenBigram, "real" can match "Is it really?". But when you use TokenBigram, only "really" can match as well.

TokenBigramSplitSymbolAlphaDigit
  It tokenise in bigram. In addition to TokenBigramSplitSymbolAlpha, continuous numbers are not treated as a token either, but tokenised in bigram. So any kind of characters are treated equally in bigram.

  When you use TokenBigramSplitSymbolAlphaDigit instead of TokenBigram, "567" can match "090-0123-4567". But when you use TokenBigram, only "4567" can match as well.

TokenBigramIgnoreBlank
  It tokenise in bigram. Unlike TokenBigram, it ignores white spaces.

  When you use TokenBigramIgnoreBlank instead of TokenBigram, "みなさん" can match "み な さ ん 注 目". But when you use TokenBigram, only "み な さ ん" can match as well.

TokenBigramIgnoreBlankSplitSymbol
  It tokenise in bigram. Unlike TokenBigramSplitSymbol, it ignores white spaces.

  When you use TokenBigramIgnoreBlankSplitSymbol instead of TokenBigramSplitSymbol, "???" can match "! ? ???". But when you use TokenBigramSplitSymbol, only "? ??" can match as well.

TokenBigramIgnoreBlankSplitSymbolAlpha
  It tokenise in bigram. Unlike TokenBigramSplitSymbolAlpha, it ignores white spaces.

  When you use TokenBigramIgnoreBlankSplitSymbolAlpha instead of TokenBigramSplitSymbolAlpha, "ama" can match "I am a pen.". But when you use TokenBigramSplitSymbolAlpha, only "am a" can match as well.

TokenBigramIgnoreBlankSplitSymbolAlphaDigit
  It tokenise in bigram. Unlike TokenBigramSplitSymbolAlphaDigit, it ignores white spaces.

  When you use TokenBigramIgnoreBlankSplitSymbolAlphaDigit instead of TokenBigramSplitSymbolAlphaDigit, "9001" can match "090 0123 4567". But when you use TokenBigramSplitSymbolAlphaDigit, only "90 01" can match as well.

TokenDelimit
  It tokenise by splitting with a white space.

  "映画 ホラー 話題" will be tokenised as "映画", "ホラー", "話題".

TokenDelimitNull
  It tokenise by splitting with a null character (\\0).

  "映画\\0ホラー\\0話題" will be tokenised as "映画", "ホラー", "話題".

TokenUnigram
  It tokenises in unigram. But continuous alphabets, numbers or symbols are treated as a token. So there can exist tokes with 2 letters or more. It is to reduce noises.

TokenTrigram
  It tokenises in trigram. But continuous alphabets, numbers or symbols are treated as a token. So there can exist tokes with 4 letters or more. It is to reduce noises.

You can specify the default parser by passing ``--with-default-parser`` option in ``configure`` when you build mroonga ::

  ./configure --with-default-parser TokenMecab ...

Or you can set ``mroonga_default_parser`` variable in my.cnf or by SQL. If you specify it in my.cnf, the change will not be lost after restarting MySQL, but you need to restart MySQL to make it effective. On the other hand, if you set it in SQL, the change is effective immediately, but it will be lost when you restart MySQL.

my.cnf::

  [mysqld]
  mroonga_default_parser=TokenMecab

SQL::

  mysql> SET GLOBAL mroonga_default_parser = TokenMecab;
  Query OK, 0 rows affected (0.00 sec)

How to use geolocation search
-----------------------------

In storage mode, you can use fast geolocation search in addition to full text search. But unlike MyISAM, you can only store POINT type data. You cannot store other types data like LINE. And fast search using index only supports MBRContains. It does not support MBRDisjoint.

For the table definition for geolocation search, you need to define a POINT type column like in MyISAM and define SPATIAL INDEX for it. ::

  mysql> CREATE TABLE shops (
      ->   id INT PRIMARY KEY AUTO_INCREMENT,
      ->   name VARCHAR(255),
      ->   location POINT NOT NULL,
      ->   SPATIAL INDEX (location)
      -> ) ENGINE = mroonga;
  Query OK, 0 rows affected (0.06 sec)

To store data, you create POINT type data by using geomFromText() function like in MyISAM. ::

  mysql> INSERT INTO shops VALUES (null, '根津のたいやき', GeomFromText('POINT(139.762573 35.720253)'));
  Query OK, 1 row affected (0.00 sec)

  mysql> INSERT INTO shops VALUES (null, '浪花家', GeomFromText('POINT(139.796234 35.730061)'));
  Query OK, 1 row affected (0.00 sec)

  mysql> INSERT INTO shops VALUES (null, '柳屋 たい焼き', GeomFromText('POINT(139.783981 35.685341)'));
  Query OK, 1 row affected (0.00 sec)

If you want to find shops within the rectangle where Ikebukuro station (139.7101 35.7292) is the top-left point and Tokyo Station (139.7662 35.6815) is the bottom-right point, SELECT phrase is like the following. ::

  mysql> SELECT id, name, AsText(location) FROM shops WHERE MBRContains(GeomFromText('LineString(139.7101 35.7292, 139.7662 35.6815)'), location);
  +----+-----------------------+------------------------------------------+
  | id | name                  | AsText(location)                         |
  +----+-----------------------+------------------------------------------+
  |  1 | 根津のたいやき | POINT(139.762572777778 35.7202527777778) |
  +----+-----------------------+------------------------------------------+
  1 row in set (0.00 sec)

Here you can search by geolocation!

How to get the record ID
------------------------

Groonga assigns a unique number to identify the record when a record is added in the table.

To make the development of applications easier, you can get this record ID by SQL in mroonga

To get the record ID, you need to create a column named ``_id`` when you create a table. ::

  mysql> CREATE TABLE memos (
      ->   _id INT,
       >   content VARCHAR(255),
      ->   UNIQUE KEY (_id) USING HASH
      -> ) ENGINE = mroonga;
  Query OK, 0 rows affected (0.04 sec)

Tye typo of _id column should be integer one (TINYINT, SMALLINT, MEDIUMINT, INT or BIGINT).

You can create an index for _id column, but it should be HASH type.

Let's add records in the table by INSERT. Since _id column is implemented as a virtual column and its value is assigned by groonga, you cannot specify the value when updating.
So you need to exclude it from setting columns, or you need to use ``null`` as its value. ::

  mysql> INSERT INTO memos VALUES (null, "今夜はさんま。");
  Query OK, 1 row affected (0.00 sec)

  mysql> INSERT INTO memos VALUES (null, "明日はmroongaをアップデート。");
  Query OK, 1 row affected (0.00 sec)

  mysql> INSERT INTO memos VALUES (null, "帰りにおだんご。");
  Query OK, 1 row affected (0.00 sec)

  mysql> INSERT INTO memos VALUES (null, "金曜日は肉の日。");
  Query OK, 1 row affected (0.00 sec)

To get the record ID, you invoke SELECT with _id column. ::

  mysql> SELECT * FROM memos;
  +------+------------------------------------------+
  | _id  | content                                  |
  +------+------------------------------------------+
  |    1 | 今夜はさんま。                    |
  |    2 | 明日はmroongaをアップデート。 |
  |    3 | 帰りにおだんご。                 |
  |    4 | 金曜日は肉の日。                 |
  +------+------------------------------------------+
  4 rows in set (0.00 sec)

By using last_insert_grn_id function, you can also get the record ID that is assigned by the last INSERT. ::

  mysql> INSERT INTO memos VALUES (null, "冷蔵庫に牛乳が残り1本。");
  Query OK, 1 row affected (0.00 sec)

  mysql> SELECT last_insert_grn_id();
  +----------------------+
  | last_insert_grn_id() |
  +----------------------+
  |                    5 |
  +----------------------+
  1 row in set (0.00 sec)

last_insert_grn_id function is included in mroonga as a User-Defined Function (UDF), but if you have not yet register it in MySQL by CREATE FUNCTION, you need to invoke the following SQL for defining a function. ::

  mysql> CREATE FUNCTION last_insert_grn_id RETURNS INTEGER SONAME 'ha_mroonga.so';

As you can see in the example above, you can get the record ID by _id column or last_insert_grn_id function. It will be useful to use this value in the ensuing SQL queries like UPDATE. ::

  mysql> UPDATE memos SET content = "冷蔵庫に牛乳はまだたくさんある。" WHERE _id = last_insert_grn_id();
  Query OK, 1 row affected (0.00 sec)
  Rows matched: 1  Changed: 1  Warnings: 0

How to get snippet (Keyword in context)
---------------------------------------

.. include:: mroonga_snippet_syntax.inc

Here is the schema definition for execution examples::

  CREATE TABLE `snippet_test` (
    `id` int(11) NOT NULL,
    `text` text,
    PRIMARY KEY (`id`),
    FULLTEXT KEY `text` (`text`)
  ) ENGINE=mroonga DEFAULT CHARSET=utf8

.. include:: mroonga_snippet_example.inc

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

You can reopen the log file by FLUSH LOGS. If you want to rotate the log file without stopping MySQL server, you can do in the following procedure.

1. change the file name of ``groonga.log`` (by using OS's mv command etc.).
2. invoke "FLUSH LOGS" in MySQL server (by mysql command or mysqladmin command).

Choosing appropriate columns
----------------------------

Groonga uses one file per column to store data, and mroonga accesses needed columns only when accessing a table to utilise this characteristic.

This optimisation is done automatically in mroonga internal, you don't need any specific configuration.

Imagine that we have a table with 20 columns like below. ::

  CREATE TABLE t1 (
    c1 INT PRIMARY KEY AUTO_INCREMENT,
    c2 INT,
    c3 INT,
    ...
    c11 VARCHAR(20),
    c12 VARCHAR(20),
    ...
    c20 DATETIME
  ) ENGINE = mroonga DEFAULT CHARSET utf8;

When we run SELECT phrase like the following, mroonga reads data from columns that are referred by SELECT phrase and WHERE phrase only (and it does not access columns that not required internally).

  SELECT c1, c2, c11 FROM t1 WHERE c2 = XX AND c12 = "XXX";

In this case above, only columns c1, c2, c11 and c12 are accessed, and we can process the SQL rapidly.

Optimisation for counting rows
------------------------------

In MySQL's storage engine interface, there is no difference between counting rows like COUNT(\*) and normal data retrieving by SELECT. So access to data that is not included in SELECT result can happen even if you just want to count rows.

Tritonn (MySQL + Senna), that is mroonga's predecessor, introduced "2ind patch" to skip needless access to data and solved this performance issue.

Mroonga also has the optimisation for counting rows.

In the following SELECT, for example, needless read of columns are skipped and you can get the result of counting rows with the minimal cost.

  SELECT COUNT(*) FROM t1 WHERE MATCH(c2) AGAINST("hoge");

You can check if this optimisation works or not by the status variable. ::

  mysql> SHOW STATUS LIKE 'mroonga_count_skip';
  +--------------------+-------+
  | Variable_name      | Value |
  +--------------------+-------+
  | mroonga_count_skip | 1     |
  +--------------------+-------+
  1 row in set (0.00 sec)

Each time the optimisation for counting rows works, ``mroonga_count_skip`` status variable value is increased.

Note : This optimisation is implemented by using the index. It only works in the case where we records can be specified only by the index.

Optimisation for ORDER BY LIMIT in full text search
---------------------------------------------------

Generally speaking, MySQL can process "ORDER BY" query with almost no cost if we can get records by index, and can process "LIMIT" with low cost by limiting the range of processing data even if the number of query result is very big.

But for the query where "ORDER BY" cannot use index, like sort full text search result by the score and use LIMIT, the processing cost is proportional to the number of query results. So it might take very long time for the keyword query that matches with many records.

Tritonn took no specific countermeasure for this issue, but it introduced a workaround in the latest repository so that it sorted Senna result in descending order of the score by using sen_records_sort function so that we could remove ORDER BY from the SQL query.

Mroonga also has the optimisation for ORDER BY LIMIT.

In the SELECT example below, ORDER BY LIMIT is processed in groonga only and the minimal records are passed to MySQL. ::

  SELECT * FROM t1 WHERE MATCH(c2) AGAINST("hoge") ORDER BY c1 LIMIT 1;

You can check if this optimisation works or not by the status variable. ::

  mysql> SHOW STATUS LIKE 'mroonga_fast_order_limit';
  +--------------------------+-------+
  | Variable_name            | Value |
  +--------------------------+-------+
  | mroonga_fast_order_limit | 1     |
  +--------------------------+-------+
  1 row in set (0.00 sec)

Each time the optimisation for counting rows works, ``mroonga_fast_order_limit`` status variable value is increased.

Note : This optimisation is targeting queries like "select ... match against order by _score desc limit X, Y" only, and it works if all of the following conditions are right.

* WHERE phrase has "match...against" only
* no JOIN
* with LIMIT
* ORDER BY phrase has columns (including _id) or "match...against" that is used in WHERE phrase only

.. rubric:: Footnotes

.. [#score] `MySQL 5.1 Reference Manual :: 11 Functions and Operations :: 11.7 Full-Text Search Functions <http://dev.mysql.com/doc/refman/5.1/ja/fulltext-search.html>`_
.. [#parser] In groonga, we call it a 'tokeniser'.
