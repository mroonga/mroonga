.. highlightlang:: none

Recent news
===========

.. _release-1-0-1:

1.0.1 released - 2011/10/29
---------------------------

The important changes in this release are the enhancement of geolocation search and the improvement of dynamic index modification in storage mode.

Improvements
++++++++++++

* [storage mode][wrapper mode] support reopening a database by `flush tables`.
* [wrapper mode] support geolocation index. (Only Point type can be stored in a column. Search using index is only available for MBRContains).
* [benchmark] add `groonga_dry_write` variable to specify not to write to groonga database, that is useful to check bottle necks in benchmarks.
* mention MySQL version in the installation guide for CentOS 6. [proposed by @yoshi_ken]
* [geolocation] improve performance by skip needless processes.
* add  `--disable-fast-mutexes` configure option to ignore fast mutexes even if mysql_config says it is enabled.
* [storage mode] support `create index`.
* [storage mode] support `drop index`.
* [storage mode] support multi columns index for full text search.
* support `D` pragma.
* support MySQL 5.5.17.
* support MySQL 5.6.3-m6.
* support groonga 1.2.7. (1.2.6 or below are no longer supported).
* support Ubuntu 11.10 Oneiric Ocelot.

Bug fixes
+++++++++

* fix a bug that we have no results if we specify '+' at the beginning of the query in boolean mode. [reported by Hajime Nishiyama]
* [Fedora] fix package dependencies. [reported by Takahiro Nagai]
* [Fedora] fix a problem that we get undefined symbol error when the plugin is loaded. [reported by Takahiro Nagai]
* [storage mode] fix a bug that index will not be correctly created if `varchar` is used in a multi-column index. #1143 [reported by Takahiro Nagai]

Acknowledgements
++++++++++++++++

* @yoshi_ken
* Hajime Nishiyama
* Takahiro Nagai

.. _release-1-0-0:

1.0.0 リリース - 2011/09/29
---------------------------

初回リリースから約1年経って、初のメジャーリリース！

改良
++++

* [ラッパーモード] drop index対応。 #1040
* [ストレージモード] GEOMETRY対応。（ただし、カラムに保存できる型はPointのみ対応。インデックスを利用した位置検索はMBRContainsのみ対応。） #1041
* [ストレージモード] マルチカラムインデックスに対応。 #455
* [ストレージモード][ラッパーモード] 全文検索用パーサー（トークナイザー）のカスタマイズに対応。 #592
* configureにデフォルトの全文検索用パーサーを指定する `--with-default-parser` オプションを追加。
* 実行時にデフォルトの全文検索用パーサーを指定する `groonga_default_parser` 変数を追加。
* [ラッパーモード] ストレージモードで実装している `order` と `limit` が指定された場合に必要のないレコードを返さないようにする高速化に対応。
* [ストレージモード] 1つの `select` 中での複数の `match against` 指定に対応。
* [非互換][ストレージモード] `_score` カラムの削除。代わりにMySQL標準の書き方である `match against` を使ってください。
* [ラッパーモード] プライマリキーの更新に対応。
* MySQL 5.5.16に対応。
* CentOS 6に対応。
* groonga 1.2.6に対応。（1.2.5以下のサポートを削除。）

修正
++++

* [Ubuntu] Lucid上でインストールエラーが発生する問題を修正。 （Isao Sugimotoさんが報告）
* auto_incrementを使った場合にテキストデータが壊れる問題を修正。 （@zaubermaerchenさんが報告） #1072
* [Ubuntu] Lucid上でテーブルを削除するとクラッシュする問題を修正。 #1063 （Isao Sugimotoさんが報告）
* MySQLと同じビルドオプションを使っていなかった問題を修正。 GitHub#4 (groongaのGitHubのIssues) （Tomohiro MITSUMUNEさんが報告）

感謝
++++

* Isao Sugimotoさん
* @zaubermaerchenさん
* Tomohiro MITSUMUNEさん

.. _release-0-9:

0.9 リリース - 2011/08/29
-------------------------

改良
++++

* MySQL 5.1.58に対応。
* MySQL 5.6.3に対応。

.. _release-0-8:

0.8 リリース - 2011/07/29
-------------------------

改良
++++

* [deb] プラグインインストール時のエラーを無視するようにした。
* [ラッパーモード] マルチカラムインデックスのサポート。 #1031
* [ラッパーモード] 大量レコードの全文検索に対応。 #1032
* [ラッパーモード] MyISAM対応。 #1033

.. _release-0-7:

0.7 リリース - 2011/06/29
-------------------------

改良
++++

* 既存のストレージエンジンに全文検索機能を追加する :doc:`userguide/wrapper` の追加。
* MySQL 5.5.13サポートの追加。 #984
* 安定してきたので、groongaのデフォルトログレベルをDUMPからNOTICEに変更。
* Mac OS Xでのビルドをサポート。（@issmさんが報告）

修正
++++

* 常にデバッグモードでビルドされる問題を修正。（@supistarさんが報告）

感謝
++++

* @issmさん
* @supistarさん

.. _release-0-6:

0.6 リリース - 2011/05/29
-------------------------

改良
++++

* auto_increment機能の追加。#670
* 不必要な"duplicated _id on insert"というエラーメッセージを
  抑制。 #910（←は未修正）
* CentOSで利用しているMySQLのバージョンを5.5.10から5.5.12へ
  アップデート。
* Ubuntu 11.04 Natty Narwhalサポートの追加。
* Ubuntu 10.10 Maverick Meerkatサポートの削除。
* Fedora 15サポートの追加。
* Fedora 14サポートの削除。

修正
++++

* ORDER BY LIMITの高速化が機能しないケースがある問題の修正。#845
* デバッグビルド時のメモリリークを修正。
* 提供しているCentOS用パッケージをOracle提供MySQLパッケージ
  と一緒に使うとクラッシュする問題を修正。

感謝
++++

* Mitsuhiro Shibuyaさん
* Hiroki Minetaさん
* @kodakaさん

0.5 リリース - 2011/03/29
-------------------------

改良
++++

* "uninstall plugin"対応 #741
* MariaDB対応 （かずひこさんが提案）
* 不要なデバッグシンボルを削除
* MySQL 5.5への対応強化。
* エラーメッセージの改良

感謝
++++

* かずひこさん

0.4 リリース - 2010/11/29
-------------------------

改良
++++

* 全文検索のスコア取得機能の追加。
* レコードIDへのアクセス機能の追加。
* 直近のレコードIDを参照するためのUDFの追加。
* インデックスによる範囲検索機能の追加。
* 全文検索におけるORDER BY LIMITパタンの高速化。
* ``groonga_fast_order_limit`` ステータス変数の追加。
* ログ出力機能の設定強化。
* ``groonga_log_level`` システム変数の追加。
* 全文検索機能の強化(NOT MATCH AGAINST対応)。
* MySQL 5.5への対応。

感謝
++++

* とみたまさひろさん

0.3 リリース - 2010/10/29
-------------------------

改良
++++

* エラーメッセージの出力を実装。
* カラムの刈り込みの実装を強化。
* 行カウント高速化機能の実装。
* ``groonga_count_skip`` ステータス変数の追加。
* ユーザガイドドキュメントの追加。

変更
++++

* インデックス作成時にNORMALIZEフラグを付与。

修正
++++

* LIMITなどを用いた場合にカーソルが正しくクローズされない問題の修正。

0.2 リリース - 2010/09/29
-------------------------

改良
++++

* packages.groonga.orgでのバイナリパッケージ配布開始。aptitude/yumによるインストールが可能に。
* バイナリログの出力に対応。

変更
++++

* 共有ライブラリの名前を"libgroonga_storage_engine.so"から"ha_groonga.so"に変更。
* configureオプションの ``--with-mysql`` および ``--libdir`` を削除。
* configureオプションの ``--with-mysql-source`` および ``--with-mysql-config`` を追加。

修正
++++

* ヘッダファイルのincludeパスを修正。
* "SHOW CREATE TABLE"に出力されるENGINE名を修正。

感謝
++++

* とみたまさひろさん


0.1 リリース - 2010/08/19
-------------------------

初回テストリリース
