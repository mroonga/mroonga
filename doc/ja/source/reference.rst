.. highlightlang:: none

リファレンス
===============================

利用可能なSQLコマンドの一覧
----------------------------

groongaストレージエンジンのソースコードの以下のディレクトリにSQL実行サンプルファイルがたくさんあります。 ::

 test/sql/t

ファイル一覧 ::

 binlog.test             create_table.test   drop_table.test  information_schema.test  select_pkey.test           show_table_status.test
 btree.test              delete.test         fulltext.test    insert.test              select_secondary_key.test  update.test
 count_performance.test  drop_database.test  hash.test        select_all.test          show_create_table.test

ここに記述されているSQL文が現在利用可能なSQL文となります。

※逆に言うとここに記述されていないSQL文の動作はサポートしておりません。

