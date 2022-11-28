``mroonga_highlight_html()``
============================

.. versionadded:: 7.05

Summary
-------

``mroonga_highlight_html()`` highlights the specified keywords in
target text. It surrounds each keyword with ``<span
class="keyword">...</span>`` and special characters in HTML such as
``<`` and ``>`` are escaped. You can use the result as is safely in
HTML.

Syntax
------

``mroonga_highlight_html()`` has required parameter and optional parameter::

  mroonga_highlight_html(text, query AS query [, open_tag AS open_tag, close_tag AS close_tag])

  mroonga_highlight_html(text, keyword1, ..., keywordN [, open_tag AS open_tag, close_tag AS close_tag])

``AS query`` is very important. You must specify it to extract keywords from query.

``open_tag`` and ``close_tag`` are optional. You can specify a tag for highlighting.

Usage
-----

Here is a sample to highlight keywords "mroonga" and "groonga" in
target text by query "mroonga OR groonga". You must specify ``AS
query``::

  SELECT mroonga_highlight_html('Mroonga is the Groonga based storage engine.',
                                'mroonga OR groonga' AS query) AS highlighted;


Here is the result of the execution example::

  +--------------------------------------------------------------------------------------------------------+
  | highlighted                                                                                            |
  +--------------------------------------------------------------------------------------------------------+
  | <span class="keyword">Mroonga</span> is the <span class="keyword">Groonga</span> based storage engine. |
  +--------------------------------------------------------------------------------------------------------+
  1 row in set (0.00 sec)

Here is a sample to highlight keywords "mroonga" and "groonga" in
target text by keywords "mroonga" and "groonga"::

  SELECT mroonga_highlight_html('Mroonga is the Groonga based storage engine.',
                                'mroonga', 'groonga') AS highlighted;


Here is the result of the execution example::

  +--------------------------------------------------------------------------------------------------------+
  | highlighted                                                                                            |
  +--------------------------------------------------------------------------------------------------------+
  | <span class="keyword">Mroonga</span> is the <span class="keyword">Groonga</span> based storage engine. |
  +--------------------------------------------------------------------------------------------------------+

Parameters
----------

Required parameters
^^^^^^^^^^^^^^^^^^^

There is one required parameter.

``text``
""""""""

The column name of string or string value to be highlighted.

Optional parameters
^^^^^^^^^^^^^^^^^^^

There are some optional parameters.

``query``
"""""""""

Specify query in `Groonga's query syntax
<http://groonga.org/docs/reference/grn_expr/query_syntax.html>`_.

You must specify ``AS query`` to extract keywords from query like the
following::

  SELECT mroonga_highlight_html('...', 'mroonga OR groonga' AS query);

``keyword``
"""""""""""

Specify 0 or more keywords to be highlighted.

``open_tag``
""""""""""""

Specify an open tag for highlighting.

You must specify ``AS open_tag`` to specify this parameter like the following.

.. code-block:: SQL
   
   SELECT mroonga_highlight_html('Mroonga is the Groonga based storage engine.', 'groonga', 
                                 '<span class="my-class">' AS open_tag) AS highlighted;
  
   -- +----------------------------------------------------------------------------+
   -- | highlighted                                                                |
   -- +----------------------------------------------------------------------------+
   -- | Mroonga is the <span class="my-class">Groonga</span> based storage engine. |
   -- +----------------------------------------------------------------------------+

The default value is ``<span class="keyword">``.

``close_tag``
"""""""""""""

Specify a close tag for highlighting.

You must specify ``AS close_tag`` to specify this parameter like the following.

.. code-block:: SQL
   
   SELECT mroonga_highlight_html('Mroonga is the Groonga based storage engine.', 'groonga', 
                                 '<mark>' AS open_tag, '</mark>' AS close_tag) AS highlighted;
   
   -- +-----------------------------------------------------------+
   -- | highlighted                                               |
   -- +-----------------------------------------------------------+
   -- | Mroonga is the <mark>Groonga</mark> based storage engine. |
   -- +-----------------------------------------------------------+

The default value is ``</span>``.

Return value
------------

It returns highlighted HTML. If optional parameter is not given, it
only escapes special characters in HTML such as ``<``, ``>`` in
``text``.
