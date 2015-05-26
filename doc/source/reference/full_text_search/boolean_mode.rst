Mroonga full text search in boolean mode
========================================

Mroonga can perform boolean full-text searches using the "IN BOOLEAN MODE" modifier.

When building a boolean search query, Mroonga supports the use of qualifiers which MySQL support, and original pragmas.

These qualifiers and pragmas can change the relative rank of search results.

In the case of a search string not using neither a qualifier nor a pragma, the search results that contain the search string will be rated higher.

Mroonga supports the following qualifiers
-----------------------------------------

.. list-table::
  :header-rows: 1

  * - Qualifier
    - Description
  * - ``+``
    - A leading plus sign indicates that this word must be present in each row that is returned.
  * - ``-``
    - A leading minus sign indicates that this word must not be present in any of the rows that are returned.
  * - ``*``
    - The asterisk serves as the truncation (or wildcard) operator.
  * - ``"``
    - A phrase that is enclosed within double quote ( ``"`` ) characters matches only rows that contain the phrase literally, as it was typed.
  * - ``()``
    - Parentheses group words into subexpressions.

Examples
--------

We show you how to rank in boolean mode using the following schema and data. ::

  CREATE TABLE books (
    `id` INTEGER AUTO_INCREMENT,
    `title` text,
    PRIMARY KEY(`id`),
    FULLTEXT INDEX title_index (title)
  ) ENGINE=mroonga default charset utf8;
  
  INSERT INTO books (title) VALUES (
    'MySQL'
  );
  
  INSERT INTO books (title) VALUES (
    'MySQL Groonga'
  );
  
  INSERT INTO books (title) VALUES (
    'MySQL Groonga Mroonga'
  );

.. _pragma:

Pragma
------

You can embed pragma at the head of query for specifying how to execute.

The pragma must exist in the beginning of a query. (Don't put a blank into a head of the query)

D pragma
^^^^^^^^

D pragma is a form for specifying which operation should be execute when an individual operator is omitted.

The syntax of D pragma is as follows. You can choose one of the three operators, either ``OR``, ``+``, or ``-``. ::

  *D[operator]

``*DOR``
""""""""

We use ``*DOR Groonga Mroonga`` to search.::

  SELECT title, MATCH (title) AGAINST('*DOR Groonga Mroonga' IN BOOLEAN MODE) AS score FROM books;

The search result is a row that contains ``Groonga`` or ``Mroonga``. The row score is as follows::

  +-----------------------+-------+
  | title                 | score |
  +-----------------------+-------+
  | MySQL                 |     0 |
  | MySQL Groonga         |     1 |
  | MySQL Groonga Mroonga |     2 |
  +-----------------------+-------+
  3 rows in set (0.00 sec)


``*D+``
"""""""

We use a query ``*D+ Groonga Mroonga`` to search.::

  SELECT title, MATCH (title) AGAINST('*D+ Groonga Mroonga' IN BOOLEAN MODE) AS score FROM books;

The search result is a row that contains ``Groonga`` and ``Mroonga``. The row score is as follows::

  +-----------------------+-------+
  | title                 | score |
  +-----------------------+-------+
  | MySQL                 |     0 |
  | MySQL Groonga         |     0 |
  | MySQL Groonga Mroonga |     2 |
  +-----------------------+-------+
  3 rows in set (0.01 sec)

``*D-``
"""""""

We use a query ``*D- Groonga Mroonga`` to search.::

  SELECT title, MATCH (title) AGAINST('*D- Groonga Mroonga' IN BOOLEAN MODE) AS score FROM books;


The search result is a row that contains ``Groonga``, and does not contain ``Mroonga``. The row score is as follows::

  +-----------------------+-------+
  | title                 | score |
  +-----------------------+-------+
  | MySQL                 |     0 |
  | MySQL Groonga         |     1 |
  | MySQL Groonga Mroonga |     0 |
  +-----------------------+-------+
  3 rows in set (0.00 sec)

W pragma
^^^^^^^^

W pragma is a form for specifying target section and its weight when using multiple-column index.

You can specify the multiple of the weight for every section. The default value of the weight is set to 1.

The weight is omissible, and a negative value is allowed.

The syntax of W pragma is as follows. A section number is begun not from 0 but from 1. ::

  *W[number1[:weight1][,number2[:weight2]]...

We show you how to rank in multiple-column index using the following schema and data.::

  CREATE TABLE books (
    `id` INTEGER AUTO_INCREMENT,
    `title` text,
    `comment` text,
    PRIMARY KEY(`id`),
    FULLTEXT INDEX content_index (title, comment)
  ) engine=mroonga default charset utf8;
  
  INSERT INTO books (title, comment) VALUES (
    'MySQL', 'MySQL Introduction'
  );
  
  INSERT INTO books (title, comment) VALUES (
    'MySQL Groonga', 'Groonga Introduction'
  );
  
  INSERT INTO books (title, comment) VALUES (
    'MySQL Groonga Mroonga', 'Mroonga Introduction'
  );

Consider the case that 10 weight is given to ``title`` column, and 1 weight is given to ``comment`` column, and use ``Groonga`` to search.

Here is the query such a case::

  SELECT title, comment, MATCH (title,comment) AGAINST('*W1:10,2:1 +Groonga' IN BOOLEAN MODE) AS score FROM books;

The search result is a row that contains "Groonga". The row score is as follows::

  +-----------------------+----------------------+-------+
  | title                 | comment              | score |
  +-----------------------+----------------------+-------+
  | MySQL                 | MySQL Introduction   |     0 |
  | MySQL Groonga         | Groonga Introduction |    11 |
  | MySQL Groonga Mroonga | Mroonga Introduction |    10 |
  +-----------------------+----------------------+-------+
  3 rows in set (0.01 sec)

The row score is set to 11 that the ``title`` and ``comment`` column contain ``Groonga``, and the row score is set to 10 that only ``title`` column contains ``Groonga``.

Qualifier
---------

(no operator)
^^^^^^^^^^^^^

We use a query ``Groonga Mroonga`` to search.::

  SELECT title, MATCH (title) AGAINST('Groonga Mroonga' IN BOOLEAN MODE) AS score FROM books;

Rows that contain even one word will be ranked higher.

A row score is set to 1 when the row contains a single word.

A row score is set to 2 when the row contains both words.

Here is the search result::

  +-----------------------+-------+
  | title                 | score |
  +-----------------------+-------+
  | MySQL                 |     0 |
  | MySQL Groonga         |     1 |
  | MySQL Groonga Mroonga |     2 |
  +-----------------------+-------+
  3 rows in set (0.00 sec)

``+``
^^^^^

We use a query ``+Groonga +Mroonga`` to search.::

  SELECT title, MATCH (title) AGAINST('+Groonga +Mroonga' IN BOOLEAN MODE) AS score FROM books;

The search result is a row that contains both words. The row score is as follows::

  +-----------------------+-------+
  | title                 | score |
  +-----------------------+-------+
  | MySQL                 |     0 |
  | MySQL Groonga         |     0 |
  | MySQL Groonga Mroonga |     2 |
  +-----------------------+-------+
  3 rows in set (0.00 sec)

``-``
^^^^^

We use a query ``-Groonga -Mroonga`` to search.::

  SELECT title, MATCH (title) AGAINST('-Groonga -Mroonga' IN BOOLEAN MODE) AS score FROM books;

The search result is a row that does not contain both words. The row score is as follows::

  +-----------------------+-------+
  | title                 | score |
  +-----------------------+-------+
  | MySQL                 |     1 |
  | MySQL Groonga         |     0 |
  | MySQL Groonga Mroonga |     0 |
  +-----------------------+-------+
  3 rows in set (0.01 sec)

``*``
^^^^^

We use a query ``M*`` to search.::

  SELECT title, MATCH (title) AGAINST('M*' IN BOOLEAN MODE) AS score FROM books;

The search result is a row that contains ``MySQL`` or ``Mroonga``. The row score is as follows::

  +-----------------------+-------+
  | title                 | score |
  +-----------------------+-------+
  | MySQL                 |     1 |
  | MySQL Groonga         |     1 |
  | MySQL Groonga Mroonga |     2 |
  +-----------------------+-------+
  3 rows in set (0.01 sec)


``"``
^^^^^

We use a query ``"Groonga Mroonga"`` to search.::

  SELECT title, MATCH (title) AGAINST('"Groonga Mroonga"' IN BOOLEAN MODE) AS score FROM books;

The search result is a row that contains a phrase that matches ``Groonga Mroonga``. The row score is as follows::

  +-----------------------+-------+
  | title                 | score |
  +-----------------------+-------+
  | MySQL                 |     0 |
  | MySQL Groonga         |     0 |
  | MySQL Groonga Mroonga |     1 |
  +-----------------------+-------+
  3 rows in set (0.00 sec)

``()``
^^^^^^

We use a query ``+MySQL +(Groonga Mroonga)`` to search.::

  SELECT title, MATCH (title) AGAINST('+MySQL +(Groonga Mroonga)' IN BOOLEAN MODE) AS score FROM books;

The search result is a row that contains ``MySQL`` and ``Groonga``, or ``MySQL`` and ``Mroonga``, or ``MySQL`` and ``Groonga`` and ``Mroonga``.

The row score is as follows::

  +-----------------------+-------+
  | title                 | score |
  +-----------------------+-------+
  | MySQL                 |     0 |
  | MySQL Groonga         |     2 |
  | MySQL Groonga Mroonga |     3 |
  +-----------------------+-------+
  3 rows in set (0.01 sec)
