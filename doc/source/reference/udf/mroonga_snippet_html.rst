.. highlightlang:: none

``mroonga_snippet_html()``
==========================

.. versionadded:: 5.09

Summary
-------

``mroonga_snippet_html()`` UDF provides functionality to get
highlighted keyword in context.

.. note::

   This feature is in experimental stage.
   So, the required arguments or value is changed without notice in the
   future.

Syntax
------

``mroonga_snippet_html()`` has required parameter and optional parameter::

  mroonga_snippet_html(document, key_word)

Usage
-----

Parameters
----------

Required parameters
^^^^^^^^^^^^^^^^^^^

There are many required parameters.

Optional parameters
^^^^^^^^^^^^^^^^^^^


Return value
------------

It returns snippet string.
