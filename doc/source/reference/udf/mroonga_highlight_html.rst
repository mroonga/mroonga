.. highlight:: none

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

  mroonga_highlight_html(text, query AS query)

  mroonga_highlight_html(text, keyword1, ..., keywordN)

``AS query`` is very important. You must specify it to extract keywords from query.

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

Return value
------------

It returns highlighted HTML. If optional parameter is not given, it
only escapes special characters in HTML such as ``<``, ``>`` in
``text``.
