# How to debug

## Building for debugging

When you build software for debugging, you can get more information like symbol resolutions in GDB. So we build both MySQL and Mroonga for debugging in development.

:::{note}
If you build one of them for debugging, the size of structures etc. might be different, and you might not be able to load Mroonga, or assertions don't work in running.
:::

### How to build MySQL for debugging

As you can see in [MySQL :: 2.9.4 Installing MySQL Using a Standard Source Distribution](https://dev.mysql.com/doc/refman/8.4/en/installing-source-distribution.html), you can build MySQL for debugging by passing `-DWITH_DEBUG=ON` option in CMake options.

The procedure from download to build is the following:

```console
$ mkdir -p ~/work/
$ cd ~/work/
$ wget https://cdn.mysql.com/Downloads/MySQL-8.4/mysql-8.4.2.tar.gz
$ tar xf mysql-8.4.2.tar.gz
$ cmake \
    -Smysql-8.4.2 \
    -Bmysql-8.4.2.build \
    -GNinja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_INSTALL_PREFIX=/tmp/local \
    -DWITH_DEBUG=ON
$ cmake --build mysql-8.4.2.build
```

### How to build MariaDB for debugging

If you want to use MariaDB instead of MySQL, you can use `-DWITH_DEBUG=ON` too.

Note that you need to remove `storage/mroonga/` (Mroonga bundled in
MariaDB) before you build MariaDB.

The procedure from download to build is the following:

```console
$ mkdir -p ~/work/
$ cd ~/work/
$ release_id=$(curl -s https://downloads.mariadb.org/rest-api/mariadb/ \
    | jq -r '.major_releases[] | select(.release_support_type == "Long Term Support" and .release_status == "Stable") | .release_id' \
    | head -n1)
$ version=$(curl -s https://downloads.mariadb.org/rest-api/mariadb/$release_id \
    | jq -r '.releases[].release_id' \
    | head -n1)
$ wget https://downloads.mariadb.org/rest-api/mariadb/$version/mariadb-$version.tar.gz
$ tar xf mariadb-$version.tar.gz
$ rm -rf mariadb-$version/storage/mroonga
$ cmake \
    -Smariadb-$version \
    -Bmariadb-$version.build \
    -GNinja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_INSTALL_PREFIX=/tmp/local \
    -DWITH_DEBUG=ON
$ cmake --build mariadb-$version.build
```

### How tom build Mroonga for debugging

You can build Mroonga for debugging by passing `--preset=debug` in CMake options.

:::{note}
In order to build Mroonga, you need to install required tools and libraries beforehand.

See {doc}`/install/others` for the details of dependencies.
:::

The procedure from cloning repository to build is the following:

```console
$ cd ~/work/
$ git clone git@github.com:mroonga/mroonga.git
$ cmake \
    -Smroonga \
    -Bmroonga.mysql-8.4 \
    --preset=debug \
    -DCMAKE_INSTALL_PREFIX=/tmp/local \
    -DMYSQL_BUILD_DIR=$HOME/work/mysql-8.4.2.build \
    -DMYSQL_CONFIG=$HOME/work/mysql-8.4.2.build/scripts/mysql_config \
    -DMYSQL_SOURCE_DIR=$HOME/work/mysql-8.4.2
$ cmake --build mroonga.mysql-8.4
```

When you successfully build both, please invoke tests like the following. If you get `[pass]` for all tests, you succeeded to build for debugging:

```console
$ cd mroonga.mysql-8.4
$ ../mroonga/test/run-sql-test.sh
```

## More about `run-sql-test.sh`

`run-sql-test.sh` is our friend for debugging. Here we show some examples of its usage.

### Run the specified test only

When you invoke `run-sql-test.sh` without any option, all tests under `mysql-test/mroonga` will be invoked.

So if you want to run certain tests only, you can specify the test:

```console
$ ../mroonga/test/run-sql-test.sh ../mroonga/mysql-test/mroonga/storage/t/truncate.test
```

### See the trace

When you run tests by adding `--debug` option like the following, function calls information is recorded:

```console
$ ../mroonga/test/run-sql-test.sh --debug ../mroonga/mysql-test/mroonga/storage/t/truncate.test
```

This information is stored in `${MySQL's build directory}/mysql-test/var/log/mysqld.1.trace`.

When you add a new method, it would be a good idea to put `MRN_DBUG_ENTER_METHOD()`/`DBUG_RETURN()` in the beginning/ending of method and record its calls.

### Invoking GDB

By adding `--manual-gdb` option, you can debug with GDB when you run tests:

```console
$ ../mroonga/test/run-sql-test.sh --manual-gdb ../mroonga/mysql-test/mroonga/storage/t/truncate.test
```

You need to run `gdb` in other terminal for this. The command line to run `gdb` will be showed by `run-sql-test.sh`.
