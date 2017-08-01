.. highlightlang:: none

``mroonga_highlight_html()``
============================

.. versionadded:: 7.05

Summary
-------

``mroonga_highlight_html()`` UDF provides functionality to highlight target column or string.

Syntax
------

``mroonga_highlight_html()`` has required parameter and optional parameter::

  mroonga_highlight_html(document, word1, ..., wordN)

  mroonga_highlight_html(document, query)

Usage
-----

Here is the sample query to highlight target text by keyword 'mroonga'::

  SELECT mroonga_highlight_html('Mroonga is the Groonga based storage engine.',
                                'mroonga') AS highlighted;


Here is the results of execution examples::

  +---------------------------------------------------------------------------+
  | highlighted                                                               |
  +---------------------------------------------------------------------------+
  | <span class="keyword">Mroonga</span> is the Groonga based storage engine. |
  +---------------------------------------------------------------------------+
  1 row in set (0.00 sec)

Parameters
----------

Required parameters
^^^^^^^^^^^^^^^^^^^

There is one required parameter.


``document``
""""""""""""
  The column name or string value is required.

Optional parameters
^^^^^^^^^^^^^^^^^^^

There is some optional parameters.

``wordN``
"""""""""

  Specify any word.

``query``
"""""""""

  Specify any keywords and pragma.

Return value
------------

It returns highlighted html. If optional parameter is not given, just returns ``document`` itself.
