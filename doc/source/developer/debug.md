# How to debug

## Building for debugging

When you build software for debugging, you can get more information like symbol resolutions in gdb. So we build both MySQL and Mroonga for debugging in development.

:::{note}
If you build one of them for debugging, the size of structures etc. might be different, and you might not be able to load Mroonga, or assertions don't work in running.
:::

### How to build MySQL for debugging

As you can see in [MySQL :: 2.9.4 Installing MySQL Using a Standard Source Distribution](https://dev.mysql.com/doc/refman/8.0/en/installing-source-distribution.html), you can build MySQL for debugging by passing `-DWITH_DEBUG=yes` option in CMAKE options.

The procedure from download to build is the following:

```console
$ mkdir -p ~/work/
$ cd ~/work/
$ wget https://cdn.mysql.com/Downloads/MySQL-8.0/mysql-boost-8.0.29.tar.gz
$ tar xvzf mysql-boost-8.0.29.tar.gz
$ mkdir mysql-8.0.29-build
$ cd mysql-8.0.29-build
$ cmake ../mysql-8.0.29 \
    -DCMAKE_INSTALL_PREFIX=/tmp/local \
    -DWITH_DEBUG=yes \
    -DWITH_BOOST=../mysql-8.0.29/boost
$ make
```

### How tom build Mroonga for debugging

You can build Mroonga for debugging by passing `--with-debug` in configure options.

:::{note}
In order to build Mroonga, you need to install required tools and libraries beforehand.

See {doc}`/install/others` for the details of dependencies.
:::

The procedure from cloning repository to build is the following:

```console
$ cd ~/work/
$ git clone git@github.com:mroonga/mroonga.git
$ cd mroonga
$ ./autogen.sh
$ ./configure --with-debug \
              --prefix=/tmp/local \
              --with-mysql-source=$HOME/work/mysql-8.0.29 \
              --with-mysql-config=$HOME/work/mysql-8.0.29-build/scripts/mysql_config \
              --with-mysql-build=$HOME/work/mysql-8.0.29-build
$ make
```

When you successfully build both, please invoke tests like the following. If you get `[pass]` for all tests, you succeeded to build for debugging:

```console
$ test/run-sql-test.sh
```

## More about run-sql-test.sh

run-sql-test.sh is our friend for debugging. Here we show some examples of its usage.

### Run the specified test only

When you invoke run-sql-test.sh without any option, all tests under `mysql-test/mroonga` will be invoked.

So if you want to run certain tests only, you can specify the test name in `--do-test` option:

```console
$ ./test/run-sql-test.sh --do-test=foobar
```

### See the trace

When you run tests by adding `--debug` option like the following, function calls information is recorded:

```console
$ ./test/run-sql-test.sh --debug --do-test=foobar
```

This information is stored in `${MySQL's working directory}/${MySQL version}/mysql-test/var/log/mysqld.1.trace`.

When you add a new function, it would be a good idea to put it in the beginning of MRN_DBUG_ENTER_FUNCTION function and record its calls.

### Invoking GDB

By adding `--gdb` option, you can debug with GDB when you run tests:

```console
$ ./test/run-sql-test.sh --gdb
```
