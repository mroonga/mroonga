.. highlightlang:: none

ストレージモード
================

ここでは groonga ストレージエンジンにおけるストレージモードの利用方法を説明します。

全文検索の利用方法
------------------

インストールが確認できたら、テーブルを1つ作成してみましょう。 ``ENGINE = groonga`` とgroongaストレージエンジンを指定するところがポイントです。::

  mysql> CREATE TABLE diaries (
      ->   id INT PRIMARY KEY AUTO_INCREMENT,
      ->   content VARCHAR(255),
      ->   FULLTEXT INDEX (content)
      -> ) ENGINE = groonga DEFAULT CHARSET utf8;
  Query OK, 0 rows affected (0.10 sec)

INSERTでデータを投入してみましょう。 ::

  mysql> INSERT INTO diaries (content) VALUES ("明日の天気は晴れでしょう。");
  Query OK, 1 row affected (0.01 sec)

  mysql> INSERT INTO diaries (content) VALUES ("明日の天気は雨でしょう。");
  Query OK, 1 row affected (0.00 sec)

全文検索を実行してみます。 ::

  mysql> SELECT * FROM diaries WHERE MATCH(content) AGAINST("晴れ");
  +----+-----------------------------------------+
  | id | content                                 |
  +----+-----------------------------------------+
  |  1 | 明日の天気は晴れでしょう。 |
  +----+-----------------------------------------+
  1 row in set (0.00 sec)

おぉぉー。検索できましたね。

検索スコアの取得方法
--------------------

.. note::

   1.0.0以前のgroongaストレージエンジンではMySQLの標準的な検索スコアの取得方法ではなく、 ``_score`` という専用のカラムを作成するという独自の方法でした。1.0.0からはMySQLの標準的な取得方法になっています。

全文検索を行う際、指定したキーワードにより内容が一致するレコードを上位に表示したいというような場合があります。そうしたケースでは検索スコアを利用します。

検索スコアはMySQLの標準的な方法 [#score]_ で取得できます。つまり、SELECTの取得するカラム名を指定するところやORDER BYのところにMATCH...AGAINSTを指定します。

それでは実際にやってみましょう。::

  mysql> INSERT INTO diaries (content) VALUES ("今日は晴れました。明日も晴れるでしょう。");
  Query OK, 1 row affected (0.00 sec)

  mysql> INSERT INTO diaries (content) VALUES ("今日は晴れましたが、明日は雨でしょう。");
  Query OK, 1 row affected (0.00 sec)

  mysql> SELECT *, MATCH (content) AGAINST ("晴れ") FROM diaries WHERE MATCH (content) AGAINST ("晴れ") ORDER BY MATCH (content) AGAINST ("晴れ") DESC;
  +----+--------------------------------------------------------------+------------------------------------+
  | id | content                                                      | MATCH (content) AGAINST ("晴れ") |
  +----+--------------------------------------------------------------+------------------------------------+
  |  3 | 今日は晴れました。明日も晴れるでしょう。 |                                  2 |
  |  1 | 明日の天気は晴れでしょう。                      |                                  1 |
  |  4 | 今日は晴れましたが、明日は雨でしょう。    |                                  1 |
  +----+--------------------------------------------------------------+------------------------------------+
  3 rows in set (0.00 sec)

検索対象の文字列 ``晴れ`` をより多く含む、すなわち検索スコアの高い ``id = 3`` のメッセージが上に来ていることが確認できます。また、SELECT句にMATCH AGAINSTを記述しているため、検索スコアも取得できています。

属性名を変更したい場合は ``AS`` を使って下さい。 ::

  mysql> SELECT *, MATCH (content) AGAINST ("晴れ") AS score FROM diaries WHERE MATCH (content) AGAINST ("晴れ") ORDER BY MATCH (content) AGAINST ("晴れ") DESC;
  +----+--------------------------------------------------------------+------------------------------------+
  | id | content                                                      | MATCH (content) AGAINST ("晴れ") |
  +----+--------------------------------------------------------------+------------------------------------+
  |  3 | 今日は晴れました。明日も晴れるでしょう。 |                                  2 |
  |  1 | 明日の天気は晴れでしょう。                      |                                  1 |
  |  4 | 今日は晴れましたが、明日は雨でしょう。    |                                  1 |
  +----+--------------------------------------------------------------+------------------------------------+
  3 rows in set (0.00 sec)


全文検索用パーサの変更
----------------------

MySQLは全文検索用のパーサ [#parser]_ を指定する以下のような構文を持っています。::

  FULLTEXT INDEX (content) WITH PARSER パーサ名

しかし、この構文を利用する場合は、あらかじめすべてのパーサをMySQLに登録しておく必要があります。一方、groongaはトークナイザー（MySQLでいうパーサ）を動的に追加することができます。そのため、groognaストレージエンジンでもこの構文を採用するとgroonga側に動的に追加されたトークナイザーに対応できなくなります。groongaに動的に追加されるトークナイザーにはMeCabを用いたトークナイザーもあり、この制限に縛られることは利便性を損なうと判断し、以下のようなコメントを用いた独自の構文を採用することにしました。::

  FULLTEXT INDEX (content) COMMENT 'parser "TokenMecab"'

.. note::

   ``FULLTEXT INDEX`` に ``COMMENT`` を指定できるのはMySQL 5.5からになります。MySQL 5.1を利用している場合は後述の ``groonga_default_parser`` 変数を利用してください。

パーサに指定できるのは以下の値です。

TokenBigram
  バイグラムでトークナイズする。ただし、連続したアルファベット・連続した数字・連続した記号はそれぞれ1つのトークンとして扱う。そのため、3文字以上のトークンも存在する。これはノイズを減らすためである。

  デフォルト値。

TokenMecab
  MeCabを用いてトークナイズする。groongaがMeCabサポート付きでビルドされている必要がある。

TokenBigramSplitSymbol
  バイグラムでトークナイズする。TokenBigramと異なり、記号が連続していても特別扱いして1つのトークンとして扱わず通常のバイグラムの処理を行う。

  TokenBigramではなくTokenBigramSplitSymbolを利用すると「Is it really!?!?!?」の「!?!?!?」の部分に「!?」でマッチする。TokenBigramの場合は「!?!?!?」でないとマッチしない。

TokenBigramSplitSymbolAlpha
  バイグラムでトークナイズする。TokenBigramSplitSymbolに加えて、連続したアルファベットも特別扱いせずに通常のバイグラムの処理を行う。

  TokenBigramではなくTokenBigramSplitSymbolAlphaを利用すると「Is it really?」に「real」でマッチする。TokenBigramの場合は「really」でないとマッチしない。

TokenBigramSplitSymbolAlphaDigit
  バイグラムでトークナイズする。TokenBigramSplitSymbolAlphaに加えて、連続した数字も特別扱いせずに通常のバイグラムの処理を行う。つまり、すべての字種を特別扱いせずにバイグラムの処理を行う。

  TokenBigramではなくTokenBigramSplitSymbolAlphaDigitを利用すると「090-0123-4567」に「567」でマッチする。TokenBigramの場合は「4567」でないとマッチしない。

TokenBigramIgnoreBlank
  バイグラムでトークナイズする。TokenBigramと異なり、空白を無視して処理する。

  TokenBigramではなくTokenBigramIgnoreBlankを利用すると「み な さ ん 注 目」に「みなさん」でマッチする。TokenBigramの場合は「み な さ ん」でないとマッチしない。

TokenBigramIgnoreBlankSplitSymbol
  バイグラムでトークナイズする。TokenBigramSymbolと異なり、空白を無視して処理する。

  TokenBigramSplitSymbolではなくTokenBigramIgnoreBlankSplitSymbolを利用すると「! !? ??」に「???」でマッチする。TokenBigramSplitBlankの場合は「? ??」でないとマッチしない。

TokenBigramIgnoreBlankSplitSymbolAlpha
  バイグラムでトークナイズする。TokenBigramSymbolAlphaと異なり、空白を無視して処理する。

  TokenBigramSplitSymbolAlphaではなくTokenBigramIgnoreBlankSplitSymbolAlphaを利用すると「I am a pen.」に「ama」でマッチする。TokenBigramSplitBlankAlphaの場合は「am a」でないとマッチしない。

TokenBigramIgnoreBlankSplitSymbolAlphaDigit
  バイグラムでトークナイズする。TokenBigramSymbolAlphaDigitと異なり、空白を無視して処理する。

  TokenBigramSplitSymbolAlphaDigitではなくTokenBigramIgnoreBlankSplitSymbolAlphaDigitを利用すると「090 0123 4567」に「9001」でマッチする。TokenBigramSplitBlankAlphaDigitの場合は「90 01」でないとマッチしない。

TokenDelimit
  空白区切りでトークナイズする。

  「映画 ホラー 話題」は「映画」・「ホラー」・「話題」にトークナイズされる。

TokenDelimitNull
  null文字（\\0）区切りでトークナイズする。

  「映画\\0ホラー\\0話題」は「映画」・「ホラー」・「話題」にトークナイズされる。

TokenUnigram
  ユニグラムでトークナイズする。ただし、連続したアルファベット・連続した数字・連続した記号はそれぞれ1つのトークンとして扱う。そのため、2文字以上のトークンも存在する。これはノイズを減らすためである。

TokenTrigram
  トリグラムでトークナイズする。ただし、連続したアルファベット・連続した数字・連続した記号はそれぞれ1つのトークンとして扱う。そのため、4文字以上のトークンも存在する。これはノイズを減らすためである。

デフォルトのパーサは ``configure`` の ``--with-default-parser`` オプションでビルド時に指定することができます。::

  ./configure --with-default-parser TokenMecab ...

また、my.cnfまたはSQL内で ``groonga_default_parser`` 変数を指定することでも指定できます。my.cnfで指定するとMySQLを再起動しても値は変更されたままですが、反映させるために再起動しなければいけません。一方、SQLで指定した場合はすぐに設定が反映されますが、MySQLが再起動すると設定は失われます。

my.cnf::

  [mysqld]
  groonga_default_parser=TokenMecab

SQL::

  mysql> SET GLOBAL groonga_default_parser = TokenMecab;
  Query OK, 0 rows affected (0.00 sec)

位置情報検索の利用方法
----------------------

ストレージモードでは全文検索だけではなく位置情報検索も高速に実行できます。ただし、MyISAMとは異なりデータとして格納できるのはPOINT型のみです。LINEなどの他のデータ型は保存できません。また、インデックスを用いた高速な検索に対応しているのはMBRContainsだけです。MBRDisjointなどには対応していません。

位置情報検索を利用する場合のテーブル定義はMyISAMと同様にPOINT型のカラムを定義し、そのカラムに対してSPATIAL INDEXを指定します。::

  mysql> CREATE TABLE shops (
      ->   id INT PRIMARY KEY AUTO_INCREMENT,
      ->   name VARCHAR(255),
      ->   location POINT NOT NULL,
      ->   SPATIAL INDEX (location)
      -> ) ENGINE = groonga;
  Query OK, 0 rows affected (0.06 sec)

データの登録方法もMyISAMのときと同様にGeomFromText()関数を使って文字列からPOINT型の値を作成します。::

  mysql> INSERT INTO shops VALUES (null, '根津のたいやき', GeomFromText('POINT(139.762573 35.720253)'));
  Query OK, 1 row affected (0.00 sec)

  mysql> INSERT INTO shops VALUES (null, '浪花家', GeomFromText('POINT(139.796234 35.730061)'));
  Query OK, 1 row affected (0.00 sec)

  mysql> INSERT INTO shops VALUES (null, '柳屋 たい焼き', GeomFromText('POINT(139.783981 35.685341)'));
  Query OK, 1 row affected (0.00 sec)

池袋駅（139.7101 35.7292）が左上の点、東京駅（139.7662 35.6815）が右下の点となるような長方形内にあるお店を探す場合は以下のようなSELECTになります。::

  mysql> SELECT id, name, AsText(location) FROM shops WHERE MBRContains(GeomFromText('LineString(139.7101 35.7292, 139.7662 35.6815)'), location);
  +----+-----------------------+------------------------------------------+
  | id | name                  | AsText(location)                         |
  +----+-----------------------+------------------------------------------+
  |  1 | 根津のたいやき | POINT(139.762572777778 35.7202527777778) |
  +----+-----------------------+------------------------------------------+
  1 row in set (0.00 sec)

位置情報で検索できていますね！

レコードIDの取得方法
--------------------

groongaではテーブルにレコードを追加した際にレコードを一意に識別するための番号が割当てられます。

groongaストレージエンジンではアプリケーションの開発を容易にするため、このレコードIDをSQLで取得できるようになっています。

レコードIDを取得するためには、テーブル定義時に ``_id`` という名前のカラムを作成して下さい。 ::

  mysql> CREATE TABLE memos (
      ->   _id INT,
       >   content VARCHAR(255),
      ->   UNIQUE KEY (_id) USING HASH
      -> ) ENGINE = groonga;
  Query OK, 0 rows affected (0.04 sec)

_idカラムのデータ型は整数型(TINYINT、SMALLINT、MEDIUMINT、INT、BIGINT)である必要があります。

また_idカラムにはインデックスを作成することが可能ですが、HASH形式である必要があります。

INSERTでテーブルにレコードを追加してみましょう。_idカラムは仮想カラムとして実装されており、また_idの値であるレコードIDはgroongaにより割当てられるため、SQLによる更新時に値を指定することはできません。
更新対象から外すか、値に ``null`` を使用する必要があります。 ::

  mysql> INSERT INTO memos VALUES (null, "今夜はさんま。");
  Query OK, 1 row affected (0.00 sec)

  mysql> INSERT INTO memos VALUES (null, "明日はgroongaをアップデート。");
  Query OK, 1 row affected (0.00 sec)

  mysql> INSERT INTO memos VALUES (null, "帰りにおだんご。");
  Query OK, 1 row affected (0.00 sec)

  mysql> INSERT INTO memos VALUES (null, "金曜日は肉の日。");
  Query OK, 1 row affected (0.00 sec)

レコードIDを取得するには、_idカラムを含むようにしてSELECTを行います。 ::

  mysql> SELECT * FROM memos;
  +------+------------------------------------------+
  | _id  | content                                  |
  +------+------------------------------------------+
  |    1 | 今夜はさんま。                    |
  |    2 | 明日はgroongaをアップデート。 |
  |    3 | 帰りにおだんご。                 |
  |    4 | 金曜日は肉の日。                 |
  +------+------------------------------------------+
  4 rows in set (0.00 sec)

また直前のINSERTにより割当てられたレコードIDについては、last_insert_grn_id関数により取得することもできます。 ::

  mysql> INSERT INTO memos VALUES (null, "冷蔵庫に牛乳が残り1本。");
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

  mysql> UPDATE memos SET content = "冷蔵庫に牛乳はまだたくさんある。" WHERE _id = last_insert_grn_id();
  Query OK, 1 row affected (0.00 sec)
  Rows matched: 1  Changed: 1  Warnings: 0

ログ出力
--------

groongaストレージエンジンではデフォルトでログの出力を行うようになっています。

ログファイルはMySQLのデータディレクトリ直下に ``groonga.log`` というファイル名で出力されます。

以下はログの出力例です。 ::

  2010-10-07 17:32:39.209379|n|b1858f80|groonga-storage-engine started.
  2010-10-07 17:32:44.934048|d|46953940|hash get not found (key=test)
  2010-10-07 17:32:44.936113|d|46953940|hash put (key=test)

ログのデフォルトの出力レベルはNOTICE（必要な情報のみ出力。デバッグ情報などは出力しない）となっております。

ログの出力レベルは ``groonga_log_level`` というシステム変数で確認することができます（グローバル変数）。またSET文で動的に出力レベルを変更することもできます。 ::

  mysql> SHOW VARIABLES LIKE 'groonga_log_level';
  +-------------------+--------+
  | Variable_name     | Value  |
  +-------------------+--------+
  | groonga_log_level | NOTICE |
  +-------------------+--------+
  1 row in set (0.00 sec)

  mysql> SET GLOBAL groonga_log_level=DUMP;
  Query OK, 0 rows affected (0.00 sec)

  mysql> SHOW VARIABLES LIKE 'groonga_log_level';
  +-------------------+-------+
  | Variable_name     | Value |
  +-------------------+-------+
  | groonga_log_level | DUMP  |
  +-------------------+-------+
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
----------------

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
  ) ENGINE = groonga DEFAULT CHARSET utf8;

この時、以下のようなSELECT文が発行される場合、groongaストレージエンジンではSELECT句およびWHERE句で参照しているカラムに対してのみデータの読み取りを行ってSQL文を処理します（内部的に不要なカラムに対してはアクセスしません）。 ::

  SELECT c1, c2, c11 FROM t1 WHERE c2 = XX AND c12 = "XXX";

このケースではc1,c2,c11,c12に対してのみアクセスが行われ、SQL文が高速に処理されることになります。

行カウント高速化
----------------

COUNT(\*)などの行カウントを行う場合と通常のSELECTによるデータ参照を行う場合に対して、従来よりMySQLではストレージエンジンの呼び出しを行う部分(=ストレージエンジンインタフェース)における区別が存在していないため、行数をカウントするだけで良いような場合にもレコードアクセス（SELECTの結果には含まれないデータへのアクセス）が行われる問題があります。

groongaストレージエンジンの前身であるTritonn(MySQL+Senna)ではこの問題に対して"2indパッチ"という不要なレコードアクセスを省略する仕組みを独自に実装してこの性能問題を回避していました。

これに引き続き、groongaストレージエンジンでも行カウントを高速化するための仕組みを実装しています。

例えば以下のSELECT文では不要なカラムデータの読み取りは省略され、必要最小限のコストで行カウントの結果を返すことができます。 ::

  SELECT COUNT(*) FROM t1 WHERE MATCH(c2) AGAINST("hoge");

行カウント高速化の処理が行われたかどうかはステータス変数で確認することもできます。::

  mysql> SHOW STATUS LIKE 'groonga_count_skip';
  +--------------------+-------+
  | Variable_name      | Value |
  +--------------------+-------+
  | groonga_count_skip | 1     |
  +--------------------+-------+
  1 row in set (0.00 sec)

行カウント高速化の処理が行われる度に ``groonga_count_skip`` ステータス変数がインクリメントされます。

備考：この高速化機能はインデックスを用いて実装されています。現在のところインデックスアクセスのみでレコードが特定できるパタンでのみ有効に機能します。

全文検索時の ORDER BY LIMIT 高速化
----------------------------------

一般的にMySQLでは"ORDER BY"はインデックス経由のレコード参照が行えればほぼノーコストで処理可能であり、"LIMIT"は検索結果が大量にヒットする場合でも処理対象を限定することでコストを一定に抑える効果があります。

しかし例えば全文検索のスコアの降順+LIMITのように"ORDER BY"の処理の際にインデックスが効かないクエリの場合、検索ヒット件数に比例したコストがかかってしまうため、特に大量の検索がヒットするようなキーワード検索においてクエリ処理に極端に時間がかかってしまうケースがあります。

Tritonnではこの問題に対して特に対応はできていませんでしたが、最新レポジトリではsen_records_sort関数を活用してSennaからの読み出しをスコアの降順に対応させることでSQLクエリからORDER BY句を取り除く(※スコア降順を指定していたケースに対してのみ有効)回避方法を導入しました。

groongaストレージエンジンでも ORDER BY LIMIT を高速化するための仕組みを実装しています。

例えば以下のSELECT文では ORDER BY LIMIT は、groonga内で処理され、必要最小限のレコードだけをMySQLに返却しています。 ::

  SELECT * FROM t1 WHERE MATCH(c2) AGAINST("hoge") ORDER BY c1 LIMIT 1;

ORDER BY LIMIT 高速化の処理が行われたかどうかはステータス変数で確認することもできます。::

  mysql> SHOW STATUS LIKE 'groonga_fast_order_limit';
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
* order byの指定がカラム(_id含む)またはwhere句に指定したmatch...againstである

.. rubric:: 脚注

.. [#score] `MySQL 5.1 リファレンスマニュアル :: 11 関数と演算子 :: 11.7 全文検索関数 <http://dev.mysql.com/doc/refman/5.1/ja/fulltext-search.html>`_
.. [#parser] groongaではトークナイザーと呼んでいる。
