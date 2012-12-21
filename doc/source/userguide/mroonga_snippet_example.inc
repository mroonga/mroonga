Here is the sample data for execution examples::

  insert into snippet_test (id, text) values (1, 'An open-source fulltext search engine and column store.');
  insert into snippet_test (id, text) values (2, 'An open-source storage engine for fast fulltext search with MySQL.');
  insert into snippet_test (id, text) values (3, 'Tritonn is a patched version of MySQL that supports better fulltext search function with Senna.');

Here is the results of execution examples::

  mysql> select * from snippet_test;
  +----+-------------------------------------------------------------------------------------------------+
  | id | text                                                                                            |
  +----+-------------------------------------------------------------------------------------------------+
  |  1 | An open-source fulltext search engine and column store.                                         |
  |  2 | An open-source storage engine for fast fulltext search with MySQL.                              |
  |  3 | Tritonn is a patched version of MySQL that supports better fulltext search function with Senna. |
  +----+-------------------------------------------------------------------------------------------------+
  3 rows in set (0.00 sec)
  
  mysql> select id, text, mroonga_snippet(text, 8, 2, 'ascii_general_ci', 1, 1, '...', '...<br>', 'fulltext', '<span class="w1">', '</span>', 'MySQL', '<span class="w2">', '</span>', 'search', '<span calss="w3">', '</span>') from snippet_test where match(text) against ('fulltext');
  +----+-------------------------------------------------------------------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
  | id | text                                                                                            | mroonga_snippet(text, 8, 2, 'ascii_general_ci', 1, 1, '...', '...<br>', 'fulltext', '<span class="w1">', '</span>', 'MySQL', '<span class="w2">', '</span>', 'search', '<span calss="w3">', '</span>') |
  +----+-------------------------------------------------------------------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
  |  1 | An open-source fulltext search engine and column store.                                         | ...<span class="w1">fulltext</span>...<br>... <span calss="w3">search</span> ...<br>                                                                                                                   |
  |  2 | An open-source storage engine for fast fulltext search with MySQL.                              | ...<span class="w1">fulltext</span>...<br>... <span calss="w3">search</span> ...<br>                                                                                                                   |
  |  3 | Tritonn is a patched version of MySQL that supports better fulltext search function with Senna. | ...f <span class="w2">MySQL</span> ...<br>...<span class="w1">fulltext</span>...<br>                                                                                                                   |
  +----+-------------------------------------------------------------------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
  3 rows in set (0.00 sec)
  
The keyword 'fulltext' and associated keyword 'MySQL' and 'search' has been extracted.
