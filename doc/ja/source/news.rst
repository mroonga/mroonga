.. highlightlang:: none

最新ニュース
============

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
