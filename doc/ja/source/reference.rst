.. highlightlang:: none

.. _reference:

リファレンス
===============================

利用可能なSQLコマンドの一覧
----------------------------

groongaストレージエンジンのソースコードの以下のディレクトリにSQL実行サンプルファイルがたくさんあります。 ::

 test/sql/t

ファイル一覧 ::

 binlog.test             drop_table.test          select_pkey.test
 btree.test              fulltext.test            select_secondary_key.test
 count_performance.test  hash.test                show_create_table.test
 create_table.test       information_schema.test  show_table_status.test
 delete.test             insert.test              update.test
 drop_database.test      select_all.test

ここに記述されているSQL文が現在利用可能なSQL文となります。

※逆に言うとここに記述されていないSQL文の動作はサポートしておりません。

サーバ変数の一覧
----------------------------

今のところ独自に追加されたサーバ変数はありません。

ステータス変数の一覧
----------------------------

独自に追加されたステータス変数を説明します。

groonga_count_skip
++++++++++++++++++++++++++++

行カウント高速化機能が動作する度にカウントアップされます。スキーマ/SQLチューニングを行って行カウント高速化機能を利用できるようにした際の動作確認に利用できます。