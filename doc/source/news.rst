.. highlightlang:: none

News
====

.. _release-3-05:

Release 3.05 - 2013/6/29
-------------------------

Improvements
^^^^^^^^^^^^

* Added warnings for truncated date data. [#1768] [Suggested by Y.Kentaro]
* Supported MySQL 5.6.12. [Reported by WING]
* Added documentation about :doc:`/reference/troubleshooting`.
* Supported to enable fast mutexes option by build configuration.
  [#1760] [Reported by WING]

Fixes
^^^^^

* Fixed a bug that three or more sections in W pragma doesn't work.
  [#1770] [Reported by shizuin]
* Fixed build error with "-O2". [Reported by Y.Kentaro]
* Fixed a memory leak by re-registration of ``normalizers/mysql``.
* Fixed a crush bug when updating with ``DISABLE KEYS``. [#1759]
* [doc] Fixed a wrong translation about status variable of optimization
  in wrapper mode. [Reported by YOSHIDA Mitsuo]
* Fixed a crush bug when no where clause with ``ORDER BY ... LIMIT``
  is specified. [Reported by @memorycraft]
* Fixed a bug that data is hidden when ``LOCK TABLES`` and ``ENABLE KEYS``
  are used same time. [#1778] [Reported by Y.Kentaro]

Thanks
^^^^^^

* Y.Kentaro
* WING
* shizuin
* YOSHIDA Mitsuo
* @memorycraft

.. _release-3-04:

Release 3.04 - 2013/5/29
-------------------------

Improvements
^^^^^^^^^^^^

* Improved MariaDB 10.0.2 support. [#1729]
* [doc] Updated supported SQL command list. [Reported by Y.Kentaro]
* Dropped Ubuntu 11.10 (Oneiric Ocelot) support.
* Supported mroonga bundled MariaDB package. [#1691]
* [wrapper] Stopped to parse column comment. [Reported by Y.Kentaro]
* Stopped to validate normal column comment. [Reported by Y.Kentaro]
* Improved the way to detect directory which contains libmysqlservices.a.
  [Reported by Y.Kentaro]
* Improved to accept free style normal comment in table/index comment.
  [Suggested by Y.Kentaro]
* Supported "W" pragma. This feature is derived from
  `Tritonn <http://qwik.jp/senna/query.html>`_.
* Supported ``mroonga_command()`` without the current database.
  [Reported by Y.Kentaro]
* Improved to use ``auto_increment`` value for creating table. [#1741]
* Improved to keep the value of ``auto_increment`` even though latest record is
  deleted. [#1738]
* [doc] Added documentation how to install mroonga on Windows.
* Added install SQL for initial setup. [groonga-dev,01434]
  [Suggested by Kazuhiko]
* Supported Debian 8.0 (jessie)

Fixes
^^^^^

* Fixed a bug that empty query search causes SEGV.
  [groonga-dev,01346] [Reported by Nakai Kanako]
* Fixed a package build error depend on directory existence.
  [groonga-dev,01335] [Reported by WING] [Patch by yoku ts]
* Fixed a missing build dependency to ``groonga-normalizer-mysql`` package.
  [Patch by Y.Kentaro]
* Fixed a bug that the value of ``Mroonga_log_level`` can't be set in my.cnf.
  [groonga-dev,01379] [Reported by Kazuhiro Isobe]
* Fixed a memory leak that default tokenizer is not correctly freed.
* [wrapper] Fixed a bug that comment is changed for alter table without
  engine name causes a missing table. [Reported by Y.Kentaro]

Thanks
^^^^^^

* Y.Kentaro
* Nakai Kanako
* WING
* yoku ts
* Kazuhiro Isobe
* Kazuhiko

.. _release-3-03:

Release 3.03 - 2013/4/29
-------------------------

Improvements
^^^^^^^^^^^^

* [doc] Added documentation about table limitations. [groonga-dev,01281]
  [Reported by Kazuhiro Isobe]
* [doc] Added ``mroonga_command`` documentation.
* Supported ``default_tokenizer`` as table parameter in comment.
* Supported using existing table as lexicon not only "FULLTEXT INDEX",
  but also normal "INDEX".
  This change improves compatibility to groonga.
* Supported MySQL 5.6.11.
* Supported collation in multiple column index. [groonga-dev,01302]
  [Reported by Kouhei Tanabe]
* Supported no parser fulltext index for predictive search by "XXX*"
  in groonga query syntax.
* [cmake] Dropped MySQL 5.5.x build support.
* Supported custom normalizer for FULLTEXT INDEX.
  Use can specify custom normalizer as a comment.
  Supported syntax is ``FULLTEXT INDEX (column) COMMENT 'normalizer "NormalizerXXX"'``.
  [groonga-dev:01303] [Suggested by Kouhei Tanabe]
* Supported Ubuntu 13.04 Raring Ringtail.

Fixes
^^^^^

* [storage] Fixed a bug that stored value can't be searched because of
  unexpected cast for integer. [#1696] [groonga-dev,01266]
  [Reported by smztks]
* [wrapper] Fixed a bug that multiple match against returns
  invalid aggregated count. [#1702] [groonga-dev,01279]
  [Reported by Masahiro Furuta]
* Fixed a bug that ``mrn_log_level`` is ignored. [groonga-dev,01290]
  [Reported by Kazuhiro Isobe]
* Fixed a bug that mroonga crashes when freeing internal temporary
  shared object.
* [doc] Fixed a typo about running mode of storage engine and
  a long ambiguous sentence. [Reported by Ichiro Suzuki]
* [mysql55] Fixed a bug that inplace anonymous index recreation cause a crash.
  [groonga-dev,01319] [Reported by Kouhei Tanabe]

Thanks
^^^^^^

* smztks
* Masahiro Furuta
* Kazuhiro Isobe
* Kouhei Tanabe
* Ichiro Suzuki

.. _release-3-02:


Release 3.02 - 2013/3/29
-------------------------

Improvements
^^^^^^^^^^^^

* Improved bundling to MariaDB 10.0 for Linux. [#1644]
* Added the value of list documentation about ``mroonga_log_level``
  in reference manual. [groonga-dev,01255] [Reported by Kazuhiro Isobe]
* [experimental] Added ``mroonga_command`` UDF. [#1643]
  This UDF supports to send query directly to ``groonga``.

Fixes
^^^^^

* Fixed a bug that less than conditional expression on WHERE clause doesn't work
  for multiple nullable column index. [groonga-dev,01253] [#1670]
  [Reported by Horikoshi Yuki]
* [wrapper] Fixed the invalid timing to free ``key`` object too early.
  This bug may occurs when recreating indexes by DISABLE KEYS/ENABLE KEYS. [#1671]
  [Reported by keigo ito]

Thanks
^^^^^^

* Kazuhiro Isobe
* Horikoshi Yuki
* keigo ito

.. _release-3-01:


Release 3.01 - 2013/2/28
-------------------------

Improvements
^^^^^^^^^^^^

* Supported ``utf8_unicode_ci`` and ``utf8mb4_unicode_ci``.
  Note that ``groonga-normalizer-mysql`` >= 1.0.1 is required.
* [experimental] Supported foreign key for storage mode. [#1612]

Fixes
^^^^^

* Fixed groonga derived bug that the records in specific range returns 0 records
  unexpectedly. [groonga-dev,01192] [Reported by b senboku]
* Fixed to disable DTrace by default for Mac OS X.

Thanks
^^^^^^

* b senboku


.. _release-3-00:

Release 3.00 - 2013/2/9
-------------------------

Improvements
^^^^^^^^^^^^

* Supported Fedora 18.
* Dropped Fedora 17 support.
* Supported ``utf8_general_ci`` and ``utf8mb4_generic_ci``
  compatible normalizer.
* [rpm][centos] Supported MySQL 5.6.10 on CentOS 5.

Fixes
^^^^^

* [storage] Fixed a groonga derived bug that unique key doesn't
  match mixed case keyword. [groonga-dev,01187] [Reported by Kouhei Tanabe]

Thanks
^^^^^^

* Kouhei Tanabe

.. _release-2-10:

Release 2.10 - 2012/12/29
-------------------------

Improvements
^^^^^^^^^^^^

* Supported to use ENABLE KEYS/DISABLE KEYS against table whose charset is
  different from system charset. [Reported by @kokoronavi]
* Improved to show the value of duplicated key on error message.
* Supported MySQL 5.6.9-rc. [#1547] [#1548] [Reported by wing]
* [storage] Supported to use ``GROUP BY`` with index.
  This change improves search speed against ``GROUP BY``.
* Dropped Ubuntu 11.04 (Natty Narwhal) support.

Fixes
^^^^^

* [rpm][centos] Fixed to use MySQL 5.1.66-2 on CentOS 6. [Reported by @wakisuke.ua]
* Fixed a bug that ``MATCH AGAINST`` with ``INNER JOIN`` syntax causes an error.
  [Reported by ooshiro]

Thanks
^^^^^^

* @kokoronavi
* @wakisuke.ua
* wing
* ooshiro

.. _release-2-09:

Release 2.09 - 2012/11/29
-------------------------

.. caution::

   This release has backward incompatible changes against ``TIMESTAMP``
   value and primary indexed char(N).

   ``TIMESTAMP`` value is changed to store as UTC timezone.
   If you have any table that uses ``TIMESTAMP`` column with no UTC timezone,
   please recreate (dump and restore) database.

   If you have any table that uses char(N) as primary key,
   please recreate index.

   Here is a procedure how to recreate database or recreate index.

Dump a database that uses mroonga::

  % mysqldump MY_MROONGA_DATABASE > database-mroonga.dump

Restore a database by modified dump file::

  % mysql -u root MY_MROONGA_DATABASE < database-mroonga.dump

Recreate a index::

   mysql> ALTER TABLE table_name DROP PRIMARY KEY;
   mysql> ALTER TABLE table_name ADD PRIMARY KEY(column_name);

Improvements
^^^^^^^^^^^^

* [rpm][centos] Supported MySQL 5.5.28 on CentOS 5.
* [rpm][centos] Supported MySQL 5.1.61 on CentOS 6.
* [wrapper mode] Supported last_insert_id() [#1540] [Reported by @soundkitchen]

Fixes
^^^^^

* [mysql51] Fixed crash bug by checking existence of utf8mb4.
  MySQL 5.1 doesn't have utf8mb4. [groonga-dev,01069] [Reported by wakisuke]
* [storage mode] Fixed impossible deleting problem that matched records.
  [#1533] [Reported by @HANZUBON]
* Fixed a bug that primary indexed char(N) can't be searched.
  This bug affects if any value of char(N) has M-length (M < N) string or
  it has one more spaces at the last and require index recreation.
* Fixed a bug that content after NULL character is ignored for char(N)
* Fixed to store ``TIMESTAMP`` value as UTC timezone.
  This is backward incompatible change.

Thanks
^^^^^^

* @soundkitchen
* wakisuke
* @HANZUBON

.. _release-2-08:

Release 2.08 - 2012/10/29
-------------------------

.. caution::

   This release has a backward incompatible change against multiple
   column index. If you have any tables that uses
   any multiple comlumn indexes against VARCHAR or CHAR,
   please recreate those indexes by ``ALTER TABLE DISABLE KEYS``
   and ``ALTER TABLE ENBALE KEYS``::

     mysql> ALTER TABLE table_name DISABLE KEYS;
     mysql> ALTER TABLE table_name ENABLE KEYS;

Improvements
^^^^^^^^^^^^

* [storage mode] Supported ``INFORMATION_SCHEMA.TABLES.DATA_LENGTH``.
  [Suggested by @9m]
* Supported utf8mb4. [groonga-dev,01049] [Reported by warp kawada]
* Supported Ubuntu 12.10 (Quantal Quetzal)
* [rpm][fedora] Supported MySQL 5.5.28.

Fixes
^^^^^

* [storage mode] Fixed unique judge problem of multiple column unique index
  with date column. [groonga-dev,01041] [#1495] [Reported by jd fonc]
* Fixed a bug that ``WHERE index < XXX ORDER BY DESC`` doesn't sort by descending.
  [Reported by @taro252]
* [rpm] Fixed missing ``DROP FUNCTION mroonga_snippet``.
  [Reported by @tokuhy]
* Fixed range search by multi column index by int.
* [doc] Fixed wrong command line operation during build process.
  [groonga-dev,01061] [Reported by Kazuhiro Isobe]

Thanks
^^^^^^

* @9m
* warp kawada
* jd fonc
* @taro252
* @tokuhy
* Kazuhiro Isobe

.. _release-2-07:

Release 2.07 - 2012/09/29
-------------------------

Improvements
^^^^^^^^^^^^

* [deb] Enabled AppArmor configuration for MeCab. [Reported by @Qurage]
* [storage mode][experimental] Added mroonga_snippet() function. [#1442]

Fixes
^^^^^

* [rpm] Fixed ``groonga_required_version``. [groonga-dev,01033] [Reported by wing]
* Fixed datetime out of range problems. [groonga-dev,01035] [#1476]
  [Reported by Nakatani Munetaka]

Thanks
^^^^^^

* @Qurage
* wing
* Nakatani Munetaka

.. _release-2-06:

Release 2.06 - 2012/08/29
-------------------------

.. caution::

   This release has a backward compatibility about a database.
   But this release depends on new functionality introduced at groonga 2.0.6.
   Note that you must use mroonga 2.06 with groonga 2.0.6.

Improvements
^^^^^^^^^^^^

* Supported "-WORD" syntax in BOOLEAN MODE.

Fixes
^^^^^

* Fixed nallowing records by "order by" clause with fulltext condition.
  [groonga-dev,00977] [#1422] [Reported by Nakatani Munetaka]
* Removed needless build flags. [#1453] [Reported by @nabebeb]
* [rpm][centos] Fixed missing mysql-devel BuildRequires. 
  [groonga-dev,01009] [Reported by wing]

Thanks
^^^^^^

* @nabebeb
* wing
* Nakatani Munetaka

.. _release-2-05:

Release 2.05 - 2012/07/29
-------------------------

.. caution::

   This release has a backward incompatible change against SET column
   and ENUM.
   If you use SET column or ENUM that has the number of elements < 256
   in :doc:`/userguide/storage`, please recreate (dump and restore)
   database.

Here is upgrade sequence.

Dump a database that uses mroonga::

  % mysqldump MY_MROONGA_DATABASE > database-mroonga.dump

Drop the existing database::

  % mysql -u root -e 'DROP DATABASE MY_MROONGA_DATABASE'

You must upgrade to "mroonga" storage engine. Use
``apt-get`` on Debian GNU/Linux or Ubuntu and ``yum`` on
CentOS or Fedora.

apt-get::

  % sudo apt-get install -y mysql-server-mroonga

yum (upgrade from mroonga release prior to v2.02)::

  % sudo yum remove -y mysql-mroonga
  % sudo yum install -y mysql-mroonga

yum (upgrade from mroonga release v2.03 or later)::

  % sudo yum install -y mysql-mroonga

.. caution::

   Don't forget to run ``yum remove`` before ``yum
   install`` if you upgrade mroonga prior to v2.02 release.
   If you forget to run ``yum remove``, we will break
   a MySQL's system table.

Recreate a database::

  % mysql -u root -e 'CREATE DATABASE MY_MROONGA_DATABASE'

Restore a database by modified dump file::

  % mysql -u root MY_MROONGA_DATABASE < database-mroonga.dump

Now, we can use mroonga 2.05.

Improvements
^^^^^^^^^^^^

* [storage mode] Supported index for SET column.
* [rpm] Supported MySQL 5.5.25a.
* Supported Fedora 17.
* Dropped Fedora 16 support.
* [storage mode] Supported TINYINT UNSIGNED/SMALLINT UNSIGNED/
  MEDIUMINT UNSIGNED/INT UNSIGNED/BIGINT UNSIGNED type.
* [storage mode] Reduced storage size for ENUM.

.. _release-2-04:

Release 2.04 - 2012/06/29
-------------------------

Improvements
^^^^^^^^^^^^

* [wrapper mode] Supported disable/enable keys in bulk insert. [#1311]

Fixes
^^^^^

* Fixed to disable query cache with transaction. [#1384]
* Disabled partition explicitly. [#1391]
* [rpm][deb] Fixed to ensure deleting mroonga plugin from ``mysql.plugin`` table
  before install. [groonga-dev,00948] [Suggested by Kazuhiro Isobe]
* Fixed a crash bug by setting nonexistent path to ``mroonga_log_file``
  variable. [#1404] [Reported by @nabebeb]
* [experimental] Supported mroonga related data path change.
  [groonga-dev,00914] [#1392] [Suggested by Kazuhiro Isobe]

Thanks
^^^^^^

* Kazuhiro Isobe
* @nabebeb

.. _release-2-03:

Release 2.03 - 2012/05/29
-------------------------

.. caution::

   This release has a backward incompatible change against multiple
   column index. If you have any tables that uses
   :doc:`/userguide/storage` and any multiple comlumn indexes, please
   recreate those indexes by ``ALTER TABLE DISABLE KEYS`` and ``ALTER
   TABLE ENBALE KEYS``::

     mysql> ALTER TABLE table_name DISABLE KEYS;
     mysql> ALTER TABLE table_name ENABLE KEYS;

Improvements
^^^^^^^^^^^^

* [storage mode] Supported disable/enable keys in bulk insert.
  [#1310]
* [rpm][centos] Supported MySQL 5.5.24.
* [rpm][fedora] Supported MySQL 5.5.23.
* Added :ref:`mroonga_match_escalation_threshold` system variable.
* Required groonga 2.0.3 or later.
* [yum] Changed RPM package name that provides yum repository from
  groonga-repository to groonga-release to follow RPM package name
  convension such as centos-release and fedora-release.
* [mac os x] Supported installing by Homebrew.
  See :doc:`/install` about details.

Fixes
^^^^^

* Fixed build failure on Mac OS X.
* [wrapper mode] Fixed a bug that searching in transaction reports not
  found error.
  [#1322][groonga-dev,00746] [Reported by Takken Ishibashi]
* [rpm] Fixed a bug that mroonga plugin is unregistered when upgrading.
  [groonga-dev,00810]
  [Reported by Takken Ishibashi] [Patch by Iwai, Masaharu]
* Fixed a bug that row based replication transfers wrong data on MySQL
  5.5 or earlier.
  [#1379][groonga-dev,00902] [Reported by Kenji Doi]

Thanks
^^^^^^

* Takken Ishibashi
* Iwai, Masaharu
* Kenji Doi

.. _release-2-02:

Release 2.02 - 2012/04/29
-------------------------

.. caution::

   The package sign key is changed since this release. Import the new
   package sign key before updating groogna packages.

   Debian/Ubuntu::

     % sudo apt-get update
     % sudo apt-get -y --allow-unauthenticated install groonga-keyring

   CentOS/Fedora::

     % sudo yum update
     % sudo yum install --nogpgcheck -y groonga-repository

.. caution::

   This release breaks a backward compatibility for ``ENUM`` type. If
   you have ``ENUM`` type and it has 256 or more values, you need to
   dump your table before upgrade and restore your table after
   upgrade.

.. caution::

   This release breaks a backward compatibility for ``SET`` type. If
   you have ``SET`` type and it has 32 or more values, you need to
   dump your table before upgrade and restore your table after
   upgrade.

Improvements
^^^^^^^^^^^^

* Added workaround for build for GCC 4.7 and MySQL 5.5.22.
  The combination makes build error with ``-Wdeprecated`` gcc
  option. [#1337] [Reported by Kazuhiko Shiozaki]
* [mariadb] Supported MariaDB 5.5.23.
  [#1339] [Reported by Kazuhiko Shiozaki] [Reported by Ryo Onodera]
* [storage mode] Stopped needless primary key truncated warning on
  ``REPLACE``.
* [storage mode] Supported search ``ENUM`` values by index. [#1336]
  [Suggested by @104yuki_n]
* [incompatible] Changed groonga type for ``ENUM`` to unsigned 2bytes
  integer type from signed 2bytes integer type. ``ENUM`` column
  recreation is needed.
* [incompatible] Changed groonga type for ``SET`` to unsigned 8bytes
  integer type from signed 8bytes integer type. ``SET`` column
  recreation is needed.
* Supported MySQL 5.5.23.
* Required groonga 2.0.2 or later.
* [incompatible][experimental] Changed to use similar search instead
  of phrase search for ``MATCH AGAINST IN NATURAL LANGUAGE MODE``.
* [apt][yum] Changed package sign key.
* Supported Ubuntu 12.04 Precise Pangolin.

Fixes
^^^^^

* [storage mode] Fixed wrong key length is used.
* Fixed a crash bug that is caused when searching with updaging very large
  inverted index. [#1321]
  [groonga-dev,00743] [Reported by Takashi Kawano]
  [groonga-dev,00746] [Reported by Takken Ishibashi]
* Fixed a bug that truncated table has invalid tokenizer and encoding.
  [#1335] [Reported by @boss_sato]
* Fixed a bug that the first insert is failed with Spider and mroonga
  combination. [#1319] [groonga-dev,00736] [Reported by Takayuki Honda]

Thanks
^^^^^^

* Takashi Kawano
* @boss_sato
* Kazuhiko Shiozaki
* Ryo Onodera
* @104yuki_n
* Takayuki Honda
* Takken Ishibashi

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
