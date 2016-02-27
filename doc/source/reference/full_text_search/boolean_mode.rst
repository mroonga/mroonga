Boolean mode
============

Summary
-------

Mroonga can perform boolean full text searches using the ``IN BOOLEAN
MODE`` modifier for ``MATCH AGAINST``::

  SELECT ... WHERE MATCH(column) AGAINST ('...' IN BOOLEAN MODE);

Normally, ``IN BOOLEAN MODE`` is suitable rather than the default ``IN
NATURAL LANGUAGE MODE``. Because ``IN BOOLEAN MODE`` is similar to
query in Web search engine. Most people familiar with query in Web
search engine.

You can use `qualifiers which MySQL support
<https://dev.mysql.com/doc/refman/5.7/en/fulltext-boolean.html>`_ and
Mroonga original pragmas in boolean full text search query.

These qualifiers and pragmas can change the relative rank of search
results.

In the case of a search string not using neither a qualifier nor a
pragma, the search results that contain the search string will be
rated higher.

Usage
-----

Here are schema and data to show examples::

  CREATE TABLE books (
    `id` INTEGER AUTO_INCREMENT,
    `title` text,
    PRIMARY KEY(`id`),
    FULLTEXT INDEX title_index (title)
  ) ENGINE=Mroonga DEFAULT CHARSET=utf8mb4;

  INSERT INTO books (title) VALUES ('Professional MySQL');
  INSERT INTO books (title) VALUES ('MySQL for Professional');
  INSERT INTO books (title) VALUES ('Mroonga = MySQL + Groonga');

.. _boolean-mode-qualifier:

Qualifier
---------

Here are supported qualifiers.

.. _boolean-mode-qualifier-none:

``KEYWORD1 KEYWORD2``
^^^^^^^^^^^^^^^^^^^^^

No operator between keywords such as ``KEYWORD1 KEYWORD2`` indicates
that one of keywords must be present in each row that is returned.

``Mroonga for`` query means that ``Mroonga`` or ``for`` must be
present::

  SELECT title
    FROM books
   WHERE MATCH(title) AGAINST('Mroonga for' IN BOOLEAN MODE);
  -- +---------------------------+
  -- | title                     |
  -- +---------------------------+
  -- | Mroonga = MySQL + Groonga |
  -- | MySQL for Professional    |
  -- +---------------------------+

.. _boolean-mode-qualifier-or:

``KEYWORD1 OR KEYWORD2``
^^^^^^^^^^^^^^^^^^^^^^^^

``OR`` (must be uppercase) indicates that left hand side keyword or
right hand side keyword must be present in each row that is returned.

``Mroonga OR for`` query means that ``Mroonga`` or ``for`` must be
present::

  SELECT title
    FROM books
   WHERE MATCH(title) AGAINST('Mroonga OR for' IN BOOLEAN MODE);
  -- +---------------------------+
  -- | title                     |
  -- +---------------------------+
  -- | Mroonga = MySQL + Groonga |
  -- | MySQL for Professional    |
  -- +---------------------------+

``OR`` is the default operator. You can omit it. Both ``Mroonga OR
for`` and ``Mroonga for`` return the same result.

.. _boolean-mode-qualifier-plus:

``+KEYWORD``
^^^^^^^^^^^^

A leading plus sign indicates that this word must be present in each
row that is returned.

``+MySQL +Mroonga`` query means that both ``MySQL`` and ``Mroonga``
must be present::

  SELECT title
    FROM books
   WHERE MATCH(title) AGAINST('+MySQL +Groonga' IN BOOLEAN MODE);
  -- +---------------------------+
  -- | title                     |
  -- +---------------------------+
  -- | Mroonga = MySQL + Groonga |
  -- +---------------------------+

.. _boolean-mode-qualifier-minus:

``-KEYWORD``
^^^^^^^^^^^^

A leading minus sign indicates that this word must not be present in
any of the rows that are returned.

``+MySQL -Mroonga`` query means that ``MySQL`` must be present but
``Mroonga`` must not be present::

  SELECT title
    FROM books
   WHERE MATCH(title) AGAINST('+MySQL -Mroonga' IN BOOLEAN MODE);
  -- +------------------------+
  -- | title                  |
  -- +------------------------+
  -- | Professional MySQL     |
  -- | MySQL for Professional |
  -- +------------------------+

.. _boolean-mode-qualifier-asterisk:

``PREFIX*``
^^^^^^^^^^^^

A following asterisk indicates that all words starting with this word
must be present in any of the rows that are returned.

``+M*`` query means that words starting ``M`` (``MySQL`` and
``Mroonga`` in this case) must be present::

  SELECT title
    FROM books
   WHERE MATCH(title) AGAINST('+M*' IN BOOLEAN MODE);
  -- +---------------------------+
  -- | title                     |
  -- +---------------------------+
  -- | Mroonga = MySQL + Groonga |
  -- | Professional MySQL        |
  -- | MySQL for Professional    |
  -- +---------------------------+

.. note::

   To be precise, "word" may not be "word" you think. "word" in this
   context is "token". "token" may not be word. For example, tokens in
   "It's" are "It", "'" and "s".

   You can confirm token by :doc:`/reference/udf/mroonga_command` and
   `tokenize
   <http://groonga.org/docs/reference/commands/tokenize.html>`_::

     SELECT mroonga_command('tokenize TokenBigram "It''s" NormalizerMySQLGeneralCI');
     -- +--------------------------------------------------------------------------+
     -- | mroonga_command('tokenize TokenBigram "It''s" NormalizerMySQLGeneralCI') |
     -- +--------------------------------------------------------------------------+
     -- | [                                                                        |
     -- |   {                                                                      |
     -- |     "value":"IT",                                                        |
     -- |     "position":0,                                                        |
     -- |     "force_prefix":false                                                 |
     -- |   },                                                                     |
     -- |   {                                                                      |
     -- |     "value":"'",                                                         |
     -- |     "position":1,                                                        |
     -- |     "force_prefix":false                                                 |
     -- |   },                                                                     |
     -- |   {                                                                      |
     -- |     "value":"S",                                                         |
     -- |     "position":2,                                                        |
     -- |     "force_prefix":false                                                 |
     -- |   }                                                                      |
     -- | ]                                                                        |
     -- +--------------------------------------------------------------------------+

   JSON value in the above result is formatted by hand.

.. _boolean-mode-qualifier-double-quote:

``"PHRASE"``
^^^^^^^^^^^^

Quoting phrase by double quote (``"``) indicates that the phrase must
be present in any of the rows that are returned.

``+"Professional MySQL"`` query means that ``Professional MySQL``
phrase must be present. The query doesn't match to ``MySQL for
Profession``. ``MySQL for Profession`` includes both ``MySQL`` and
``Professional`` words but doesn't include ``Professional MySQL``
phrase::

  SELECT title
    FROM books
   WHERE MATCH(title) AGAINST('+"Professional MySQL"' IN BOOLEAN MODE);
  -- +--------------------+
  -- | title              |
  -- +--------------------+
  -- | Professional MySQL |
  -- +--------------------+

.. _boolean-mode-qualifier-parentheses:

``(SUBEXPRESSION...)``
^^^^^^^^^^^^^^^^^^^^^^

Parentheses groups expressions.

``+(Groonga OR Mroonga) +MySQL`` query means the following:

  * ``Groonga`` or ``Mroonga`` must be present.
  * ``MySQL`` must be present.

Here is the result of the query::

  SELECT title
    FROM books
   WHERE MATCH(title) AGAINST('+(Groonga OR Mroonga) +MySQL' IN BOOLEAN MODE);
  -- +---------------------------+
  -- | title                     |
  -- +---------------------------+
  -- | Mroonga = MySQL + Groonga |
  -- +---------------------------+

.. _boolean-mode-pragma:

Pragma
------

Pragma is metadata for query. You can change how to parse query by
specifying pragma.

You can embed pragma at the head of query for specifying how to
execute.

Pragma must exist in the beginning of a query. Don't put a blank into
a head of the query. Pragma starts with ``*``::

  SELECT MATCH AGAINST('*PRAGMA ...' IN BOOLEAN MODE);

You can specify multiple pragmas::

  SELECT MATCH AGAINST('*PRAGMA1PRAGMA2 ...' IN BOOLEAN MODE);

Here are available pragmas.

.. _boolean-mode-pragma-d:

``D`` pragma
^^^^^^^^^^^^

``D`` pragma indicates the default operator. It's used when an
individual operator is omitted.

Here is the ``D`` pragma syntax. You can choose one of ``OR``, ``+``
or ``-`` as ``${OPERATOR}``::

  *D${OPERATOR}

.. _boolean-mode-pragma-d-or:

``DOR``
"""""""

``DOR`` means that "or" is used as the default operator.

This is the default.

Here is an example to use ``DOR``. ``'*DOR for Mroonga' IN BOOLEAN
MODE`` returns records that includes ``for`` or ``Mroonga``::

  SELECT title
   FROM books
  WHERE MATCH (title) AGAINST('*DOR for Mroonga' IN BOOLEAN MODE);
  -- +---------------------------+
  -- | title                     |
  -- +---------------------------+
  -- | MySQL for Professional    |
  -- | Mroonga = MySQL + Groonga |
  -- +---------------------------+

.. _boolean-mode-pragma-d-plus:

``D+``
""""""

``D+`` means that "and" is used as the default operator. It's similar
to query in Web search engine.

Here is an example to use ``D+``. ``'*D+ MySQL Mroonga' IN BOOLEAN
MODE`` returns records that includes ``MySQL`` and ``Mroonga``::

  SELECT title
   FROM books
  WHERE MATCH (title) AGAINST('*D+ MySQL Mroonga' IN BOOLEAN MODE);
  -- +---------------------------+
  -- | title                     |
  -- +---------------------------+
  -- | Mroonga = MySQL + Groonga |
  -- +---------------------------+

.. _boolean-mode-pragma-d-minus:

``D-``
""""""

``D-`` means that "not" is used as the default operator.

Here is an example to use ``D-``. ``'*D- MySQL Mroonga' IN BOOLEAN
MODE`` returns records that includes ``MySQL`` but doesn't include
``Mroonga``::

  SELECT title
   FROM books
  WHERE MATCH (title) AGAINST('*D- MySQL Mroonga' IN BOOLEAN MODE);
  -- +------------------------+
  -- | title                  |
  -- +------------------------+
  -- | Professional MySQL     |
  -- | MySQL for Professional |
  -- +---------------------------+

.. _boolean-mode-pragma-w:

``W`` pragma
^^^^^^^^^^^^

``W`` pragma indicates target section and its weight for multiple
column index.

You can specify different weight for each section. The default weight
is ``1``. ``1`` means that no weight.

Here is the ``W`` pragma syntax. ``${SECTION}`` is a number that is
begun not from 0 but from 1. ``${WEIGHT}`` is omitable::

  *W[${SECTION1}[:${WEIGHT1}]][,${SECTION2}[:${WEIGHT2}]][,...]

Here are schema and data to show examples. You need to create a
multiple column index to use ``W`` pragma::

  CREATE TABLE memos (
    `id` INTEGER AUTO_INCREMENT,
    `title` text,
    `content` text,
    PRIMARY KEY(`id`),
    FULLTEXT INDEX text_index (title, content)
  ) ENGINE=Mroonga DEFAULT CHARSET=utf8mb4;

  INSERT INTO memos (title, content) VALUES (
    'MySQL', 'MySQL is a RDBMS.'
  );
  INSERT INTO memos (title, content) VALUES (
    'Groonga', 'Groonga is a full text search engine.'
  );
  INSERT INTO memos (title, content) VALUES (
    'Mroonga', 'Mroonga is a storage engine for MySQL based on Groonga.'
  );

Here is an example to show weight. ``title`` column has ``10`` weight
and ``content`` columns has ``1`` weight. It means that keyword in
``title`` column is 10 times important than keyword in ``content``
column::

  SELECT title,
         content,
         MATCH (title, content) AGAINST('*W1:10,2:1 +Groonga' IN BOOLEAN MODE) AS score
    FROM memos;
  -- +---------+--------------------------------------------------------+-------+
  -- | title   | content                                                | score |
  -- +---------+--------------------------------------------------------+-------+
  -- | MySQL   | MySQL is a RDBMS.                                      |     0 |
  -- | Groonga | Groonga is a full text search engine.                  |    11 |
  -- | Mroonga | Mroonga is a storage engine for MySQL based on Groonga |     1 |
  -- +---------+--------------------------------------------------------+-------+

The score of the first record is ``0``. Because it doesn't have any
``Groonga`` in both ``title`` column and ``content`` column.

The score of the second record is ``11``. Because it has ``Groonga`` in
both ``title`` column and ``content`` column. ``Groonga`` in ``title``
column has score ``10``. ``Groonga`` in ``content`` column has score
``1``. ``11`` is sum of them.

The score of the third record is ``1``. Because it has ``Groonga`` in
only ``content`` column. ```Groonga`` in ``content`` column has score
``1``. So the score of the record is ``1``.

.. _boolean-mode-pragma-s:

``S`` pragma
^^^^^^^^^^^^

``S`` pragma indicates syntax of the query.

Here is a syntax of ``S`` pragma::

  *S${SYNTAX}

Here is a list of available ``syntax``:

  * ``S``: `Script syntax
    <http://groonga.org/docs/reference/grn_expr/script_syntax.html>`_

.. _boolean-mode-pragma-ss:

``*SS``
"""""""

You can use `script syntax
<http://groonga.org/docs/reference/grn_expr/script_syntax.html>`_ by
``*SS`` pragma. You can use full Groonga search features in script
syntax.

Here are schema and data to show example of script syntax usage::

  CREATE TABLE comments (
    `content` text,
    FULLTEXT INDEX content_index (content)
  ) ENGINE=Mroonga DEFAULT CHARSET=utf8mb4;

  INSERT INTO comments VALUES (
    'A student started to use Mroonga storage engine. It is very fast!'
  );
  INSERT INTO comments VALUES (
    'Another student also started to use Mroonga storage engine. It is very fast!'
  );

Here is an example to use `near search
<http://groonga.org/docs/reference/grn_expr/script_syntax.html#near-search-operator>`_ by script syntax::

  SELECT content,
         MATCH (content) AGAINST('*SS content *N "student fast"' IN BOOLEAN MODE) AS score
    FROM comments;
  -- +------------------------------------------------------------------------------+-------+
  -- | content                                                                      | score |
  -- +------------------------------------------------------------------------------+-------+
  -- | A student started to use Mroonga storage engine. It is very fast!            |     1 |
  -- | Another student also started to use Mroonga storage engine. It is very fast! |     0 |
  -- +------------------------------------------------------------------------------+-------+

Near search matches only when there are 10 or less words between
specified words (``student`` and ``fast`` in this case). So ``student
started ...(8 words)... very fast`` is matched but ``student also
started ...(8 words)... very fast`` isn't matched.

You can also use other advanced features.
