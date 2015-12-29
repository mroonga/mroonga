.. highlightlang:: none

``mroonga_escape()``
====================

Summary
-------

``mroonga_escape`` UDF provides functionality to escape given string.
It also accepts parameter what character should be escaped.

Syntax
------

``mroonga_escape()`` has required parameter and optional parameter::

  mroonga_escape(string)
  mroonga_escape(string, special_characters)

Usage
-----

Here is the example query which use special characters to be escaped::

  SELECT * FROM `symbols` WHERE MATCH(`content`) AGAINST(mroonga_escape("+hello_world()", "()") IN BOOLEAN MODE);

Here is the example about special characters which is escaped::

  SELECT mroonga_escape("+-<>~*()\"\:");
  '\\+\\-\\<\\>\\~\\*\\(\\)\\"\\:


Parameters
----------

Required parameters
^^^^^^^^^^^^^^^^^^^

There is one required parameter, ``string``.

``string``
""""""""""

It specifies text which you want to escape.

Optional parameters
^^^^^^^^^^^^^^^^^^^

There is one optional parameter, ``special_characters``.

``special_characters``
""""""""""""""""""""""

It specifies characters to escape.

The default value is ``+-<>~*()":``.

Return value
------------

It returns escaped string.

