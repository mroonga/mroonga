Search and Scoring in Mroonga
=============================

Mroonga Scoring in natural language mode
----------------------------------------

The search score of Mroonga in natural language mode is a similarity score between query and document. Mroonga's scoring algorithm is as follows:

  1. Splitting a query into tokens.
  2. Removing not matched tokens.
  3. Calculating weight per token.
  4. Getting the top N weight tokens.
  5. Summing up the weight per token which exists in a document, and is one of the top N weight tokens. The total weight is the similarity score for the document and the query.

We are showing by example.
The first thing we create a table and insert data as follows: ::

  SET NAMES UTF8;
  CREATE TABLE diaries (
    id INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
    content TEXT,
    FULLTEXT INDEX(content)
  ) ENGINE mroonga DEFAULT CHARSET UTF8;

  INSERT INTO diaries (content) VALUES("It'll be fine tomorrow as well.");
  INSERT INTO diaries (content) VALUES("It'll rain tomorrow.");
  INSERT INTO diaries (content) VALUES("It's fine today. It'll be fine tomorrow as well.");
  INSERT INTO diaries (content) VALUES("It's fine today. But it'll rain tomorrow.");

We use a query "fine today" to search.
The search result is as follows: ::

  mysql> SELECT *, MATCH (content) AGAINST ("fine today") AS score
      ->        FROM diaries
      ->        WHERE MATCH (content) AGAINST ("fine today")
      ->        ORDER BY MATCH (content) AGAINST ("fine today") DESC;
  +----+--------------------------------------------------+--------+
  | id | content                                          | score  |
  +----+--------------------------------------------------+--------+
  |  3 | It's fine today. It'll be fine tomorrow as well. | 131073 |
  |  4 | It's fine today. But it'll rain tomorrow.        | 131073 |
  +----+--------------------------------------------------+--------+
  2 rows in set (0.01 sec)

Now, let us explain how to make the result score "131073".


Splitting a query into tokens
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


The default tokenizer splits up the query "fine today" into two tokens as follows:

* fine
* today

Removing not matched tokens
^^^^^^^^^^^^^^^^^^^^^^^^^^^

We have nothing to do in this case because the tokens exist in documents.

* fine: include in document id=1,3,4
* today: include in document id=3,4

Calculating weight per token
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

* fine: 116508(= 1048576 / 9)
* today: 131072(= 1048576 / 8)

1048576(= 2 ** 20) denotes the number of tokens in all documents. We should get the number from the table, but as a matter of fact, the fixed value is used for simplicity.

In the fraction 1048576 / 8,  the denominator "8" indicates that the number of document which contains token "today".

In the fraction 1048576 / 9,  the denominator "9" means the number of document which contains token "fine".

The denominator "8" and "9" are approximate number, strictly speaking, "2" is the correct answer for the number of document that contains "today", "3" is the correct answer for the number of document that contains "fine".

You can check the approximate number of token by using the following query. ::

  SELECT mroonga_command("select diaries-content --query '_key:fine OR _key:today' --output_columns _key, index --limit -1") AS groonga_response;

The retrieval result of the above query is as follows: ::

  [[[2],[["_key","ShortText"],["index","diaries"]],["FINE",9],["TODAY",8]]]

Getting the top N weight tokens
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The formula for the N is "the number of occurrence of token in document / 8 + 1"

In this case, N = 2 / 8 + 1 ≒ 1

The sorted tokens are as follows:

* today: 131072(= 1048576 / 8)
* fine: 116508(= 1048576 / 9)

N = 1, then we get "today: 131072(= 1048576 / 8)".

Summing up the weight per token which exists in a document, and is one of the top N weight tokens
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

* today: include in document id=3,4

Finally document id 3,4 are hit, the similarity score between query and document(id=3) is 131072 + 1 = 131073 ("1" is the number of occurrence of token "today" in document).

The similarity score between query and document(id=4) is the same as the score between query and document(id=3).
