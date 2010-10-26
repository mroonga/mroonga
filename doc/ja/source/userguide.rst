.. highlightlang:: none

.. _userguide:

ユーザガイド
===============================

インストール方法についてはこちらを参照して下さい: :ref:`install`

インストール後の動作確認
----------------------------

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

全文検索の利用方法
----------------------------

インストールが確認できたら、テーブルを1つ作成してみましょう。 ::

 mysql> CREATE TABLE t1 (c1 INT PRIMARY KEY, c2 VARCHAR(255), FULLTEXT INDEX (c2)) ENGINE = groonga DEFAULT CHARSET utf8;
 Query OK, 0 rows affected (0.22 sec)

INSERTでデータを投入してみましょう。 ::

 mysql> INSERT INTO t1 VALUES(1, "明日の天気は晴れでしょう。");
 Query OK, 1 row affected (0.01 sec)
 
 mysql> INSERT INTO t1 VALUES(2, "明日の天気は雨でしょう。");
 Query OK, 1 row affected (0.04 sec)

全文検索を実行してみます。 ::

 mysql> SELECT * FROM t1 WHERE MATCH(c2) AGAINST("晴れ");
 +----+-----------------------------------------+
 | c1 | c2                                      |
 +----+-----------------------------------------+
 |  1 | 明日の天気は晴れでしょう。 |
 +----+-----------------------------------------+
 1 row in set (0.02 sec)

おぉぉー。検索できましたね。


ログ出力
----------------------------

groongaストレージエンジンではデフォルトでログの出力を行うようになっています。

ログファイルはMySQLのデータディレクトリ直下に ``mroonga.log`` というファイル名で出力されます。

以下はログの出力例です。 ::

 2010-10-07 17:32:39.209379|n|b1858f80|mroonga 0.1 init
 2010-10-07 17:32:44.934048|d|46953940|hash get not found (key=test)
 2010-10-07 17:32:44.936113|d|46953940|hash put (key=test)

現在のgroongaストレージエンジンは開発バージョンということもあり、ログの出力レベルは最大(=DUMP)となっております。

ログの出力レベルの変更や"flush logs"への対応はまだ行われておりませんが、将来のバージョンで対応される予定となっています。

カラムの刈り込み
----------------------------

行カウント高速化
----------------------------
