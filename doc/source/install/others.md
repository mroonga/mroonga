# Others

This section describes how to install Mroonga from source code. If
there is no package no your environment, you need to build Mroonga
from source code.

## Dependencies

Mroonga needs some tools, libraries and MySQL for build. You can use
MariaDB instead of MySQL.

### Tools

Here are required tools.

- `wget`, `curl` or Web browser for downloading source archive
- `tar` and `gzip` for extracting source archive
- shell (many shells such as `dash`, `bash` and `zsh` will work)
- C compiler and C++ compiler (`gcc` and `g++` are supported but other compilers may work)
- [CMake](https://cmake.org/) as a cross-platform build system generator
- [Ninja](https://ninja-build.org/) as a small build system with a focus on speed
- [pkg-config](http://www.freedesktop.org/wiki/Software/pkg-config) for detecting libraries

You must get them ready.

Here are optional tools.

- `sudo` for installing built Mroonga

### Libraries

Here are required libraries.

- [Groonga](https://groonga.org/)
  - If you use package, install development package such as `libgroonga-dev` for deb or `groonga-devel` for RPM
- [groonga-normalizer-mysql](https://github.com/groonga/groonga-normalizer-mysql)

Here are optional libraries.

- [MeCab](https://taku910.github.io/mecab/): Japanese morphological analysis system

```{note}
If you want to use indexes of tokenizing of each
morpheme for full text search, install MeCab before
installing Groonga.
```

### MySQL

Mroonga needs not only installed MySQL but also MySQL source and build
directory. You can't use MySQL package. It doesn't provide MySQL
source and build directory. You need MySQL source and build directory!

If you use MariaDB instead of MySQL, you need MariaDB source.

Download the latest MySQL 8.4 source code, then build and install it.

See also [Download MySQL Community Server](http://dev.mysql.com/downloads/mysql/)

Here we assume that you use mysql-8.4.1 and its source code is
extracted in the following directory.

```
${HOME}/local/src/mysql-8.4.1
```

Then build in the following directory.

```
${HOME}/local/build/mysql-8.4.1
```

Here are command lines to build and install MySQL.

```console
$ cmake \
    -S ${HOME}/local/src/mysql-8.4.1 \
    -B ${HOME}/local/build/mysql-8.4.1 \
    -GNinja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=${HOME}/local
$ cmake --build ${HOME}/local/build/mysql-8.4.1
$ cmake --install ${HOME}/local/build/mysql-8.4.1
```

You need to run MySQL before you install Mroonga. Because you need to
run some SQL statements to register Mroonga.

### MariaDB

You can use MariaDB instead of MySQL.

Note that you need to remove `storage/mroonga/` (Mroonga bundled in MariaDB) before you build MariaDB.

```console
$ mkdir -p ${HOME}/local/src
$ cd ${HOME}/local/src
$ wget https://downloads.mariadb.org/rest-api/mariadb/11.4.3/mariadb-11.4.3.tar.gz
$ tar xf mariadb-11.4.3.tar.gz
$ rm -rf mariadb-11.4.3/storage/mroonga
$ cd -
$ cmake \
    -S ${HOME}/local/src/mariadb-11.4.3 \
    -B ${HOME}/local/build/mariadb-11.4.3 \
    -GNinja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=${HOME}/local
$ cmake --build ${HOME}/local/build/mariadb-11.4.3
$ cmake --install ${HOME}/local/build/mariadb-11.4.3
```

## Build from source

Mroonga uses CMake. So the following is the simplest build steps.

```console
$ cd ${HOME}/local/src
$ wget https://packages.groonga.org/source/mroonga/mroonga-latest.tar.gz
$ tar xvf mroonga-latest.tar.gz
$ mroonga_base_name=$(find mroonga-* -maxdepth 0 -type d)
$ cmake \
    -S ${HOME}/local/src/${mroonga_base_name} \
    -B ${HOME}/local/build/${mroonga_base_name} \
    -GNinja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=${HOME}/local \
    -DMYSQL_BUILD_DIR=${HOME}/local/build/mysql-8.4.1 \
    -DMYSQL_CONFIG=${HOME}/local/bin/mysql_config \
    -DMYSQL_SOURCE_DIR=${HOME}/local/src/mysql-8.4.1
$ cmake --build ${HOME}/local/build/${mroonga_base_name}
$ cmake --install ${HOME}/local/build/${mroonga_base_name}
$ ${HOME}/local/bin/mysql -u root < ${HOME}/local/share/mroonga/install.sql
```

You need to specify the following on `cmake`.

- `-DMYSQL_BUILD_DIR`: The location of MySQL build directory
- `-DMYSQL_CONFIG`: The path of `mysql_config` command
- `-DMYSQL_SOURCE_DIR`: The location of MySQL source code

You can confirm Mroonga is installed successfully by `SHOW ENGINES`
SQL. If you can find `Mroonga` row, Mroonga is installed
successfully.

```
mysql> SHOW ENGINES;
+------------+---------+------------------------------------------------------------+--------------+------+------------+
| Engine     | Support | Comment                                                    | Transactions | XA   | Savepoints |
+------------+---------+------------------------------------------------------------+--------------+------+------------+
| Mroonga    | YES     | Fulltext search, column base                               | NO           | NO   | NO         |
| MRG_MYISAM | YES     | Collection of identical MyISAM tables                      | NO           | NO   | NO         |
| CSV        | YES     | CSV storage engine                                         | NO           | NO   | NO         |
| MyISAM     | DEFAULT | Default engine as of MySQL 3.23 with great performance     | NO           | NO   | NO         |
| InnoDB     | YES     | Supports transactions, row-level locking, and foreign keys | YES          | YES  | YES        |
| MEMORY     | YES     | Hash based, stored in memory, useful for temporary tables  | NO           | NO   | NO         |
+------------+---------+------------------------------------------------------------+--------------+------+------------+
6 rows in set (0.00 sec)
```

The following describes details about each step.

(source-cmake)=

### `cmake`

First, you need to run `cmake`. Here are important `cmake`
parameters.

#### `-DMYSQL_SOURCE_DIR=PATH`

Specifies the location of MySQL source code.

This is required parameter.

#### `-DMYSQL_BUILD_DIR=PATH`

Specifies the location where you build MySQL source code.

If you build MySQL in MySQL source code directory, you don't need to
specify this parameter. If you build MySQL in other directory, you
need to specify this parameter.

#### `-DMYSQL_CONFIG=PATH`

Specifies the path of `mysql_config` command.

If `mysql_config` command can be found by `PATH`, you don't
need to specify this parameter. For example, if `mysql_config`
command exists at `/usr/bin/mysql_config`, you don't need to specify
this parameter.

#### `-DMRN_DEFAULT_TOKENIZER=TOKENIZER`

Specifies the default tokenizer for full text. You can custom it in
`my.cnf`.

The default is `TokenBigram`.

Here is an example to use `TokenMecab` as the default tokenizer:

```console
$ cmake ... -DMRN_DEFAULT_TOKENIZER=TokenMecab
```

#### `-DCMAKE_BUILD_TYPE={Release,Debug,RelWithDebInfo}`

Specifies the build type.

If you use this Mroonga as a production environment, you must use
`-DCAMKE_BUILD_TYPE=Release` or
`-DCAMKE_BUILD_TYPE=RelWithDebInfo`. They enable optimization.

If you use this Mroonga for development, you must use
`-DCAMKE_BUILD_TYPE=Debug`. It disables optimization and enables debug
symbols. They are useful for development.

#### `-DCMAKE_INSTALL_PREFIX=PREFIX`

Specifies the install base directory. Mroonga related files are
installed under `${PREFIX}/` directory except
`ha_mroonga.so`. `ha_mroonga.so` is a MySQL plugin file. It is
installed the plugin directory of MySQL.

The default is `/usr/local`. In this case, `install.sql` that is
used for installing Mroonga is installed to
`/usr/local/share/mroonga/install.sql`.

### `cmake --build`

If `cmake` is succeeded, you can build Mroonga by `cmake --build`.

If you get some errors by `cmake --build`, please report them to us:
{doc}`/contribution/report`

### `cmake --install`

Now, you can install built Mroonga!

If you don't have write permission for `${PREFIX}` and the plugin
directory of MySQL, you need to use `sudo`:

```console
$ sudo cmake --install ${HOME}/local/build/${mroonga_base_name}
```

### `mysql -u root < install.sql`

You need to run some SQL statements to register Mroonga to MySQL such
as `INSTALL PLUGIN` and `CREATE FUNCTION`. They are written in
`${PREFIX}/share/mroonga/install.sql`.

### Uninstall Mroonga

If you want to uninstall Mroonga, use the following command lines:

```console
$ ${HOME}/local/bin/mysql -u root < ${PREFIX}/share/mroonga/uninstall.sql
$ xargs rm < ${HOME}/local/build/mroonga/install_manifest.txt
```
