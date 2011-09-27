.. highlightlang:: none

ユーザガイド
============

インストール方法についてはこちらを参照して下さい: :doc:`install`

インストール確認
----------------

MySQLサーバの起動停止方法は通常のMySQLと同じです。

MySQLサーバを起動した上でmysqlコマンドで接続します。パスワードを設定済みの場合は ``-p`` オプションでパスワードを指定して下さい。 ::

 shell> mysql -uroot test

SHOW ENGINESコマンドでgroongaストレージエンジンがインストールされているかどうかを確認します。::

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

上記のように"groonga"ストレージエンジンが見えていればインストールは無事完了しています。

インストールされていなければ以下のようにINSTALL PLUGINコマンドを実行して下さい。 ::

 mysql> INSTALL PLUGIN groonga SONAME 'ha_groonga.so';

動作モード
----------

groongaストレージエンジンは以下の2つの動作モードのどちらかで動作します。

* ストレージモード
* ラッパーモード

ストレージモードでは、全文検索機能だけではなくデータストアも含めてgroongaの機能を利用します。ストレージエンジンのすべての機能をgroongaストレージエンジンで実現するため、groongaが得意としている集計操作が高速である、 ``groonga`` コマンドで直接データベースを操作できるという特長があります。

ストレージモードの構成を図で表すと以下のようになります。MyISAMやInnoDBなど既存のストレージエンジンの代わりに利用します。

.. figure:: /images/storage-mode.png
   :alt: ストレージモード
   :align: center


ラッパーモードでは全文検索機能のみgroongaの機能を利用し、データストアはInnoDBなど既存のストレージエンジンを利用します。ラッパーモードを利用することにより、ストレージエンジンとして多くの利用実績のあるInnoDBに全文検索エンジンとして実績のあるgroongaを組み合わせて、高速な全文検索機能付きの信頼性のあるデータベースとして利用できるという特長があります。

ラッパーモードの構成を図で表すと以下のようになります。全文検索関連の処理はgroongaストレージエンジンで処理し、それ以外の処理はMyISAMやInnoDBなど既存のストレージエンジンを利用します。SQLを処理するSQL Handlerと既存のストレージエンジンの間に位置するため、すべてのデータが一度groongaストレージエンジンを通ることになります。これを利用して、全文検索用のインデックス作成などを透過的に行います。

.. figure:: /images/wrapper-mode.png
   :alt: ラッパーモード
   :align: center

モード毎の利用方法
------------------

各モード毎の利用方法に関しては、以下のページを参照して下さい。

.. toctree::
   :maxdepth: 2

   userguide/storage.rst
   userguide/wrapper.rst
