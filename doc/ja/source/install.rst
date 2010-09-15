.. highlightlang:: none

インストール
===============================

それぞれの環境毎にインストール方法を説明します。

Debian GNU/Linux lenny
----------------------

/etc/apt/sources.list.d/groonga.list::

  deb http://packages.groonga.org/debian/ lenny main
  deb-src http://packages.groonga.org/debian/ lenny main

インストール::

  % sudo apt-key adv --recv-keys --keyserver keyserver.ubuntu.com 1C837F31
  % sudo aptitude update
  % sudo aptitude -V -D -y install groonga libgroonga-dev MySQL-client-community mysql-groonga

Debian GNU/Linux squeeze
------------------------

/etc/apt/sources.list.d/groonga.list::

  deb http://packages.groonga.org/debian/ squeeze main
  deb-src http://packages.groonga.org/debian/ squeeze main

インストール::

  % sudo apt-key adv --recv-keys --keyserver keyserver.ubuntu.com 1C837F31
  % sudo aptitude update
  % sudo aptitude -V -D -y install groonga libgroonga-dev MySQL-client-community mysql-groonga

Debian GNU/Linux sid
--------------------

/etc/apt/sources.list.d/groonga.list::

  deb http://packages.groonga.org/debian/ unstable main
  deb-src http://packages.groonga.org/debian/ unstable main

インストール::

  % sudo apt-key adv --recv-keys --keyserver keyserver.ubuntu.com 1C837F31
  % sudo aptitude update
  % sudo aptitude -V -D -y install groonga libgroonga-dev MySQL-client-community mysql-groonga

Ubuntu 8.04 LTS Hardy Heron
---------------------------

注: Ubuntu本家のuniverseセクションもインストール対象としておくこと

/etc/apt/sources.list.d/groonga.list::

  deb http://packages.groonga.org/ubuntu/ hardy universe
  deb-src http://packages.groonga.org/ubuntu/ hardy universe

インストール::

  % sudo apt-key adv --recv-keys --keyserver keyserver.ubuntu.com 1C837F31
  % sudo aptitude update
  % sudo aptitude -V -D -y install groonga libgroonga-dev MySQL-client-community mysql-groonga

Ubuntu 10.04 Lucid Lynx
-----------------------

注: Ubuntu本家のuniverseセクションもインストール対象としておくこと

/etc/apt/sources.list.d/groonga.list::

  deb http://packages.groonga.org/ubuntu/ karmic universe
  deb-src http://packages.groonga.org/ubuntu/ karmic universe

インストール::

  % sudo apt-key adv --recv-keys --keyserver keyserver.ubuntu.com 1C837F31
  % sudo aptitude update
  % sudo aptitude -V -D -y install groonga libgroonga-dev MySQL-client-community mysql-groonga

CentOS 5
--------

インストール::

  % sudo rpm -ivh http://packages.groonga.org/centos/groonga-repository-1.0.0-0.noarch.rpm
  % sudo yum update
  % sudo yum install -y groonga groonga-tokenizer-mecab groonga-devel

Fedora 13
---------

インストール::

  % sudo rpm -ivh http://packages.groonga.org/fedora/groonga-repository-1.0.0-0.noarch.rpm
  % sudo yum update
  % sudo yum install -y groonga groonga-tokenizer-mecab groonga-devel MySQL-client-community mysql-groonga

形態素解析(mecab)について
-------------------------

形態素単位でトークナイズした全文検索索引を使用したい場合は、
groongaのインストール前にMeCab
(http://mecab.sourceforge.net/)をインストールしてください。

ソースコードからのインストール
------------------------------

ダウンロード
++++++++++++

以下のページからソースのtarballをダウンロード可能です。

http://github.com/mroonga/mroonga/downloads

前提条件
++++++++

MySQLおよびgroongaが既にインストールされている必要があります。

またgroongaストレージエンジンをビルドするためにはMySQLのソースコードも必要です。

MySQLのインストール
+++++++++++++++++++

MySQL 5.1最新版のソースコードをダウンロードし、ビルド＆インストールして下さい。

http://dev.mysql.com/doc/refman/5.1/ja/index.html

mysql-5.1.45を使用し、以下にソースディレクトリが展開されているものと仮定します。 ::

 /usr/local/src/mysql-5.1.45

MySQLのバイナリが以下にインストールされているものと仮定します。 ::

 /usr/local/mysql

groongaのインストール
+++++++++++++++++++++


groongaの最新版をビルド＆インストールして下さい。

http://groonga.org/docs/

ここでは/usr/libなどの標準パスにlibgroongaがインストールされているものと仮定します。

groongaストレージエンジンのビルド
+++++++++++++++++++++++++++++++++

以下のように"with-mysql"でMySQLソースコードディレクトリ、"libdir"でMySQLバイナリのプラグイン用ディレクトリ、"with-groonga"でgroongaのインストール先を指定してconfigureを実行します。 ::

 ./configure \
   --with-mysql=/usr/local/src/mysql-5.1.45 \
   --libdir=/usr/local/mysql/lib/mysql/plugin \
   --with-groonga=/usr

その後、"make"を実行します。 ::

 make

groongaストレージエンジンのインストール
+++++++++++++++++++++++++++++++++++++++

"make install"を実行するとMySQLのプラグイン用ディレクトリにlibgroonga_storage_engineが配置されます。 ::

 make install

その後、mysqldを起動し、mysqlクライアントで接続して"INSTALL PLUGIN"コマンドでインストールします。 ::

 mysql> INSTALL PLUGIN groonga SONAME 'libgroonga_storage_engine.so';

以下のように"SHOW ENGINES"コマンドで"groonga"が表示されればインストール完了です。 ::

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

githubからのインストール
------------------------
レポジトリから一式ダウンロードします。 ::

 git clone git@github.com:mroonga/mroonga.git

configureやMakefile.inなどを生成します。GNU Autotoolsが必要です。  ::

 ./autogen.sh

この後のステップは「ソースコードからのインストール」と同じです。

