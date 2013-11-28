.. highlightlang:: none

mroonga_snippet
===============

Summary
-------

``mroonga_snippet`` UDF provides functionality to get keyword
in context.

.. note::

   This feature is in experimental stage.
   So, the required arguments or value is changed without notice in the
   future.

Syntax
------

mroonga_snippet has required parameter and optional parameter::

  mroonga_snippet document
                  max_length
                  max_count
                  encoding
                  skip_leading_spaces
                  html_escape
                  snippet_prefix
                  snippet_suffix
                  word1 word1_prefix word1_suffix
                  ...
                  [wordN wordN_prefix wordN_suffix]

Usage
-----


There is a case that you want to extract keyword and surrounding text as a
search results.

Snippet means 'keyword and surrounding text'. It is called 'Keyword in context'.

``mroonga_snippet`` function provides the way to get snippet from search results.

You can use ``mroonga_snippet`` UDF in storage mode and wrapper mode.

``mroonga_snippet`` function is included in mroonga as a User-Defined Function (UDF), but if you have not yet register it in MySQL by CREATE FUNCTION, you need to invoke the following SQL for defining a function. ::

  mysql> CREATE FUNCTION mroonga_snippet RETURNS STRING SONAME 'ha_mroonga.so';

``mroonga_snippet`` function is useful for searching the text which contains keyword and associated one
by using MATCH .. AGAINST syntax.

Imagine searching the document which contains 'fulltext' as a keyword.
Assume that some keyword such as 'MySQL' and 'search' are associated with 'fulltext'.

``mroonga_snippet`` function meets above.

Here is the schema definition for execution examples(storage mode)::

  CREATE TABLE `snippet_test` (
    `id` int(11) NOT NULL,
    `text` text,
    PRIMARY KEY (`id`),
    FULLTEXT KEY `text` (`text`)
  ) ENGINE=mroonga DEFAULT CHARSET=utf8


Here is the schema definition for execution examples(wrapper mode)::

  CREATE TABLE `snippet_test` (
    `id` int(11) NOT NULL,
    `text` text,
    PRIMARY KEY (`id`),
    FULLTEXT KEY `text` (`text`)
  ) ENGINE=mroonga COMMENT 'engine = "innodb"' DEFAULT CHARSET=utf8


Here is the sample data for execution examples::

  INSERt INTO snippet_test (id, text) VALUES (1, 'An open-source fulltext search engine and column store.');
  INSERT INTO snippet_test (id, text) VALUES (2, 'An open-source storage engine for fast fulltext search with MySQL.');
  INSERT INTO snippet_test (id, text) VALUES (3, 'Tritonn is a patched version of MySQL that supports better fulltext search function with Senna.');

Here is the results of execution examples::

  mysql> SELECT * FROM snippet_test;
  +----+-------------------------------------------------------------------------------------------------+
  | id | text                                                                                            |
  +----+-------------------------------------------------------------------------------------------------+
  |  1 | An open-source fulltext search engine and column store.                                         |
  |  2 | An open-source storage engine for fast fulltext search with MySQL.                              |
  |  3 | Tritonn is a patched version of MySQL that supports better fulltext search function with Senna. |
  +----+-------------------------------------------------------------------------------------------------+
  3 rows in set (0.00 sec)
  
  mysql> SELECT id, text, mroonga_snippet(text, 8, 2, 'ascii_general_ci', 1, 1, '...', '...<br>', 'fulltext', '<span class="w1">', '</span>', 'MySQL', '<span class="w2">', '</span>', 'search', '<span calss="w3">', '</span>') FROM snippet_test WHERE MATCH(text) AGAINST ('fulltext');
  +----+-------------------------------------------------------------------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
  | id | text                                                                                            | mroonga_snippet(text, 8, 2, 'ascii_general_ci', 1, 1, '...', '...<br>', 'fulltext', '<span class="w1">', '</span>', 'MySQL', '<span class="w2">', '</span>', 'search', '<span calss="w3">', '</span>') |
  +----+-------------------------------------------------------------------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
  |  1 | An open-source fulltext search engine and column store.                                         | ...<span class="w1">fulltext</span>...<br>... <span calss="w3">search</span> ...<br>                                                                                                                   |
  |  2 | An open-source storage engine for fast fulltext search with MySQL.                              | ...<span class="w1">fulltext</span>...<br>... <span calss="w3">search</span> ...<br>                                                                                                                   |
  |  3 | Tritonn is a patched version of MySQL that supports better fulltext search function with Senna. | ...f <span class="w2">MySQL</span> ...<br>...<span class="w1">fulltext</span>...<br>                                                                                                                   |
  +----+-------------------------------------------------------------------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
  3 rows in set (0.00 sec)
  
The keyword 'fulltext' and associated keyword 'MySQL' and 'search' has been extracted.

Parameters
----------

Required parameters
^^^^^^^^^^^^^^^^^^^

There are many required parameters.


``document``
""""""""""""
  The column name or string value is required.

``max_length``
""""""""""""""

  The max length of snippet (bytes) is required.

``max_count``
"""""""""""""

  The max elements of snippets (N word) is required.

``encoding``
""""""""""""

  The encoding of document is required.
  You can specify the value of encoding such as 'ascii_general_ci',
  'cp932_japanese_ci', 'eucjpms_japanese_ci' and so on.

``skip_leading_spaces``
"""""""""""""""""""""""

  Specify whether skip leading spaces or not.
  Specify the value 1 for skipping leading spaces, 0 for not.

``html_escape``
"""""""""""""""

  HTML escape is enabled or not.
  Specify the value 1 for enabling HTML escape, 0 for not.

``prefix``
""""""""""

  The start text of snippet.

``suffix``
""""""""""

  The end text of snippet.


Optional parameters
^^^^^^^^^^^^^^^^^^^

There is one optional group parameter, ``wordN`` related prefix and suffix.

``wordN``
"""""""""

  Specify any word.

``wordN_prefix``
""""""""""""""""

  It is the start text of wordN.

``wordN_suffix``
""""""""""""""""

  It is the end text of wordN.


Return value
------------

It returns snippet string.


