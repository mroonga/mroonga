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



Past releases
-------------

.. toctree::
   :maxdepth: 2

   news/2.x
   news/1.x
   news/0.x
