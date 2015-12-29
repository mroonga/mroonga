.. highlightlang:: none

``mroonga_command()``
=====================

Summary
-------

``mroonga_command()`` UDF executes the given string as a Groonga command
and returns result of the Groonga command. Groonga command will be
faster than MySQL query.

``mroonga_command()`` is an UDF for advanced users. Normally, you
don't need to use this UDF.

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

  * [Command](http://groonga.org/docs/reference/command.html) in
    Groonga document
