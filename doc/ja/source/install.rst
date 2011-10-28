.. highlightlang:: none

インストールガイド
==================

バイナリパッケージを使用したインストールではgroonga関連パッケージと共にMySQL関連のパッケージもインストールされます。

それぞれの環境毎にインストール方法を説明します。

Debian GNU/Linux squeeze
------------------------

.. note::

   amd64版のみ提供でi386版は未提供。

/etc/apt/sources.list.d/groonga.list::

  deb http://packages.groonga.org/debian/ squeeze main
  deb-src http://packages.groonga.org/debian/ squeeze main

インストール::

  % sudo apt-key adv --recv-keys --keyserver keyserver.ubuntu.com 1C837F31
  % sudo aptitude update
  % sudo aptitude -V -D -y install mysql-server-groonga

Debian GNU/Linux wheezy
-----------------------

.. note::

   amd64版のみ提供でi386版は未提供。

/etc/apt/sources.list.d/groonga.list::

  deb http://packages.groonga.org/debian/ wheezy main
  deb-src http://packages.groonga.org/debian/ wheezy main

インストール::

  % sudo apt-key adv --recv-keys --keyserver keyserver.ubuntu.com 1C837F31
  % sudo aptitude update
  % sudo aptitude -V -D -y install mysql-server-groonga

Debian GNU/Linux sid
--------------------

.. note::

   amd64版のみ提供でi386版は未提供。

/etc/apt/sources.list.d/groonga.list::

  deb http://packages.groonga.org/debian/ unstable main
  deb-src http://packages.groonga.org/debian/ unstable main

インストール::

  % sudo apt-key adv --recv-keys --keyserver keyserver.ubuntu.com 1C837F31
  % sudo aptitude update
  % sudo aptitude -V -D -y install mysql-server-groonga

Ubuntu 10.04 LTS Lucid Lynx
---------------------------

.. note::

   amd64版のみ提供でi386版は未提供。

.. note::

   Ubuntu本家のuniverseセクションもインストール対象としておくこと

/etc/apt/sources.list.d/groonga.list::

  deb http://packages.groonga.org/ubuntu/ lucid universe
  deb-src http://packages.groonga.org/ubuntu/ lucid universe

インストール::

  % sudo apt-key adv --recv-keys --keyserver keyserver.ubuntu.com 1C837F31
  % sudo aptitude update
  % sudo aptitude -V -D -y install mysql-server-groonga

Ubuntu 11.04 Natty Narwhal
--------------------------

.. note::

   amd64版のみ提供でi386版は未提供。

.. note::

   Ubuntu本家のuniverseセクションもインストール対象としておくこと

/etc/apt/sources.list.d/groonga.list::

  deb http://packages.groonga.org/ubuntu/ natty universe
  deb-src http://packages.groonga.org/ubuntu/ natty universe

インストール::

  % sudo apt-key adv --recv-keys --keyserver keyserver.ubuntu.com 1C837F31
  % sudo aptitude update
  % sudo aptitude -V -D -y install mysql-server-groonga

Ubuntu 11.10 Oneiric Ocelot
---------------------------

.. note::

   amd64版のみ提供でi386版は未提供。

.. note::

   Ubuntu本家のuniverseセクションもインストール対象としておくこと

/etc/apt/sources.list.d/groonga.list::

  deb http://packages.groonga.org/ubuntu/ oneiric universe
  deb-src http://packages.groonga.org/ubuntu/ oneiric universe

インストール::

  % sudo apt-key adv --recv-keys --keyserver keyserver.ubuntu.com 1C837F31
  % sudo aptitude update
  % sudo aptitude -V -D -y install mysql-server-groonga

CentOS 5
--------

.. note::

   amd64版のみ提供でi386版は未提供。

既にディストリビューション由来のMySQLパッケージがインストール済みの場合には事前に削除する必要があります。

既存のMySQLパッケージを削除::

  % sudo yum remove mysql*

インストール::

  % sudo rpm -ivh http://packages.groonga.org/centos/groonga-repository-1.0.0-0.noarch.rpm
  % sudo yum update
  % sudo yum install -y mysql-groonga

CentOS 6
--------

.. note::

   amd64版のみ提供でi386版は未提供。

CentOS 6用のパッケージはCentOS 5用のパッケージと違い、ディストリビューション由来のMySQLパッケージ（MySQL 5.1系）を利用します。そのため、ディストリビューション由来のMySQLを削除する必要はありません。

インストール::

  % sudo rpm -ivh http://packages.groonga.org/centos/groonga-repository-1.0.0-0.noarch.rpm
  % sudo yum update
  % sudo yum install -y mysql-groonga

Fedora 15
---------

.. note::

   amd64版のみ提供でi386版は未提供。

インストール::

  % sudo rpm -ivh http://packages.groonga.org/fedora/groonga-repository-1.0.0-0.noarch.rpm
  % sudo yum update
  % sudo yum install -y mysql-groonga

ソースコードからのインストール
------------------------------

ソースコードからインストールする方法を説明します。パッケージ
がない環境ではソースコードからインストールすることになります。

形態素解析(MeCab)について
+++++++++++++++++++++++++

形態素単位でトークナイズした全文検索索引を使用したい場合は、
groongaのインストール前に `MeCab <http://mecab.sourceforge.net/>`_
をインストールしてください。

ダウンロード
++++++++++++

リリース版のソースコードを利用する場合は `GitHubのダウンロードページ <http://github.com/mroonga/mroonga/downloads>`_ からtarballをダウンロードしてください。

最新のソースコードを利用する場合は `GitHub <https://github.com/mroonga/mroonga/>`_ からcloneして `./autogen.sh` を実行してください。（GNU Autotoolsが必要です。）この方法は開発に慣れた方向けなので、そうでない方はtarballを使うことをお勧めします。::

 % git clone https://github.com/mroonga/mroonga.git
 % mroonga
 % ./autogen.sh

前提条件
++++++++

MySQLおよびgroongaが既にインストールされている必要があります。

またgroongaストレージエンジンをビルドするためにはMySQLのソースコードも必要です。

MySQLのインストール
+++++++++++++++++++

MySQL 5.5最新版のソースコードをダウンロードし、ビルド＆インストールして下さい。

http://dev.mysql.com/downloads/mysql/

mysql-5.5.17を使用し、以下にソースディレクトリが展開されているものと仮定します。 ::

 /usr/local/src/mysql-5.5.17

MySQLのバイナリが以下にインストールされているものと仮定します。 ::

 /usr/local/mysql

groongaのインストール
+++++++++++++++++++++

groongaの最新版をビルド＆インストールして下さい。

http://groonga.org/docs/

ここでは/usr/libなどの標準パスにlibgroongaがインストールされているものと仮定します。

groongaストレージエンジンのビルド
+++++++++++++++++++++++++++++++++

以下のように ``--with-mysql-source`` でMySQLソースコードディレクトリ、 ``--with-mysql-config`` でmysql_configコマンドのパスを指定してconfigureを実行します。 ::

 ./configure \
   --with-mysql-source=/usr/local/src/mysql-5.5.17 \
   --with-mysql-config=/usr/local/mysql/bin/mysql_config

groongaを/usr/libなど標準のパス以外にインストールした場合はPKG_CONFIG_PATHを指定する必要があります。例えば、ｰｰprefix=$HOME/localでgroongaをインストールした場合は以下のようにします。::

 ./configure \
   PKG_CONFIG_PATH=$HOME/local/lib/pkgconfig \
   --with-mysql-source=/usr/local/src/mysql-5.5.17 \
   --with-mysql-config=/usr/local/mysql/bin/mysql_config

その後、"make"を実行します。 ::

 make

groongaストレージエンジンのインストール
+++++++++++++++++++++++++++++++++++++++

"make install"を実行するとMySQLのプラグイン用ディレクトリにha_groonga.soが配置されます。 ::

 make install

その後、mysqldを起動し、mysqlクライアントで接続して"INSTALL PLUGIN"コマンドでインストールします。 ::

 mysql> INSTALL PLUGIN groonga SONAME 'ha_groonga.so';

以下のように"SHOW ENGINES"コマンドで"groonga"が表示されればgroongaストレージエンジンのインストールは完了です。 ::

 mysql> SHOW ENGINES;
 +------------+---------+------------------------------------------------------------+--------------+------+------------+
 | Engine     | Support | Comment                                                    | Transactions | XA   | Savepoints |
 +------------+---------+------------------------------------------------------------+--------------+------+------------+
 | groonga    | YES     | Fulltext search, column base                               | NO           | NO   | NO         |
 | MRG_MYISAM | YES     | Collection of identical MyISAM tables                      | NO           | NO   | NO         |
 | CSV        | YES     | CSV storage engine                                         | NO           | NO   | NO         |
 | MyISAM     | DEFAULT | Default engine as of MySQL 3.23 with great performance     | NO           | NO   | NO         |
 | InnoDB     | YES     | Supports transactions, row-level locking, and foreign keys | YES          | YES  | YES        |
 | MEMORY     | YES     | Hash based, stored in memory, useful for temporary tables  | NO           | NO   | NO         |
 +------------+---------+------------------------------------------------------------+--------------+------+------------+
 6 rows in set (0.00 sec)

続いてUDF(ユーザ定義関数)をインストールします。

INSERTを行った際にgroongaにより割当てられるレコードIDを取得するためのlast_insert_grn_id関数をインストールします。

以下のようにCREATE FUNCTIONを実行します。 ::

 mysql> CREATE FUNCTION last_insert_grn_id RETURNS INTEGER soname 'ha_groonga.so';
