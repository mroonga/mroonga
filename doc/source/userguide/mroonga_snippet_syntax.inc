.. note::

   This feature is in experimental stage.
   So, the required arguments or value is changed without notice in the
   future.

There is a case that you want to extract keyword and surrounding text as a
search results.

Snippet means 'keyword and surrounding text'.
It is called 'Keyword in context'.

``mroonga_snippet`` function provides the way to get snippet from search results.

Here is the syntax of ``mroonga_snippet`` function::

  SELECT mroonga_snippet(document, max_length, max_count, encoding,
    skip_leading_spaces, html_escape, prefix, suffix,
    word1, word1_prefix, word1_suffix,
    word2, word2_prefix, word2_suffix, ...);

Here is the detail of ``mroonga_snippet`` arguments.

document
  The column name or string value is required.

max_length
  The max length of snippet (bytes) is required.

max_count
  The max elements of snippets (N word) is required.

encoding
  The encoding of document is required.
  You can specify the value of encoding such as 'ascii_general_ci',
  'cp932_japanese_ci', 'eucjpms_japanese_ci', 'utf8_japanese_ci' and so on.

skip_leading_spaces
  Specify whether skip leading spaces or not.
  Specify the value 1 for skipping leading spaces, 0 for not.

html_escape
  HTML escape is enabled or not.
  Specify the value 1 for enabling HTML escape, 0 for not.

prefix
  The start text of snippet.

suffix
  The end text of snippet.

wordN
  Specify any word.

wordN_prefix
  It is the start text of wordN.

wordN_suffix
  It is the end text of wordN.

mroonga_snippet function is included in mroonga as a User-Defined Function (UDF), but if you have not yet register it in MySQL by CREATE FUNCTION, you need to invoke the following SQL for defining a function. ::

  mysql> CREATE FUNCTION mroonga_snippet RETURNS STRING SONAME 'ha_mroonga.so';

``mroonga_snippet`` function is useful for searching the text which contains keyword and associated one
by using MATCH .. AGAINST syntax.

Imagine searching the document which contains 'fulltext' as a keyword.
Assume that some keyword such as 'MySQL' and 'search' are associated with 'fulltext'.

``mroonga_snippet`` function meets above.

