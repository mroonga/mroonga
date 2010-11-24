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

 mysql> CREATE TABLE t1 (
      >   c1 INT PRIMARY KEY,
      >   c2 VARCHAR(255), 
      >   FULLTEXT INDEX (c2)
      > ) ENGINE = groonga DEFAULT CHARSET utf8;
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


検索スコアの取得方法
----------------------------

全文検索を行う際、指定したキーワードにより内容が一致するレコードを上位に表示したいというような場合があります。そうしたケースでは検索スコアを利用します。

検索スコアを取得するためには、テーブル定義時に ``_score`` という名前のカラムを作成して下さい。 ::

 mysql> CREATE TABLE t1 (
      >   c1 INT PRIMARY KEY,
      >   c2 TEXT,
      >   _score FLOAT,
      >   FULLTEXT INDEX (c2)
      > ) ENGINE = groonga DEFAULT CHARSET utf8;
 Query OK, 0 rows affected (0.22 sec)

_scoreカラムのデータ型はFLOATまたはDOUBLEである必要があります。

INSERTでテーブルにレコードを追加してみましょう。_scoreカラムは仮想カラムとして実装されており、更新は行えません。更新対象から外すか、値に ``null`` を使用する必要があります。 ::

 mysql> insert into t1 values(1, "aa ii uu ee oo", null);
 Query OK, 1 row affected (0.00 sec)
 
 mysql> insert into t1 values(2, "aa ii ii ii oo", null);
 Query OK, 1 row affected (0.00 sec)
 
 mysql> insert into t1 values(3, "dummy", null);
 Query OK, 1 row affected (0.00 sec)

全文検索(MATCH...AGAINSTによる検索)を実行した場合、_scoreカラムを通じて検索スコアを取得できます。_scoreカラムをORDER BYに指定することで結果のソートも行うことができます。 ::

 mysql> select * from t1 where match(c2) against("ii") order by _score desc;
 +----+----------------+--------+
 | c1 | c2             | _score |
 +----+----------------+--------+
 |  2 | aa ii ii ii oo |      3 |
 |  1 | aa ii uu ee oo |      1 |
 +----+----------------+--------+
 2 rows in set (0.00 sec)

レコードIDの取得方法
----------------------------

groongaではテーブルにレコードを追加した際にレコードを一意に識別するための番号が割当てられます。

groongaストレージエンジンではアプリケーションの開発を容易にするため、このレコードIDをSQLで取得できるようになっています。

レコードIDを取得するためには、テーブル定義時に ``_id`` という名前のカラムを作成して下さい。 ::

 mysql> CREATE TABLE t1 (
     ->   _id INT,
     ->   foo INT,
     ->   UNIQUE KEY (_id) USING HASH
     -> ) ENGINE = groonga;
 Query OK, 0 rows affected (0.01 sec)

_idカラムのデータ型は整数型(TINYINT、SMALLINT、MEDIUMINT、INT、BIGINT)である必要があります。

また_idカラムにはインデックスを作成することが可能ですが、HASH形式である必要があります。

INSERTでテーブルにレコードを追加してみましょう。_idカラムは仮想カラムとして実装されており、また_idの値であるレコードIDはgroongaにより割当てられるため、SQLによる更新時に値を指定することはできません。
更新対象から外すか、値に ``null`` を使用する必要があります。 ::

 mysql> INSERT INTO t1 VALUES (null, 100);
 Query OK, 1 row affected (0.00 sec)
 
 mysql> INSERT INTO t1 VALUES (null, 100);
 Query OK, 1 row affected (0.00 sec) 
 
 mysql> INSERT INTO t1 VALUES (null, 100);
 Query OK, 1 row affected (0.00 sec)
 
 mysql> INSERT INTO t1 VALUES (null, 100);
 Query OK, 1 row affected (0.00 sec)

レコードIDを取得するには、_idカラムを含むようにしてSELECTを行います。 ::

 mysql> select * from t1;
 +------+------+
 | _id  | foo  |
 +------+------+
 |    1 |  100 |
 |    2 |  100 |
 |    3 |  100 |
 |    4 |  100 |
 +------+------+
 4 rows in set (0.00 sec)

また直前のINSERTにより割当てられたレコードIDについては、last_insert_grn_id関数により取得することもできます。 ::

 mysql> INSERT INTO t1 VALUES (null, 100);
 Query OK, 1 row affected (0.00 sec)
 
 mysql> SELECT last_insert_grn_id();
 +----------------------+
 | last_insert_grn_id() |
 +----------------------+
 |                    5 |
 +----------------------+
 1 row in set (0.00 sec)

last_insert_grn_id関数はユーザ定義関数(UDF)としてgroongaストレージエンジンに含まれていますが、インストール時にCREATE FUNCTIONでMySQLに追加していない場合には、以下の関数定義DDLを実行しておく必要があります。 ::

 mysql> CREATE FUNCTION last_insert_grn_id RETURNS INTEGER SONAME 'ha_groonga.so';

ご覧のように_idカラムやlast_insert_grn_id関数を通じてレコードIDを取得することができました。ここで取得したレコードIDは後続のUPDATEなどのSQL文で利用すると便利です。 ::

 mysql> UPDATE t1 SET foo = 200 WHERE _id = 5;

ログ出力
----------------------------

groongaストレージエンジンではデフォルトでログの出力を行うようになっています。

ログファイルはMySQLのデータディレクトリ直下に ``groonga.log`` というファイル名で出力されます。

以下はログの出力例です。 ::

 2010-10-07 17:32:39.209379|n|b1858f80|groonga-storage-engine started.
 2010-10-07 17:32:44.934048|d|46953940|hash get not found (key=test)
 2010-10-07 17:32:44.936113|d|46953940|hash put (key=test)

現在のgroongaストレージエンジンは開発バージョンということもあり、ログのデフォルトの出力レベルは最大(=DUMP)となっております。

ログの出力レベルは ``groonga_log_level`` というシステム変数で確認することができます（グローバル変数）。またSET文で動的に出力レベルを変更することもできます。 ::

 mysql> SHOW VARIABLES LIKE 'groonga_log_level';
 +-------------------+-------+
 | Variable_name     | Value |
 +-------------------+-------+
 | groonga_log_level | DUMP  |
 +-------------------+-------+
 1 row in set (0.00 sec)
 
 mysql> SET GLOBAL groonga_log_level=NOTICE;
 Query OK, 0 rows affected (0.05 sec)
 
 mysql> SHOW VARIABLES LIKE 'groonga_log_level';
 +-------------------+--------+
 | Variable_name     | Value  |
 +-------------------+--------+
 | groonga_log_level | NOTICE |
 +-------------------+--------+
 1 row in set (0.00 sec)

設定可能なログレベルは以下の通りです。

* NONE
* EMERG
* ALERT
* CRIT
* ERROR
* WARNING
* NOTICE
* INFO
* DEBUG
* DUMP

またFLUSH LOGSでログの再オープンを行うことができます。MySQLサーバを停止せずにログのローテートを行いたいような場合には、以下の手順で実行すると良いでしょう。

1. ``groonga.log`` ファイルの名前を変更（OSコマンドのmvなどで）
2. MySQLサーバに対して"FLUSH LOGS"を実行（mysqlコマンドあるいはmysqladminコマンドにて）

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

全文検索時の ORDER BY LIMIT 高速化
-----------------------------------

一般的にMySQLでは"ORDER BY"はインデックス経由のレコード参照が行えればほぼノーコストで処理可能であり、"LIMIT"は検索結果が大量にヒットする場合でも処理対象を限定することでコストを一定に抑える効果があります。

しかし例えば全文検索のスコアの降順+LIMITのように"ORDER BY"の処理の際にインデックスが効かないクエリの場合、検索ヒット件数に比例したコストがかかってしまうため、特に大量の検索がヒットするようなキーワード検索においてクエリ処理に極端に時間がかかってしまうケースがあります。

Tritonnではこの問題に対して特に対応はできていませんでしたが、最新レポジトリではsen_records_sort関数を活用してSennaからの読み出しをスコアの降順に対応させることでSQLクエリからORDER BY句を取り除く(※スコア降順を指定していたケースに対してのみ有効)回避方法を導入しました。

groongaストレージエンジンでも行カウントを高速化するための仕組みを実装しています。

例えば以下のSELECT文では ORDER BY LIMIT は、groonga内で処理され、必要最小限のレコードだけをMySQLに返却しています。 ::

 SELECT * FROM t1 WHERE MATCH(c2) AGAINST("hoge") ORDER BY c1 LIMIT 1;

ORDER BY LIMIT 高速化の処理が行われたかどうかはステータス変数で確認することもできます。::

 mysql> show status like 'groonga_fast_order_limit';
 +--------------------------+-------+
 | Variable_name            | Value |
 +--------------------------+-------+
 | groonga_fast_order_limit | 1     |
 +--------------------------+-------+
 1 row in set (0.00 sec)

ORDER BY LIMIT 高速化の処理が行われる度に ``groonga_fast_order_limit`` ステータス変数がインクリメントされます。

備考：この高速化機能は、「select ... match against order by _score desc limit X, Y」を狙い撃ちした高速化で、現在のところ以下の条件が成立した場合に機能します。

* where句がmatch...againstのみ
* joinしていない
* limitの指定がある
* order byの指定がカラムである(_score、_id含む)
