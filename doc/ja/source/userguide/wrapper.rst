.. highlightlang:: none

ラッパーモード
==============

ここでは groonga ストレージエンジンにおけるラッパーモードの利用方法を説明します。

ラッパーモードの利用方法
------------------------

ラッパーモードでは、既存のストレージエンジンをラップするかたちで groonga ストレージエンジンが動作します。ラップする対象となるストレージエンジンは、現在のところ SQL のコメントを利用して ``COMMENT = 'engine "innodb"'`` のように指定するようになっています。

.. note::

   現在のところ、ラッパーモードではテーブルに必ずプライマリーキーを設定する必要があります。ストレージモードはこの限りではありません。

.. note::

   ラッパーモードでは現在ストレージモードでサポートされていない、以下をサポートしています。
   * マルチカラムインデックス
   * null値

全文検索の利用方法
------------------

それでは早速 groonga ストレージエンジンのラッパーモードを利用して、テーブルを1つ作成してみましょう。 ::

 mysql> CREATE TABLE weather_forecasts (
     ->   id INT PRIMARY KEY,
     ->   content VARCHAR(255),
     ->   FULLTEXT INDEX (content)
     -> ) ENGINE = groonga COMMENT = 'engine "innodb"' DEFAULT CHARSET utf8;
 Query OK, 0 rows affected (0.30 sec)

次に INSERT でデータを投入してみましょう。 ::

 mysql> INSERT INTO weather_forecasts VALUES(1, "明日の天気は晴れでしょう。");
 Query OK, 1 row affected (0.04 sec)
 
 mysql> INSERT INTO weather_forecasts VALUES(2, "明日の天気は雨でしょう。");
 Query OK, 1 row affected (0.04 sec)

データの投入が終了したら、全文検索を実行してみます。 ::

 mysql> SELECT * FROM weather_forecasts WHERE MATCH(content) AGAINST("晴れ");
 +----+-----------------------------------------+
 | id | content                                 |
 +----+-----------------------------------------+
 |  1 | 明日の天気は晴れでしょう。 |
 +----+-----------------------------------------+
 1 row in set (0.00 sec)

お、検索できましたね。

検索スコアの取得方法
--------------------

全文検索を行う際、指定したキーワードにより内容が一致するレコードを上位に表示したいというような場合があります。そうしたケースでは検索スコアを利用します。

ストレージモードでは、検索スコアの取得を行うために ``_score`` という名前の仮想カラムを作成していました。一方、ラッパーモードではこの必要がありません。その代わりに WHERE 句へ指定したものと同じ match against を ORDER BY や SELECT 部分に指定することで、そのマッチの検索スコアを利用することができるようになっています。これは MySQL の全文検索機能における標準的な検索スコアの取得方法にならったものです（ `MySQL 5.1 リファレンスマニュアル :: 11 関数と演算子 :: 11.7 全文検索関数`_ ）。

.. _`MySQL 5.1 リファレンスマニュアル :: 11 関数と演算子 :: 11.7 全文検索関数`: http://dev.mysql.com/doc/refman/5.1/ja/fulltext-search.html

それでは、実際に検索スコアを利用してみることにしましょう。まずはテーブルを作成します。 ::

 mysql> CREATE TABLE messages (
     ->   id INT PRIMARY KEY,
     ->   message TEXT,
     ->   FULLTEXT INDEX (message)
     -> ) ENGINE = groonga COMMENT = 'engine "innodb"' DEFAULT CHARSET utf8;
 Query OK, 0 rows affected (0.28 sec)

次に、検索対象となるデータを挿入します。 ::

 mysql>  INSERT INTO messages VALUES(1, "aa ii uu ee oo");
 Query OK, 1 row affected (0.04 sec)

 mysql>  INSERT INTO messages VALUES(2, "aa ii ii ii oo");
 Query OK, 1 row affected (0.04 sec)

 mysql>  INSERT INTO messages VALUES(3, "hoge huga hehe");
 Query OK, 1 row affected (0.05 sec)

 mysql>  INSERT INTO messages VALUES(4, "foo bar baz");
 Query OK, 1 row affected (0.04 sec)

データが挿入し終わったので、実際に検索スコアを利用した検索を行なってみます。 ::

 mysql> SELECT * FROM messages WHERE MATCH(message) AGAINST("ii") ORDER BY MATCH(message) AGAINST("ii") DESC;
 +----+----------------+
 | id | message        |
 +----+----------------+
 |  2 | aa ii ii ii oo |
 |  1 | aa ii uu ee oo |
 +----+----------------+
 2 row in set (0.00 sec)

検索対象の文字列 ``ii`` をより多く含む、すなわち検索スコアの高い ``id = 2`` のメッセージが上に来ていることが確認できます。

ここで、SELECT 句に match against を記述することで、検索スコアの値自体を検索結果に含めることも可能です。 ::

 mysql> SELECT *, MATCH(message) AGAINST("ii") FROM messages WHERE MATCH(message) AGAINST("ii") ORDER BY MATCH(message) AGAINST("ii") DESC;
 +----+----------------+------------------------------+
 | id | message        | MATCH(message) AGAINST("ii") |
 +----+----------------+------------------------------+
 |  2 | aa ii ii ii oo |                            3 |
 |  1 | aa ii uu ee oo |                            1 |
 +----+----------------+------------------------------+
 2 rows in set (0.00 sec)

属性名を変更したい場合は ``AS`` を使って下さい。 ::

 mysql> SELECT *, MATCH(message) AGAINST("ii") AS score FROM messages WHERE MATCH(message) AGAINST("ii") ORDER BY MATCH(message) AGAINST("ii") DESC;
 +----+----------------+-------+
 | id | message        | score |
 +----+----------------+-------+
 |  2 | aa ii ii ii oo |     3 |
 |  1 | aa ii uu ee oo |     1 |
 +----+----------------+-------+
 2 rows in set (0.00 sec)

レコードIDの取得方法
--------------------

ストレージモードでは ``_id`` という名前のカラムを作成することにより groonga 内部でのレコード ID 値を取得することが可能となっていました。

他方、ラッパーモードでは groonga 内部でのレコード ID 値を取得することができません。これは「レコードを一意に識別するためにはより MySQL の作法に従ったプライマリキーを利用すべきである」という設計方針によるものです。

ログ出力
--------

groongaストレージエンジンではデフォルトでログの出力を行うようになっています。

ログファイルはMySQLのデータディレクトリ（/var/lib/mysql/ など）直下に ``groonga.log`` というファイル名で出力されます。

以下はログの出力例です。 ::

 2011-06-24 11:11:31.282121|n|6bdea740|groonga-storage-engine started.
 2011-06-24 11:11:31.282154|n|6bdea740|log level is 'NOTICE'
 2011-06-24 11:30:58.485508|n|3cda6700|DDL:table_create x
 2011-06-24 11:31:05.131690|n|cee84700|DDL:obj_remove x
 2011-06-24 13:37:31.692572|n|86ceb700|DDL:column_create t1_0001 c2
 2011-06-24 13:37:31.781556|n|86ceb700|DDL:set_source t1_0001.c2 t1.c2
 2011-06-24 13:49:27.767387|n|5cd1f700|DDL:obj_remove t1_0001
 2011-06-24 14:33:55.867480| |8cd59700|96a20c50|:18446744072478952540 filter(2)

ログのデフォルトの出力レベルは NOTICE （必要な情報のみ出力。デバッグ情報などは出力しない）となっています。

ログの出力レベルは ``groonga_log_level`` というシステム変数で確認することができます（グローバル変数）。またSET文で動的に出力レベルを変更することもできます。 ::

 mysql> SHOW VARIABLES LIKE 'groonga_log_level';
 +-------------------+--------+
 | Variable_name     | Value  |
 +-------------------+--------+
 | groonga_log_level | NOTICE |
 +-------------------+--------+
 1 row in set (0.00 sec)
 
 mysql> SET GLOBAL groonga_log_level=DUMP;
 Query OK, 0 rows affected (0.05 sec)
 
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

注意点
------

0.7でのラッパーモードの実装はMySQLのストレージエンジンAPIを実直に利用したものになっています。そのため、「全文検索結果で結果レコードを絞り込むことにより不要なレコードアクセスを減らし、全文検索結果を用いた高速な検索を実現する」ということができません。これはMySQLのストレージエンジンAPIの制限なのですが、0.8では解消し、高速に検索できるようにする予定です。

0.7での動作と0.8で予定している動作の概要は以下の通りです。

0.7での動作
^^^^^^^^^^^

0.7ではMySQLのストレージエンジンAPIに素直に従った実装になってるため、全文検索結果を有効に活用しきれていません。MySQLのストレージエンジンAPIに従った場合の動作を以下に示します。

サンプルクエリは ``SELECT * FROM users WHERE users.age >= 20 AND MATCH(description) AGAINST("趣味");`` とします。

1. MySQLはクエリのうち全文検索条件の部分「 ``MATCH(description) AGAINST("趣味")`` 」だけで検索するようにgroongaストレージエンジンに依頼します。
2. groongaストレージエンジンはMySQLから渡された条件で全文検索を行います。
3. MySQLはクエリから全文検索条件の部分「 ``MATCH(description) AGAINST("趣味")`` 」を取り除いたクエリ「 ``SELECT * FROM users WHERE users.age >= 20`` 」で検索するようgroongaストレージエンジンに依頼します。
4. groongaストレージエンジンはMySQLから渡された条件で検索するようにラップしているストレージエンジンに依頼します。
5. MySQLはgroongaストレージエンジンにマッチしたレコードを順に返すように要求します。
6. groongaストレージエンジンはラップしているストレージエンジンからレコードを順に取り出してMySQLに返します。このとき返すレコードは「 ``users.age >= 20`` 」にマッチしたレコードです。「 ``MATCH(description) AGAINST ("趣味")`` 」での絞り込みは行われていません。
7. MySQLはgroongaストレージエンジンから返ってきたそれぞれのレコードについて、全文検索結果のスコアを問い合せます。
8. groongaストレージエンジンは各レコードについて「 ``MATCH(description) AGAINST ("趣味")`` 」がヒットしていればスコアを返し、そうでなければ「ヒットしなかったという特別なスコア」を返します。
9. MySQLはgroongaストレージエンジンが返したスコアを確認し、「ヒットしなかったという特別なスコア」が返されたレコードを検索結果から除去します。この時点ではじめて全文検索結果がレコードの絞り込みに使われます。
10. MySQLはgroongaストレージエンジンが返したレコード（= ラップしているストレージエンジンが返したレコード）から、「 ``MATCH(description) AGAINST ("趣味")`` 」にヒットしなかったレコードを削除したものをクライアントに返します。

このうち、6.のところで全文検索結果を反映したレコードのみを返すことができれば余計なレコードアクセスが減り、より高いパフォーマンスをだせます。しかし、MySQLのストレージエンジンAPIでは「全文検索以外の条件でレコードを取得した後に全文検索結果を参照してレコードをフィルターする」という動作のためせっかくのgroongaの高速な全文検索機能を活かしきれていません。

0.8での動作
^^^^^^^^^^^

0.8ではMySQLのストレージエンジンAPIの制限を回避し、全文検索結果を有効に活用した高速な検索を実現する方法を実装する予定です。予定している実装方法を以下に示します。

サンプルクエリは ``SELECT * FROM users WHERE users.age >= 20 AND MATCH(description) AGAINST("趣味");`` とします。

1. MySQLはクエリのうち全文検索条件の部分「 ``MATCH(description) AGAINST("趣味")`` 」だけで検索するようにgroongaストレージエンジンに依頼します。
2. groongaストレージエンジンはMySQLから渡された条件で全文検索を行います。
3. MySQLはクエリから全文検索条件の部分「 ``MATCH(description) AGAINST("趣味")`` 」を取り除いたクエリ「 ``SELECT * FROM users WHERE users.age >= 20`` 」で検索するようgroongaストレージエンジンに依頼します。
4. groongaストレージエンジンはMySQLから渡されたクエリに全文検索でヒットしたレコードで絞り込む条件「 ``id IN (1, 3, 4, ...)`` 」を追加したクエリ「 ``SELECT * FROM users WHERE users.age >= 20 AND id IN (1, 3, 4, ...)`` 」で検索するよう、ラップしているストレージエンジンに依頼します。
5. MySQLはgroongaストレージエンジンにレコードを順に返すように要求します。
6. groongaストレージエンジンはラップしているストレージエンジンからレコードを順に取り出してMySQLに返します。このとき返すレコードは「 ``users.age >= 20 AND id IN (1, 3, 4, ...)`` 」にマッチしたレコードなので、「 ``MATCH(description) AGAINST ("趣味")`` 」での絞り込み結果も反映されています。
7. MySQLはgroongaストレージエンジンから返ってきたそれぞれのレコードについて、全文検索結果のスコアを問い合せます。
8. groongaストレージエンジンは各レコードについて「 ``MATCH(description) AGAINST ("趣味")`` 」で検索した結果のスコアを返します。
9. MySQLはgroongaストレージエンジンが返したレコードをクライアントに返します。

ポイントは4.の「全文検索でヒットしたレコードで絞り込む条件」を追加している部分です。これで「全文検索条件での絞り込みで無駄なレコードアクセスを減らすことができない」というMySQLのストレージエンジンAPIの制限を回避することができます。これにより、全文検索結果を用いて無駄なレコードアクセスを減らすことができるため、高速な全文検索を実現できます。
