``mroonga_command()``
=====================

.. versionadded:: 3.02

Summary
-------

``mroonga_command()`` UDF executes the given string as a Groonga command
and returns result of the Groonga command. Groonga command will be
faster than MySQL query.

``mroonga_command()`` is an UDF for advanced users. Normally, you
don't need to use this UDF.

.. warning::

   ``mroonga_command()`` sends the given command to Groonga
   directly. It bypasses the privilege control of MySQL/MariaDB. Don't
   register this UDF on a multi-user system that relies on privilege
   separation of accounts.

Install
-------

.. versionchanged:: 16.13

   ``install.sql`` doesn't register ``mroonga_command()``.

If you need ``mroonga_command()``, use ``install_mroonga_command.sql``
in the same directory as ``install.sql``::

  $ mysql -u root < /usr/share/mroonga/install_mroonga_command.sql

Syntax
------

``mroonga_command()`` has only one required parameter::

  mroonga_command(command)

``command`` is a string value. It's a Groonga command to be executed.

Usage
-----

TODO

Parameters
----------

Required parameters
^^^^^^^^^^^^^^^^^^^

There is one required parameter, ``command``.

``command``
"""""""""""

It specifies a Groonga command to be executed.

Return value
------------

It returns an evaluated result of the given Groonga command as a string.

See also
--------

`Command <https://groonga.org/docs/reference/command.html>`_ in Groonga document.
