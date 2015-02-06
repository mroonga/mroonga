:orphan:

.. highlightlang:: none

News
====

.. _release-4-10:

Release 4.10 - 2015-01-29
-------------------------

This release has a bug fix for unique index. If you're using unique
index in storage mode, your data may be broken. If you're using unique
index in storage mode, we recommend to upgrade to this version and
re-create your unique indexes.

Here are SQL statements to re-create your indexes including unique
indexes::

    ALTER TABLE ${YOUR_TABLE} DISABLE KEYS;
    ALTER TABLE ${YOUR_TABLE} ENABLE KEYS;

You may get "duplicated record" error on ``ENABLE KEYS``. If you get
the error, please confirm your data carefully and fix the duplication.

Here are broken data scenario:

  * An unique index is created.
  * Insert data.
  * Insert duplicated data. It's reported as an error.
  * Insert duplicated data again. It can be inserted. It breaks unique
    consistency.

Improvements
^^^^^^^^^^^^

* [storage] Supported static index construction in FULLTEXT
  INDEX comment with ``'table "XXX"'``. Execute ``DISABLE KEYS``, then
  ``ENABLE KEYS`` for static index construction.
  [Reported by Naoya Murakami]
* [rpm][centos] Built with MySQL 5.6.22 on CentOS 7.
  [groonga-dev,03047] [Reported by Hiroshi Kagami]
* [rpm][centos] Built with MariaDB 5.5.40-2 on CentOS 7.
* [storage][mysql56] Supported to report duplicated error
  for ``ADD UNIQUE INDEX``. [Reported by kazeburo]

Fixes
^^^^^

* [storage] Fixed a bug that referenced table is accidentally
  removed on error. This means that index creation is failed for
  FULLTEXT INDEX comment with ``'table "terms"'``, "terms"
  table can be removed on error.
* [storage] Fixed a bug that duplicated entry is removed in unique index
  when duplicated error is occurred. This bug makes consistency of indexes broken,
  so recommended to recreate existing indexes again after upgrading Mroonga. [Reported by kazeburo]
* [storage] Fixed a bug that ``INSERT ON DUPLICATE KEY UPDATE`` is
  broken. This bug may change existing other records by executing update fallback when
  insert error occurred. [Reported by kazeburo]

Thanks
^^^^^^

* Naoya Murakami
* Hiroshi Kagami
* kazeburo

.. _release-4-09:

Release 4.09 - 2014-12-29
-------------------------

Improvements
^^^^^^^^^^^^

* [rpm] Supported MySQL 5.6.22 on CentOS 6.
  [Reported by @oreradio]
* Added :ref:`mroonga_boolean_mode_syntax_flags` that custom syntax in
  ``MATCH () AGAINST ('...' IN BOOLEAN MODE)``.
* Supported no normalizer ``FULLTEXT INDEX`` by specifying ``none`` as
  normalizer such as ``FULLTEXT INDEX (...) COMMENT 'normalizer
  "none"'``.
* Supported referencing primary key value of auto created records.

Thanks
^^^^^^

* @oreradio

.. _release-4-08:

Release 4.08 - 2014/11/29
-------------------------

Improvements
^^^^^^^^^^^^

* [deb] Supported Ubuntu 12.04 again for Travis-CI.
* [test] Added script which checks performance schema.
  [Patch by Elena Stepanova]
* [rpm] Supported MySQL 5.5.40 on CentOS 6 and 7.
  [groonga-dev,02955] [Reported Noboru Nishiyama]
* [doc] Added :doc:`/reference/limitations` about column size.
* [storage] Added missing primary key check when primary key
  is required to specify.
  [groonga-dev,02963] [Reported by kashihara]

Fixes
^^^^^

* [doc] Fixed nonexistent MySQL version. [groonga-dev,02899]
  [Reported by GMO Media, Inc.]
* [doc] Removed needless SCL related install description for CentOS 5.
* [doc] Use ``service`` command to start MySQL on CentOS 5 and 6.
* [doc] Use ``systemctl`` command to start MySQL on CentOS 7.

Thanks
^^^^^^

* GMO Media, Inc.
* Elena Stepanova
* Noboru Nishiyama
* kashihara

.. _release-4-07:

Release 4.07 - 2014/10/29
-------------------------

Improvements
^^^^^^^^^^^^

* [storage] Changed to treat deprecated ``INSERT DELAYED`` as error. [GitHub#20] [MDEV#6837] [Reported by Elena Stepanova]
* [storage][mariadb10] Added proper error message for duplicated entries on adding an unique index.
  [GitHub#19] [Reported by Elena Stepanova]
* [cmake] Removed needless status message for compiler flag checks [GitHub#22]
* Supported token filter in table/index comment for wrapper/storage mode.
  [GitHub#25] [Patch by Naoya Murakami]
* [mroonga_command] Improved to need not to know whether Groonga database already exists
  before executing mroonga_command. In the previous versions, you must prepare table which use Mroonga.
* [storage] Supported to specify normalizer in table comment.
  [GitHub#27] [Patch by Naoya Murakami]
* [storage] Supported column compression flag (``COMPRESS_LZ4``, ``COMPRESS_ZLIB``).
  [GitHub#32] [Patch by Naoya Murakami]
* Added :ref:`mroonga_libgroonga_support_lz4` and :ref:`mroonga_libgroonga_support_zlib`
  system variables. [GitHub#33,#34,#35] [Patch by Naoya Murakami]
* Dropped MySQL 5.1 support.

Fixes
^^^^^

* [storage][mariadb10] Fixed a crash bug that it doesn't properly locked.
  [GitHub#18] [Reported by Elena Stepanova]
* [storage] Fixed overflow about signed tinyint. [GitHub#29]
* [doc] Fixed markups about normalizer. [GitHub#32] [Patch by Naoya Murakami]

Thanks
^^^^^^

* Elena Stepanova
* Naoya Murakami

.. _release-4-06:

Release 4.06 - 2014/09/29
-------------------------

Improvements
^^^^^^^^^^^^

* [doc] Added more details about Groonga's development package.
* [doc] Changed to recommend GitHub issue tracker.
* [deb] Dropped Debian jessie and sid support.

Fixes
^^^^^

* [storage] Fixed a bug that no record returns with multiple column index.
  This bug occurs when it meets two conditions. First, columns indexed by
  multiple column index are used in order and WHERE clause. Second,
  a column indexed by multiple column index but it's no the first column is
  used in ORDER by clause.
  [#2651] [Reported by foamcentime, Naoya Murakami]

Thanks
^^^^^^

* foamcentime
* Naoya Murakami

.. _release-4-05:

Release 4.05 - 2014/08/29
-------------------------

Improvements
^^^^^^^^^^^^

* Dropped Ubuntu 13.10 Saucy Salamander support.
* Added new variable :ref:`mroonga_vector_column_delimiter`.
  It is used to change delimiter of vector column.
  [GitHub#16] [Patch by Naoya Murakami]
* [rpm][centos] Supported MySQL 5.6 official repository packages on CentOS 6/7.
  Use mysql56-community-mroonga package. [Patch by miko]
* [doc] Moved document about upgrading which is included in install document.
  Now, you can refer it as :doc:`/upgrade`.
* Supported MariaDB 10.0.13.
* [rpm][centos] Supported CentOS 7 as mariadb-mroonga package. It uses
  the bundled MariaDB.
  [groonga-dev,02604] [Tested by Miyawaki][Tested by Kawada]

Fixes
^^^^^

* [doc] Fixed wrong MySQL version about news of Mroonga 4.04. [Reported by _so4]
* [doc] Removed needless explicit install procedure for groonga-normalizer-mysql package.
* [storage] Fixed a bug that value remains in unique index on duplicated primary key error.
  [groonga-dev,02633] [Reported by 6elz]
* [doc] Fixed a bug that building document in other directory fails.
  [groonga-dev,02652] [Reported by cosmo0920]
* Changed to use "Mroonga" form for engine name.
* [storage] Fixed a bug that table specified index (``COMMENT 'table "XXX"'``)
  can't be removed. [groonga-dev,02677] [Reported by Naoya Murakami]


Thanks
^^^^^^

* _so4
* Naoya Murakami
* miko
* 6elz
* cosmo0920
* Miyawaki
* Kawada

.. _release-4-04:

Release 4.04 - 2014/07/29
-------------------------

Improvements
^^^^^^^^^^^^

* Removed Groonga and groonga-normalizer-mysql specific test files for
  MariaDB bundled version on Windows. There is too long file name issue.
  [groonga-dev,02391] [Reported by Masafumi Yokoyama]
* [doc] Updated :doc:`/developer`.
* Added overflow/underflow check whether valid time to find errors on 32-bit
  environment. [Patch by Toshihisa Tashiro]
* [storage] Supproted INPLACE ALTER TABLE for adding/dropping columns.
  [#2277] [GitHub#12] [Patch by Naoya Murakami]

Fixes
^^^^^

* [rpm][centos] Built with MySQL 5.5.37 on CentOS.
  [Reported by YOSHIDA Mitsuo]
* Fixed to use lowercase for table name to avoid  lower/upper-case specific
  issue on Mac OS. [Patch by Toshihisa Tashiro]
* Fixed build error for VC++ 2013 [GitHub#10] [Patch by cosmo0920]
* [doc] Added missing quote for install procedure on CentOS6 SCL.
  [GitHub#11] [Patch by Naoya Murakami]
* [storage] Fixed a bug that renaming column by ALTER TABLE CHANGE
  causes crash. [#2637]

Thanks
^^^^^^

* YOSHIDA Mitsuo
* Masafumi Yokoyama
* Toshihisa Tashiro
* cosmo0920
* Naoya Murakami

.. _release-4-03:

Release 4.03 - 2014/05/29
-------------------------

Improvements
^^^^^^^^^^^^

* [doc] Updated MariaDB version. [Patch by cosmo0920]
* Supported daylight saving time. [#2385]
* Migrated Ubuntu package distribution site to PPA on Launchpad.
  See :doc:`/install` for details.

Fixes
^^^^^

* [doc] Fixed command line in :doc:`/install`. [Reported by YOSHIDA Mitsuo]

Thanks
^^^^^^

* cosmo0920
* YOSHIDA Mitsuo

.. _release-4-02:

Release 4.02 - 2014/04/29
-------------------------

Improvements
^^^^^^^^^^^^

* Supported MariaDB 10.0.10 [#2460] [Reported by Kazuhiko Shiozaki]
* Supported MySQL 5.6.17.
* Supported Ubuntu 14.04 LTS Trusty Tahr.
* Enabled MariaDB bundled build.
* Dropped Ubuntu 12.10 Quantal Quetzal support.
* [doc] Updated MySQL version. [GitHub#8] [Patch by cosmo0920]

Fixes
^^^^^

* [storage] Stopped to use truncate for ``DELETE FROM table``.
  [groonga-dev,02222] [Reported by GMO Media, Inc.]
* [wrapper] Stopped to use truncate for ``DELETE FROM table``.
* [storage] Fixed a bug that inplace alter table with no primary key
  crashes. [groonga-dev,02227] [Reported by GMO Media, Inc.]
* [storage] Fixed a bug that ``ORDER BY function(vector_reference_column)``
  doesn't work.
  [groonga-dev,02234] [Reported by Naoya Murakami]
* Fixed a bug that setting the current value to ``mroonga_default_parser``
  or ``mroonga_log_file`` crash.
  [GitHub#6] [Patch by Satoshi MITANI]
* Fixed a bug that ``mroonga_lock_timeout`` in my.cnf or command line option
  is ignored.
  [GitHub#7] [Patch by GMO Media, Inc.]
* Fixed a bug that deleting by primary key doesn't update unique index.
  [groonga-dev,02244] [Reported by Akihiro Tsukui]

Thanks
^^^^^^

* Kazuhiko Shiozaki
* GMO Media, Inc.
* Naoya Murakami
* Satoshi MITANI
* Akihiro Tsukui
* cosmo0920

.. _release-4-01:

Release 4.01 - 2014/03/29
-------------------------

Improvements
^^^^^^^^^^^^

* Supported MariaDB 10.0.9 [#2387] [Reported by Kazuhiko Shiozaki]
* Supported ten or more sections in W pragma [#2348] [groonga-dev,02138]
  [Reported by yoku0825]
* [rpm][centos] Supported SCL MySQL package on CentOS 6.

Fixes
^^^^^

* [storage] Fixed a bug that ALTER TABLE with fulltext index which refer to table causes mysqld crash.
  [#2327] [groonga-dev,02130] [Reported by Naoya Murakami]
* [doc] Fixed to use Mroonga/Groonga (capitalized notation) in characteristics document.
  [GitHub#5] [Patch by Naoya Murakami]

Thanks
^^^^^^

* yoku0825
* Naoya Murakami
* Kazuhiko Shiozaki

.. _release-4-00:

Release 4.00 - 2014/02/09
-------------------------

* Bump version to 4.00! We recommend to upgrade because crash bug and updating issue are fixed now!

Improvements
^^^^^^^^^^^^

* Dropped Ubuntu 13.04 (Raring Ringtail) support.
* [storage] Supported to search with empty string.
  [#2214] [groonga-dev,02052] [Reported by Naoya Murakami]

Fixes
^^^^^

* Fixed a crash bug that bulk inserting null value into geometry column which has NOT NULL constraint.
  [#2281] [groonga-dev,02095] [Reported by yoku]
* [storage] Fixed a bug that existing records may be unexectedlly removed by ON DUPLICATE KEY
  UPDATE. In the previous versions, such a query can't update the value of column correctly.
  [#2263] [Reported Masahiro Nagano]

Thanks
^^^^^^

* yoku
* Naoya Murakami
* Masahiro Nagano

.. _release-3-12:

Release 3.12 - 2014/01/29
-------------------------

Improvements
^^^^^^^^^^^^

* Added a new system variable :ref:`mroonga_lock_timeout`. By changing this global variable,
  You can customize Groonga's lock timeout dynamically.
* Improved compatibility with Tritonn 'W' pragma's behaviour.
  In the previous versions, Mroonga used omitted section as weight 0. By this change,
  Mroonga uses omitted section as weight 1. This behaviour is same as Tritonn.
  [#2152] [Patch by Kenji Maruyama]

Fixes
^^^^^

* Fixed a crash bug on FLUSH TABLES during SELECT MATCH AGAINST ... [#2137] [Reported by yoku]
* Fixed wrong implementation for "W" pragma. It was changed to 1-origin. [#2151]
  In the previous versions, "W" pragma is implemented as 0-origin, but it is not compatible
  with Tritonn.
  This is incompatible change, please check existing query which use "W" pragma.
* Fixed a bug that searching empty records with "NOT" query returns duplicated results. [#2215]
  [groonga-dev,02052] [Reported by Naoya Murakami]


Thanks
^^^^^^

* yoku
* Naoya Murakami

.. _release-3-11:

Release 3.11 - 2013/12/29
-------------------------

Improvements
^^^^^^^^^^^^

* [deb] Dropped Debian 6.0 (squeeze) support.
* [deb] Dropped Ubuntu 10.04 (lucid) support.
* [storage] Supported optimization for "MATCH AGAINST AND INT_COLUMN OPERATOR XXX ORDER BY
  LIMIT" query. The OPERATOR in this query supports '<', '>', '<=' and '>='. [groonga-dev,01940]
  [Reported by Horikoshi Yuki]
* [storage] Supported optimization for "MATCH AGAINST AND DATETIME_COLUMN OPERATOR XXX ORDER BY
  LIMIT" query. The OPERATOR in this query supports '<', '>', '<=' and '>='. [groonga-dev,01943]
  [Suggested by yoku]
* [storage] Supported optimization for "MATCH AGAINST AND TIME_COLUMN OPERATOR XXX ORDER BY
  LIMIT" query. The OPERATOR in this query supports '<', '>', '<=' and '>='.
* [doc] Renamed documentation from 'UserGuide' to 'Tutorial'.
* [doc] Added 'FAQ' section for documentation.
* [doc] Added independant section for 'Full text search'.
* [storage] Supported 'BETWEEN' for INT_COLUMN, DATETIME_COLUMN and TIME_COLUMN.
  Note that it requres Groonga 3.1.1 or later. [groonga-dev,01943] [Suggested by yoku]
* Supported MariaDB 10.0.7 (it doesn't released yet.) [#1964]
* Dropped support for Microsoft Visual Studio 2008 or previous versions.
* Dropped Fedora 19 support.
* Supported Fedora 20.
* Dropped mysql-mroonga package on CentOS 5. Use mysql55-mroonga package instead.

Fixes
^^^^^

* [storage] Fixed a bug that MySQL crashes on 'LOCK TABLE .. READ' when dumping/restoring. [#2098]
* Fixed wrong implementation for "\*D-" pragma. [#2099]
* Fixed a memory leak on full text search. It affects such as "MATCH AGAINST ... ORDER BY LIMIT ...". [#2144]

Thanks
^^^^^^

* Horikoshi Yuki
* yoku

.. _release-3-10:

Release 3.10 - 2013/11/29
-------------------------

Improvements
^^^^^^^^^^^^

* Supported MariaDB 10.0.6.
* [plugin][mroonga_snippet] Improved to report not supported charset in error message.

Fixes
^^^^^

* Fixed multi range read disk sweep issue. [groonga-dev,01715] [#1959]
  The problem that query which has "WHERE IN (...)" clause causes an error is fixed. [Reported by Kimura]
* Fixed SEGV issue with trunk version of MariaDB [#1958]
* Fixed a bug that geometry column is not properly updated by
  'REPLACE INTO' or 'INSERT ON DUPLICATE KEY UPDATE' query. [#2057]
* Fixed compiler warnings on Windows. [#1964]
* Fixed the issue that vector column is no properly updated by
  'REPLACE INTO' query. [#2064]
* [mariadb10.0.5] Fixed build error which is derived from changed directory structure. [#2066]
* Fixed a bug that shorter column name such as "_i" is treated as "_id" which is
  special column. [#2085] [Reported by Alexander Barkov]

Thanks
^^^^^^

* Kimura
* Alexander Barkov

.. _release-3-09:

Release 3.09 - 2013/10/29
-------------------------

Improvements
^^^^^^^^^^^^

* Supported Ubuntu 13.10 (Saucy Salamander).
* [rpm][centos] Supported rpm packages for MySQL 5.5 on CentOS 5.
  It provides mysql55-mroonga to support mysql55-mysql packages.
  [groonga-dev,01860] [Reported by WING]

Fixes
^^^^^

* [wrapper] Fixed a memory leak. It occurs when wrapper
  handler doesn't destroy mutexes properly. MyISAM is known
  as effected storage engine.
* [wrapper] Fixed a bug that force index causes crush for MySQL 5.6.
  [#2015] [groonga-dev,01867] [Reported by Ichiro Yasuda]

Thanks
^^^^^^

* WING
* Ichiro Yasuda

.. _release-3-08:

Release 3.08 - 2013/9/29
-------------------------

Improvements
^^^^^^^^^^^^

* [doc] Added missing description about wrapper mode specific issue.
  [groonga-dev,01747] [Reported by Kazuhiro Isobe]
* [mroonga_escape] Supported to call mroonga_escape as an argument of UDF.
  [#1923] [Reported by Tomoatsu Shimada]
* [doc] Added language annotation to "Show Source" label.
  [groonga-dev,01747] [Reported by Kazuhiro Isobe]
* [wrapper] Supported REPAIR TABLE for broken groonga database.
  [groonga-dev,01540] [Suggested by Naoya Murakami]
* Supported MariaDB 10.x build with configure [groonga-dev,01727]
  [Reported by WING]

Fixes
^^^^^

* Fixed a bug that mroonga is accidentally removed on upgrade. [#1918]
  [Reported by @ceekz]
* Fixed a bug that mysqld 32bit executable binary crashes when install plugin on Windows.
* [storage][mariadb] Fixed a memory for mroonga_log_file.
  This memory leak occurs when log file name is changed.

Thanks
^^^^^^

* Kazuhiro Isobe
* Tomoatsu Shimada
* @ceekz
* Naoya Murakami
* WING

.. _release-3-07:

Release 3.07 - 2013/8/29
-------------------------

Improvements
^^^^^^^^^^^^

* Added :doc:`/reference/udf/mroonga_escape` UDF which escapes special characters in query
  for BOOLEAN MODE. [groonga-dev,01576] [Suggested by Kouhei Tanabe]
* Supported VARCHAR equal expression in WHERE clause for ORDER BY LIMIT optimization.
* Supported MariaDB 5.5.32 timestamp.
* Supported MariaDB 10.0.4. [Reported by WING] [Reported by yoku ts]

Fixes
^^^^^

* [mariadb] Fixed a crash bug when installing plugin on Windows.
* [storage][mysql55] Fixed a bug that changing column that has index fails.
  [groonga-talk] [Reported by Chang]
  This bug affects the case that ``FULLTEXT INDEX (column1)`` is changed by ``ALTER TABLE
  table1 CHANGE column1 column1 new_column_definition`` for example.
* [doc][wrapper] Fixed incorrect description about condition of ORDER BY LIMIT optimization.
* [storage] Disabled ORDER BY LIMIT optimization for not indexed VARCHAR condition.
  It should be indexed to handle COLLATION properly.
* Fixed a bug that missing internal flag causes crash on MySQL 5.6.13.

Thanks
^^^^^^

* Kouhei Tanabe
* Chang
* WING
* yoku ts

.. _release-3-06:

Release 3.06 - 2013/7/29
-------------------------

Improvements
^^^^^^^^^^^^

* Added :ref:`mroonga_action_on_fulltext_query_error` session variable.
  This affects how to report errors about invalid escape.
  [groonga-dev,01529] [Reported by Kouhei Tanabe]
* Supported count skip optimization for wrapper mode even though
  MySQL 5.5 or MariaDB 10.x. [#1841] [groonga-dev,01523] [Reported by Naoya Murakami]
  This optimization does not work if you use wrapper mode with
  storage engine which supports transaction.
* Supported Fedora 19.
* Dropped Fedora 18 support.

Thanks
^^^^^^

* Kouhei Tanabe
* Naoya Murakami

.. _release-3-05:

Release 3.05 - 2013/6/29
-------------------------

Improvements
^^^^^^^^^^^^

* Added warnings for truncated date data. [#1768] [Suggested by Y.Kentaro]
* Supported MySQL 5.6.12. [Reported by WING]
* Added documentation about troubleshooting.
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
   in :doc:`/tutorial/storage`, please recreate (dump and restore)
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
   :doc:`/tutorial/storage` and any multiple comlumn indexes, please
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


Past releases
-------------

.. toctree::
   :maxdepth: 2

   news/1.x
   news/0.x
