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

- [sudo](http://www.gratisoft.us/sudo/) for installing built Mroonga.

### Libraries

Here are required libraries.

- [Groonga](http://groonga.org/). (If you use package, install development package such as `libgroonga-dev` for deb or `groonga-devel` for RPM.)
- [groonga-normalizer-mysql](https://github.com/groonga/groonga-normalizer-mysql).

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
/usr/local/src/mysql-8.4.1
```

Then build in the following directory.

```
/usr/local/src/mysql-8.4.1.build
```

Here are command lines to build and install MySQL.

```console
% cd /usr/local/src/mysql-8.4.1
% cmake -S . -B ../mysql-8.4.1.build -GNinja
% cmake --build ../mysql-8.4.1.build
% sudo cmake --install ../mysql-8.4.1.build
```

And we assume that MySQL is installed in the following directory.

```
/usr/local/bin/mysql
```

## Build from source

Mroonga uses GNU build system. So the following is the simplest build
steps.

```console
% wget https://packages.groonga.org/source/mroonga/mroonga-6.12.tar.gz
% tar xvzf mroonga-6.12.tar.gz
% cd mroonga-6.12
% ./configure \
    --with-mysql-source=/usr/local/src/mysql-5.6.21 \
    --with-mysql-build=/usr/local/build/mysql-5.6.21 \
    --with-mysql-config=/usr/local/mysql/bin/mysql_config
% make
% sudo make install
% /usr/local/mysql/bin/mysql -u root < /usr/local/share/mroonga/install.sql
```

You need to specify the following on `configure`.

- The location of MySQL source code with `--with-mysql-source`.
- The location of MySQL build directory with `--with-mysql-build`.
- The path of `mysql_config` command with `--with-mysql-config`.

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

(source-configure)=

### `configure`

First, you need to run `configure`. Here are important `configure`
parameters.

#### `--with-mysql-source=PATH`

Specifies the location of MySQL source code.

This is required parameter.

```console
% ./configure \
    --with-mysql-source=/usr/local/src/mysql-5.6.21 \
    --with-mysql-config=/usr/local/mysql/bin/mysql_config
```

#### `--with-mysql-build=PATH`

Specifies the location where you build MySQL source code.

If you build MySQL in MySQL source code directory, you don't need to
specify this parameter. If you build MySQL in other directory, you
need to specify this parameter.

Here is an example when you build MySQL in
`/usr/local/build/mysql-5.6.21`.

```console
% ./configure \
    --with-mysql-source=/usr/local/src/mysql-5.6.21 \
    --with-mysql-build=/usr/local/build/mysql-5.6.21 \
    --with-mysql-config=/usr/local/mysql/bin/mysql_config
```

#### `--with-mysql-config=PATH`

Specifies the path of `mysql_config` command.

If `mysql_config` command can be found by `PATH`, you don't
need to specify this parameter. For example, if `mysql_config`
command exists at `/usr/bin/mysql_config`, you don't need to specify
this parameter.

```console
% ./configure \
    --with-mysql-source=/usr/local/src/mysql-5.6.21
```

#### `--with-default-tokenizer=TOKENIZER`

Specifies the default tokenizer for full text. You can custom it in
my.cnf.

The default is `TokenBigram`.

Here is an example to use `TokenMecab` as the default tokenizer.

```console
% ./configure \
    --with-mysql-source=/usr/local/src/mysql-5.6.21 \
    --with-mysql-config=/usr/local/mysql/bin/mysql_config \
    --with-default-tokenizer=TokenMecab
```

#### `--prefix=PATH`

Specifies the install base directory. Mroonga related files are
installed under `${PATH}/` directory except
`ha_mroonga.so`. `ha_mroonga.so` is a MySQL plugin file. It is
installed the plugin directory of MySQL.

The default is `/usr/local`. In this case, `install.sql` that is
used for installing Mroonga is installed to
`/usr/local/share/mroonga/install.sql`.

Here is an example that installs Mroonga into `~/local` for an user
use instead of system wide use.

```console
% ./configure \
    --prefix=$HOME/local \
    --with-mysql-source=$HOME/local/src/mysql-5.6.21 \
    --with-mysql-config=$HOME/local/mysql/bin/mysql_config
```

#### `PKG_CONFIG_PATH=PATH`

This is not a `configure` parameter but we describe it for users who
doesn't install Groonga into the standard location.

If Groonga is not installed in the standard location like
`/usr/lib`, you need to specify its location by
`PKG_CONFIG_PATH`. For example, if Groonga is installed with
`--prefix=$HOME/local`, use the following command line.

```console
./configure \
  PKG_CONFIG_PATH=$HOME/local/lib/pkgconfig \
  --with-mysql-source=/usr/local/src/mysql-5.6.21 \
  --with-mysql-config=/usr/local/mysql/bin/mysql_config
```

### `make`

`configure` is succeeded, you can build Mroonga by `make`.

```console
% make
```

If you have multi cores CPU, you can make faster by using `-j`
option. If you have 4 cores CPU, it's good for using `-j4` option.

```console
% make -j4
```

If you get some errors by `make`, please report them to us:
{doc}`/contribution/report`

### `make install`

Now, you can install built Mroonga!

```console
% sudo make install
```

If you have write permission for `${PREFIX}` and the plugin
directory of MySQL, you don't need to use
`sudo`. e.g. `--prefix=$HOME/local` case. In this case, use `make
install`.

```console
% make install
```

### `mysql -u root < install.sql`

You need to run some SQLs to register Mroonga to MySQL such as
`INSTALL PLUGIN` and `CREATE FUNCTION`. They are written in
`${PREFIX}/share/mroonga/install.sql`.

Here is an example when you specify `--prefix=$HOME/local` to
`configure`.

```console
% mysql -u root < $HOME/local/share/mroonga/install.sql
```

### `uninstall Mroonga`

If you want to remove Mroonga,
type below commands.

```console
% mysql < ${PREFIX}/share/mroonga/uninstall.sql
% cd ${MROONGA_BUILD_DIR}
% sudo make uninstall
```
