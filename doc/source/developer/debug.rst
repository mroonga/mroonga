.. highlightlang:: none

How to debug
============

Building for debugging
----------------------

When you build software for debugging, you can get more information like symbol resolutions in gdb.
So we build both MySQL and Mroonga for debugging in development.

.. note::

   If you build one of them for debugging, the size of structures etc. might be different, and you might not be able to load mroonga, or assertions don't work in running.

How to build MySQL for debugging
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

As you can see in `MySQL :: MySQL 5.5 Reference Manual :: 2.9.2 Installing MySQL from a Standard Source Distribution`_, you can build MySQL for debugging by passing ``-DWITH_DEBUG=yes`` option in CMAKE options.

The procedure from download to build is the following. ::

  % mkdir -p ~/work/
  % cd ~/work/
  % wget http://ftp.jaist.ac.jp/pub/mysql/Downloads/MySQL-5.5/mysql-5.5.13.tar.gz
  % tar xvzf mysql-5.5.13.tar.gz
  % cd mysql-5.5.13
  % cmake . -DCMAKE_INSTALL_PREFIX=/tmp/local -DWITH_DEBUG=yes
  % make

.. _`MySQL :: MySQL 5.5 Reference Manual :: 2.9.2 Installing MySQL from a Standard Source Distribution`: http://dev.mysql.com/doc/refman/5.5/en/installing-source-distribution.html

How tom build mroonga for debugging
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

You can build mroonga for debugging by passing ``--with-debug`` in configure options.

The procedure from cloning repository to build is the following. ::

  % cd ~/work/
  % git clone git@github.com:mroonga/mroonga.git
  % cd mroonga
  % ./autogen.sh
  % ./configure CFLAGS="-g3 -O0" CXXFLAGS="-g3 -O0" --with-debug --prefix=/tmp/local --with-mysql-source=$HOME/work/mysql-5.5.13 --with-mysql-config=$HOME/work/mysql-5.5.13/scripts/mysql_config
  % make

When you successfully build both, please invoke tests like the following.
If you get ``[pass]`` for all tests, you succeeded to build for debugging. ::

  % test/run-sql-test.sh

More about run-sql-test.sh
--------------------------

run-sql-test.sh is our friend for debugging.
Here we show some examples of its usage.

Run the specified test only
^^^^^^^^^^^^^^^^^^^^^^^^^^^

When you invoke run-sql-test.sh without any option, all tests under ``test/sql/t/`` will be invoked.

So if you want to run certain tests only, you can specify the test name in --do-test option. ::

  ./test/run-sql-test.sh --do-test=foobar

See the trace
^^^^^^^^^^^^^

When you run tests by adding ``--debug`` option like the following, function calls information is recorded.
This information is stored in ``${MySQL's working directory}/${MySQL version}/mysql-test/var/log/mysqld.1.trace``.

When you add a new function, it would be a good idea to put it in the beginning of MRN_DBUG_ENTER_FUNCTION function and record its calls.

Invoking GDB
^^^^^^^^^^^^

By adding ``--gdb`` option, you can debug with GDB when you run tests. ::

  ./test/run-sql-test.sh --gdb
