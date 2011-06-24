.. highlightlang:: none

リファレンス
============

利用可能なSQLコマンドの一覧
---------------------------

groongaストレージエンジンのソースコードの以下のディレクトリにSQL実行サンプルファイルがたくさんあります。 ::

 test/sql/t

ファイル一覧 ::

  auto_increment.test        insert.test
  binlog.test                insert_wrapper.test
  btree.test                 last_insert_grn_id.test
  count_performance.test     log_level.test
  create_table.test          order_limit_performance.test
  create_table_wrapper.test  replace.test
  delete.test                select_all.test
  delete_wrapper.test        select_pkey.test
  drop_database.test         select_secondary_key.test
  drop_table.test            show_create_table.test
  flush_logs.test            show_table_status.test
  fulltext.test              tinyint.test
  fulltext_wrapper.test      update.test
  hash.test                  update_wrapper.test
  information_schema.test

ここに記述されているSQL文が現在利用可能なSQL文となります。

※逆に言うとここに記述されていないSQL文の動作はサポートしておりません。

サーバ変数の一覧
----------------

今のところ独自に追加されたサーバ変数はありません。

ステータス変数の一覧
--------------------

独自に追加されたステータス変数を説明します。

groonga_count_skip
^^^^^^^^^^^^^^^^^^

行カウント高速化機能が動作する度にカウントアップされます。スキーマ/SQLチューニングを行って行カウント高速化機能を利用できるようにした際の動作確認に利用できます。

groonga_fast_order_limit
^^^^^^^^^^^^^^^^^^^^^^^^

ORDER BY LIMIT 高速化機能が動作する度にカウントアップされます。スキーマ/SQLチューニングを行って ORDER BY LIMIT 高速化機能を利用できるようにした際の動作確認に利用できます。
