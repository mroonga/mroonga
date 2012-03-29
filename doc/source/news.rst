.. highlightlang:: none

News
====

.. _release-2-01:

Release 2.01 - 2012/03/29
-------------------------

Improvements
^^^^^^^^^^^^

* Supported MySQL 5.5.22.
* Supported MySQL 5.1.62.
* Required groonga 2.0.1 or later.
* [rpm] Improved plugin uninstall on upgrade.
* [rpm] Improved plugin uninstall on upgrade.
* [wrapper mode] Supported ``INSERT ON DUPLICATE KEY ERROR``
  with MyISAM. [#1300] [Reported by @104yuki_n]
* [wrapper mode] Used wrapped table's ``table_flags()``
  correctly. [#1304]
* Added ``--with-valgrind`` configure option for MySQL that
  enables Valgrind support.
* [mariadb] Supported ``DATETIME`` type with fraction
  seconds.
* Supported building without geometry support.
  [#1313] [Reported by Kazuhiko]
* [storage mode] Supported multiple column index with
  optimization build flags on i386 environment.
  [Reported by Kazuhiko]
* [wrapper mode] Confirmed InnoDB tests are passed with
  wrapper mode.
* [solaris] Supported build on Solaris 11. [Reported by Kazuhiko]
* [mariadb55] Supported ``mroonga_default_parser`` with
  MariaDB 5.5. [#1314] [Reported by Kazuhiko]
* [mariadb55] Supported ``ORDER LIMIT`` optimization with
  MariaDB 5.5. [#1315] [Reported by Kazuhiko]
* [doc] Added about MeCab.
* [storage mode] Supported index search for 0 value.
  [Reported by @104yuki_n]
* [storage mode] Supported Ubuntu Hardy with optimize
  build flags. [Reported by Kazuhiko]
* Added `logos <http://mroonga.github.com/logo/>`_ .
* Updated HTML design.

Fixes
^^^^^

* [storage mode] Fixed a memory leak.
* [storage mode] Fixed a bug that search by multi column
  index may not return some found records.
  [#1300] [Reported by @ytaka5]
* [storage mode] Fixed a bug that ``COUNT (*)`` isn't effective.
  [groonga-dev,00717] [Reported by Takayuki Honda]
* Fixed a memory leak on ``DROP DATABASE``.
* [storage mode] Fixed a bug that ``last_insert_grn_id()``
  may return broken value on 32bit environment.
  [Reported by Kazuhiko]
* [storage mode] Fixed a bug that ``COUNT (*)`` may
  return 0. [groonga-dev,00736] [Reported by Takayuki Honda]

Thanks
^^^^^^

* @ytaka5
* @104yuki_n
* Takayuki Honda
* Kazuhiko

.. _release-2-00:

Release 2.00 - 2012/02/29
-------------------------

.. caution::

   This release breaks backward compatibility. We need to
   dump and restore our database for upgrading.

In this release, mroonga has two changes that requires
database recreation:

1. Supported all characters for database, table and column
   names.
2. Groonga's native time data type is used for ``YEAR`` type
   in MySQL.

Here are upgrade sequence.

We dump a database that uses mroonga::

  % mysqldump MY_MROONGA_DATABASE > database-mroonga.dump

We drop the existing database::

  % mysql -u root -e 'DROP DATABASE MY_MROONGA_DATABASE'

We upgrade to "mroonga" storage engine. We will use
``apt-get`` on Debian GNU/Linux or Ubuntu and ``yum`` on
CentOS or Fedora.

apt-get::

  % sudo apt-get install -y mysql-server-mroonga

yum::

  % sudo yum remove -y mysql-mroonga
  % sudo yum install -y mysql-mroonga

.. caution::

   We don't forget to run ``yum remove`` before ``yum
   install``. If we forget to run ``yum remove``, we will break
   a MySQL's system table.

We recreate a database::

  % mysql -u root -e 'CREATE DATABASE MY_MROONGA_DATABASE'

We restore a database by modified dump file::

  % mysql -u root MY_MROONGA_DATABASE < database-mroonga.dump

Now, we can use mroonga 2.00.

Improvements
^^^^^^^^^^^^

* Supported MySQL 5.5.21.
* Required groonga 2.0.0 or later.
* Supported TIMESTAMP with fractional seconds on MySQL 5.6 and MariaDB.
* [storage mode] Supported ``ORDER LIMIT`` optimization on no primary key.
* [storage mode] Supported ``ORDER LIMIT`` optimization with
  fulltext search and ``COLUMN = INT_VALUE``.
* [storage mode] Supported fulltext search in sub query.
  [Reported by @camyuy]
* [incompatible] Mapped ``YEAR`` in MySQL to ``Time`` in
  groonga to improve groonga integration.
* [storage mode] Removed a needless write lock on update.
  [#1271] [Reported by Takahiro Nagai]
* Added ``mroonga_enable_optimization`` system variable to
  on/off optimization. It's useful for benchmark.
* [wrapper mode] Supported temporary table. [#1267]
* [incompatible] Supported ``/`` in database name. [#1281]
* Suppressed needless messages on ``INSERT ... ON DUPLICATE
  KEY UPDATE``.
* Supported ``INSERT ... ON DUPLICATE KEY UPDATE`` with
  ``UNIQUE KEY``. [#1283] [Reported by @104yuki_n]
* Supported ``DATETIME``, ``DATE``, ``TIMESTAMP``, ``TIME``,
  ``DECIMAL``, ``YEAR`` for primary key.
* [incompatible] Supported all characters for database, table and
  column names. [#1284]
* [wrapper mode] Supported 255 bytes over index. [#1282]
* [wrapper mode] Supported updating primary key. [#1195]
* [wrapper mode] Supported error handling on ``ALTER TABLE``. [#1195]
* [wrapper mode] Improved error message on unsupported
  geometry type like ``LineString``. [#1195]
* [wrapper mode] Supported
  ``INFORMATION_SCHEMA.REFERENTIAL_CONSTRAINTS``. [#1195]

Fixes
^^^^^

* [rpm] Changed to ensure re-install plugin.
* [doc] Fixed wrong storage engine name. [Reported by Tomoatsu Shimada]

Thanks
^^^^^^

* @camyuy
* Takahiro Nagai
* Tomoatsu Shimada
* @104yuki_n

.. _release-1-20:

Release 1.20 - 2012/01/29
-------------------------

.. caution::

   This release breaks backward compatibility. We need to
   dump and restore our database for upgrading.

In this release, mroonga has two changes that requires
database recreation:

1. Storage engine name is changed to "mroonga" from "groonga".
2. Groonga's native time data type is used for DATE, DATETIME
   and TIMESTAMP type in MySQL.

We need to modify dumped database to change "ENGINE=groonga"
in "CREATE TABLE" SQL. Here are upgrade sequence.

We dump a database that uses mroonga::

  % mysqldump MY_MROONGA_DATABASE > database-groonga.dump

We convert storage engine in dump file::

  % sed -e 's/^) ENGINE=groonga/) ENGINE=mroonga/' database-groonga.dump > database-mroonga.dump

We confirm that ``ENGINE=groonga`` only in ``CREATE TABLE``
is replaced with ``ENGINE=mroonga``. We need to check ``@@
... @@`` line includes ``CREATE TABLE``. If the line
includes ``CREATE TABLE``, the hunk will be a change for
``CREATE TABLE``::

  % diff -up database-groonga.dump database-mroonga.dump
  --- database-groonga.dump	2012-01-29 16:53:20.732624670 +0900
  +++ database-mroonga.dump	2012-01-29 16:54:47.608420981 +0900
  @@ -29,7 +29,7 @@ CREATE TABLE `diaries` (
     PRIMARY KEY (`id`),
     FULLTEXT KEY `title_index` (`title`),
     FULLTEXT KEY `body_index` (`body`)
  -) ENGINE=groonga DEFAULT CHARSET=utf8;
  +) ENGINE=mroonga DEFAULT CHARSET=utf8;
   /*!40101 SET character_set_client = @saved_cs_client */;

   --

We drop the existing database::

  % mysql -u root -e 'DROP DATABASE MY_MROONGA_DATABASE'

We upgrade to "mroonga" storage engine. We will use
``apt-get`` on Debian GNU/Linux or Ubuntu and ``yum`` on
CentOS or Fedora.

apt-get::

  % sudo apt-get install -y mysql-server-mroonga

yum::

  % sudo yum remove -y mysql-mroonga
  % sudo yum install -y mysql-mroonga

.. caution::

   We don't forget to run ``yum remove`` before ``yum
   install``. If we forget to run ``yum remove``, we will break
   a MySQL's system table.

We recreate a database::

  % mysql -u root -e 'CREATE DATABASE MY_MROONGA_DATABASE'

We restore a database by modified dump file::

  % mysql -u root MY_MROONGA_DATABASE < database-mroonga.dump

Now, we can use mroonga 1.20.

Improvements
^^^^^^^^^^^^

* Supported MySQL 5.5.20.
* Supported MySQL 5.1.61.
* Required groonga 1.3.0 or later.
* [incompatible] Changed storage engine name to "mroonga" from "groonga".
* Supported UTF8_BIN collate. [#1150]
* Disabled strict-aliasing warnings. [Reported by @issm]
* Supported decimal. [#1249] [Reported by @Kiske]
* [storage mode] Supported HA_KEYREAD_ONLY. It will improve
  column value access in index. [#1212]
* [storage mode] Supported float value in multiple column index.
* [storage mode] Supported double value in multiple column index.
* [storage mode] Mapped enum and set types to more suitable
  groonga data types.
* [wrapper mode] Supported REPAIR TABLE.
* [storage mode] Supported ``count(*)`` on view. [#1255]
  [Reported by Takahiro Nagai]
* [incompatible] Groonga's native time value is used for
  DATE, DATETIME and TIMESTAMP type values.

Fixes
^^^^^

* [debian] Fixed wrong mysql-server-groonga version in
  replaces and breaks. [Reported by @ytaka5]
* [doc] Fixed wrong execution result. [Reported by Hidekazu Tanaka]
* [wrapper mode] Fixed a memory leak. [Reported by montywi]

Thanks
^^^^^^

* @ytaka5
* Hidekazu Tanaka
* @issm
* montywi
* @Kiske

.. _release-1-11:

Release 1.11 - 2011/12/29
-------------------------

Improvements
^^^^^^^^^^^^

* Supported MySQL 5.5.19.
* Supported MySQL 5.6.4.
* Dropped MySQL 5.6.3 support.
* Supported Fedora 16.
* Dropped Fedora 15 support.
* Suppress strict-aliasing warnings. [groonga-dev,00659]
  [Reported by Kazuhiko Shiozaki]
* Supported utf8, binary, ascii, latin1, cp932, sjis,
  eucjpms, ujis and koi8r charset. [#1160] [Reported by nobody]
* [wrapper mode] Improved rollback handling on
  delete. [#1224] [Reported by Koichi Shishikura]

Fixes
^^^^^

* [storage mode] Fixed a bug that full text search on a
  table without primary key returns empty results. [#1193]
  [Reported by Kazuhiko Shiozaki]
* Fixed -Wno- compiler flag detection. [Patch by Arnaud Fontaine]
* [wrapper mode] Fixed a problem that index isn't
  used for full count. [#1196][groonga-dev,0653] [Reported by Kaneoka]

Thanks
^^^^^^

* Kazuhiko Shiozaki
* Arnaud Fontaine
* Kaneoka
* nobody
* Koichi Shishikura

.. _release-1-10:

Release 1.10 - 2011/11/29
-------------------------

Since this release, this project and product is called
"mroonga" instead of "groonga storage engine".

This release introduces MariaDB support and fixes several bugs.

Improvements
^^^^^^^^^^^^

* rename to "mroonga" from "groonga storage engine". #1170
* add groonga_libgroonga_version and groonga_version variables. #1158
* information_schema.plugins.plugin_version returns mroonga's version. #1157
* add groonga_log_file variable. #1178 [Suggested by nobody]
* FLUSH STATUS flushes groonga_* status variables. #1166 [Reported by Kazuhiko]
* support TRUNCATE TABLE #1151. [Suggested by Takahiro Nagai]
* support ALTER TABLE #1168.
* support MariaDB 5.2.9. #1152 [Reported by Kazuhiko]
* support MariaDB 5.3.2-beta. #1152 [Reported by Kazuhiko]
* [rpm] split document package.
* improve memory allocation for string system variables.
* use PLUGIN_LICENSE_GPL. [Suggested by Kazuhiko]
* remove needless MeCab related configurations.
* support FOUND_ROWS() and SQL_CALC_FOUND_ROWS. #1163 [Reported by Horikoshi Yuki]
* support table name that contains '-'. #1165 [Reported by nobody]
* support inplace index change on MySQL 5.1.
* [deb] support i386.
* [rpm] support i386.

Fixes
^^^^^

* [storage mode] fix a bug that REPLACE INTO with TEXT column does not work. #1153 [Reported by Kazuhiko]
* [wrapper mode] fix a bug that INSERT inside LOCK TABLE does not work with InnoDB. #1154 [Reported by Kazuhiko]
* fix a bug that using ORDER and LIMIT returns a wrong result. #1161 [Reported by Horikoshi Yuki]
* fix a crash bug when FORCE INDEX with unknown key is used. #1181 [Reported by Takahiro Nagai]

Thanks
^^^^^^

* Kazuhiko
* Horikoshi Yuki
* nobody
* Takahiro Nagai

.. _release-1-0-1:

Release 1.0.1 - 2011/10/29
--------------------------

The important changes in this release are the enhancement of geolocation search and the improvement of dynamic index modification in storage mode.

Improvements
^^^^^^^^^^^^

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

Fixes
^^^^^

* fix a bug that we have no results if we specify '+' at the beginning of the query in boolean mode. [reported by Hajime Nishiyama]
* [Fedora] fix package dependencies. [reported by Takahiro Nagai]
* [Fedora] fix a problem that we get undefined symbol error when the plugin is loaded. [reported by Takahiro Nagai]
* [storage mode] fix a bug that index will not be correctly created if `varchar` is used in a multi-column index. #1143 [reported by Takahiro Nagai]

Thanks
^^^^^^

* @yoshi_ken
* Hajime Nishiyama
* Takahiro Nagai

.. _release-1-0-0:

1.0.0 リリース - 2011/09/29
---------------------------

初回リリースから約1年経って、初のメジャーリリース！

改良
^^^^

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
^^^^

* [Ubuntu] Lucid上でインストールエラーが発生する問題を修正。 （Isao Sugimotoさんが報告）
* auto_incrementを使った場合にテキストデータが壊れる問題を修正。 （@zaubermaerchenさんが報告） #1072
* [Ubuntu] Lucid上でテーブルを削除するとクラッシュする問題を修正。 #1063 （Isao Sugimotoさんが報告）
* MySQLと同じビルドオプションを使っていなかった問題を修正。 GitHub#4 (groongaのGitHubのIssues) （Tomohiro MITSUMUNEさんが報告）

感謝
^^^^

* Isao Sugimotoさん
* @zaubermaerchenさん
* Tomohiro MITSUMUNEさん

Past releases
-------------

.. toctree::
   :maxdepth: 2

   news/0.x
