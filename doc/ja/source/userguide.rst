.. highlightlang:: none

.. _userguide:

ユーザガイド
===============================

インストール方法についてはこちらを参照して下さい: :ref:`install`

インストール確認
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

groongaでは各カラムごとにファイルを分けてデータを格納する「カラムストア方式」が採用されており、groongaストレージエンジンではこの特性を活かすためにテーブルアクセス時に必要なカラムに対してのみアクセスを行う実装を行っています。

この高速化の仕組みはgroongaストレージエンジン内部で自動的に行われるため、特に設定などを行う必要はありません。

例えば以下のようにカラムが20個定義されているテーブルが存在するものと仮定します。 ::

 CREATE TABLE t1 (
   c1 INT PRIMARY KEY AUTO_INCREMENT,
   c2 INT,
   c3 INT,
   ...
   c11 VARCHAR(20),
   c12 VARCHAR(20),
   ...
   c20 DATETIME
 ) ENGINE = InnoDB DEFAULT CHARSET utf8;

この時、以下のようなSELECT文が発行される場合、groongaストレージエンジンではSELECT句およびWHERE句で参照しているカラムに対してのみデータの読み取りを行ってSQL文を処理します（内部的に不要なカラムに対してはアクセスしません）。 ::

 SELECT c1, c2, c11 FROM t1 WHERE c2 = XX AND c12 = "XXX";

このケースではc1,c2,c11,c12に対してのみアクセスが行われ、SQL文が高速に処理されることになります。

行カウント高速化
----------------------------

COUNT(*)などの行カウントを行う場合と通常のSELECTによるデータ参照を行う場合に対して、従来よりMySQLではストレージエンジンの呼び出しを行う部分(=ストレージエンジンインタフェース)における区別が存在していないため、行数をカウントするだけで良いような場合にもレコードアクセス（SELECTの結果には含まれないデータへのアクセス）が行われる問題があります。

groongaストレージエンジンの前身であるTritonn(MySQL+Senna)ではこの問題に対して"2indパッチ"という不要なレコードアクセスを省略する仕組みを独自に実装してこの性能問題を回避していました。

これに引き続き、groongaストレージエンジンでも行カウントを高速化するための仕組みを実装しています。

例えば以下のSELECT文では不要なカラムデータの読み取りは省略され、必要最小限のコストで行カウントの結果を返すことができます。 ::

 SELECT COUNT(*) FROM t1 WHERE MATCH(c2) AGAINST("hoge");

行カウント高速化の処理が行われたかどうかはステータス変数で確認することもできます。::

 mysql> show status like 'groonga_count_skip';
 +--------------------+-------+
 | Variable_name      | Value |
 +--------------------+-------+
 | groonga_count_skip | 1     |
 +--------------------+-------+
 1 row in set (0.00 sec)

行カウント高速化の処理が行われる度に ``groonga_count_skip`` ステータス変数がインクリメントされます。

備考：この高速化機能はインデックスを用いて実装されています。現在のところインデックスアクセスのみでレコードが特定できるパタンでのみ有効に機能します。
