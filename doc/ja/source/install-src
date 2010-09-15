.. highlightlang:: none

チュートリアル
===============================

ダウンロード
----------------------------

現在はsource tarballのみ配布しています。

以下のページからダウンロード可能です。

http://github.com/mroonga/mroonga/downloads

インストール
----------------------------

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

