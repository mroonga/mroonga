:orphan:

.. highlightlang:: none

News
====

.. _release-6-05:

Release 6.05 - 2016-06-29
-------------------------

Improvements
^^^^^^^^^^^^

* [centos] Supported CentOS 6.8.

* [storage] Supported multibyte column name in inplace ALTER TABLE.
  Note that MySQL 5.6 or later can use inplace ALTER TABLE for adding
  columns and indexes.

* [storage] Supported ORDER BY LIMIT optimization for multibyte column.
  [Gitter:groonga/ja:575e6e671cf76dd64536997c][Reported by yoyoshifgs]

Fixes
^^^^^

* [storage count skip] Fixed invalid optimization which should no be
  applied. It affects to the result of row count.
  [Gitter:groonga/ja:5761ea97da1c26b045368c84][Reported by yoyoshifgs]

* Fixed to apply custom normalizer even if collation is bin family.
  [Gitter:groonga/ja:576d2a2d80f1c6a5257f1270][Reported by
  big_bridge_]

Thanks
^^^^^^

* yoyoshifgs
* big_bridge_

.. _release-6-03:

Release 6.03 - 2016-05-29
-------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/install/centos`] Supported Percona Server 5.7 on CentOS 6/7.

* [UDF][``mroonga_snippet_html``] Supported query format as keywords::

    mroonga_snippet_html(column, '...query format...' AS query);

  ``AS query`` is important in the above example.

* [:doc:`/install/windows`] Supported MariaDB 10.1.14 on Windows.

Fixes
^^^^^

* [storage] Fixed a bug that full text index against multibyte column
  doesn't work. [Gitter:groonga/ja:5745315728011d9f574b9425][Reported
  by yoyoshifgs]

Thanks
^^^^^^

* yoyoshifgs

.. _release-6-02:

Release 6.02 - 2016-04-29
-------------------------

Improvements
^^^^^^^^^^^^

* [deb] Supported multiarch. [Reported by hirobanex]

* [:doc:`/developer/release`] Updated to use Visual Studio 12(2013) in
  building instruction in Windows.

* [:doc:`/reference/full_text_search/boolean_mode`] Translated
  documentation about boolean mode.

* [experimental][mariadb10.2] Supported MariaDB 10.2.0.

* [:doc:`/install/debian`] Dropped Debian 7.0 (Wheezy).
  It had been reached to End of Life on April 26, 2016.

* [:doc:`/install/ubuntu`] Supported Ubuntu 16.04 (Xenial Xerus)

* [storage] Supported mutlibyte characters in column name.
  [Gitter:groonga/ja:570270f7d478c81e2cbcdc89][Reported by yoyoshifgs]

Fixes
^^^^^

* Fixed compile error with MySQL 5.7.

Thanks
^^^^^^

* hirobanex
* yoyoshifgs

.. _release-6-01:

Release 6.01 - 2016-03-29
-------------------------

Improvements
^^^^^^^^^^^^

* Added a warning when deprecated keyword "parser" is used.
  [Patch by GMO Media, Inc.]
* [storage] Reduced index size when ``WITH_POSITION`` is used without tokenizer.
  Stopped to add ``WITH_POSITION`` to index that doesn't use tokenizer.
  Index without tokenizer doesn't need to store position information.
  Because there is only one token.
* [:doc:`/install/windows`] Fixed URLs of binaries for Windows. [Reported by torinky]
* Added missing LZ4 source. [GitHub#100][Reported by Hiroshi Hatake]
* [:doc:`/install/ubuntu`] Dropped Ubuntu 15.04 (Vivid Vervet) support. It had been
  reached to End of Life on February 4, 2016.
* [:doc:`/install/windows`] Updated base MariaDB to 10.1.13.

Fixes
^^^^^

* Fixed a bug that the following UDFs can't be used in ``MATCH AGAINST``.
  [groonga-dev,03964][Reported by Hironori Matsubara]

  * ``mroonga_escape()``
  * ``mroonga_normalize()``
  * ``mroonga_snippet()``
  * ``mroonga_snippet_html()``

* [storage] Fixed a bug that ``DELETE`` without condition remains unique indexes.
  [GitHub#99][Reported by GMO Media, Inc.]

Thanks
^^^^^^

* GMO Media, Inc.
* Hironori Matsubara
* torinky
* Hiroshi Hatake

.. _release-6-00:

Release 6.00 - 2016-02-29
-------------------------

Improvements
^^^^^^^^^^^^

* [:ref:`boolean-mode-pragma-ss`] Added a pragma to use `script syntax
  <http://groonga.org/docs/reference/grn_expr/script_syntax.html>`_
  for search. [GitHub#95][Patch by Naoya Murakami]

* [Windows] Bundled LZ4.

* [Windows] Updated base MariaDB to 10.1.12.

Fixes
^^^^^

* [:doc:`/reference/udf/mroonga_normalize`] Add missing ``CREATE
  FUNCTION`` to install SQL. [GitHub#94][Patch by Naoya Murakami]

Thanks
^^^^^^

* Naoya Murakami

.. _release-5-12:

Release 5.12 - 2016-01-29
-------------------------

Fixes
^^^^^

* [storage] Fixed a bug that no records may be returned for range
  search when you have many deleted or updated records.
  [groonga-dev,03802][Reported by Hiroshi Kagami]

* [:doc:`/reference/udf`] Fixed a bug that wrong value may be returned
  on ``JOIN``. [Reported by teamsky]

Thanks
^^^^^^

* Hiroshi Kagami

* teamsky

.. _release-5-11:

Release 5.11 - 2015-12-29
-------------------------

Improvements
^^^^^^^^^^^^

* [rpm][mysql56-community-mroonga] Updated target MySQL to 5.6.28.
  [Reported by @stealthinu]

* [rpm][mysql57-community-mroonga] Updated target MySQL to 5.7.10.
  [Reported by @stealthinu]

* Stopped to use ``-`` character in internal lexicon name. Because
  ``-`` character can't be used symbol name in query syntax in
  Groonga. It's inconvenient in
  :doc:`/reference/udf/mroonga_command`. ``#`` character that can be
  used symbol name is used instead of ``-`` character since this
  release. [groonga-dev,03714] [Reported by keizi murakami]

  This change doesn't break backward compatibility. The latest Mroonga
  can use databases created by old Mroonga without any changes.

* [rpm] Suppressed a confusing error message when root password check.
  [GitHub#90] [Patch by Larry Kim]

* [rpm][mariadb-mroonga] Rebuilt for CentOS 7.2.
  [GitHub#91] [Reported by Larry Kim]

* [UDF][:doc:`/reference/udf/mroonga_normalize`] Added a new UDF that
  normalizes a string.
  [GitHub#89][GitHub#92] [Patch by Naoya Murakami]

Fixes
^^^^^

* [wrapper] Fixed a bug that ``CHECK TABLE FOR UPGRADE`` drops other tables.
  [groonga-dev,03699] [Reported by Tomohiro KATO]

Thanks
^^^^^^

* Tomohiro KATO
* @stealthinu
* keizi murakami
* Larry Kim
* Naoya Murakami

.. _release-5-10:

Release 5.10 - 2015-11-29
-------------------------

Improvements
^^^^^^^^^^^^

* [rpm][mysql57-community-mroonga] Supported auto Mroonga registration.
  [GitHub#78] [Patch by GMO Media, Inc.]

* [rpm][mysql57-community-mroonga] Supported CentOS 7.

* [wrapper] Supported ``CHECK TABLE FOR UPGRADE``.
  [groonga-dev,03688] [Reported by Tomohiro KATO]

Fixes
^^^^^

* [storage] Fixed a bug that auto repair may cause crash.
  [groonga-dev,03608] [Reported by Hiroshi Kagami]

* Fixed a crash bug when you use UDF concurrency.
  [groonga-dev,03640] [Reported by Hironori Matsubara]

* [wrapper] Fixed a bug that ``DROP TABLE`` may keep files.
  [groonga-dev,03673] [Reported by Tomohiro KATO]

Thanks
^^^^^^

* GMO Media, Inc.
* Hiroshi Kagami
* Hironori Matsubara
* Tomohiro KATO

.. _release-5-09:

Release 5.09 - 2015-10-29
-------------------------

Improvements
^^^^^^^^^^^^

* [rpm][mysql56-community-mroonga] Updated target MySQL to 5.6.27.
  [Reported by @star_orihime]

* [UDF][``mroonga_snippet_html``] Added a new UDF that returns snippet
  for HTML output.

* [Windows] Bundled MeCab.

* [Windows] Updated base MariaDB to 10.1.8.

* [MariaDB 10.1] Supported MariaDB 10.1.8.

* Supported auto repair on crash.
  [groonga-dev,03515][Suggested by Hiroshi Kagami]

* [MySQL 5.7] Supported MySQL 5.7.9.

* [MySQL 5.7] Supported JSON type.

* [:doc:`/install/ubuntu`] Added Ubuntu 15.10 Wily Werewolf support.

* [:doc:`/install/centos`] Added MySQL 5.7 package provided by Oracle support on CentOS 6.

Fixes
^^^^^

* Fixed a bug that ``MRBContains()`` doesn't use index.
  [GitHub#73] [Reported by Petri Rautiainen]

Thanks
^^^^^^

* @star_orihime

* Hiroshi Kagami

* Petri Rautiainen

.. _release-5-08:

Release 5.08 - 2015-09-29
-------------------------

Improvements
^^^^^^^^^^^^

* [CMake][MariaDB 10.1] Supported ``PLUGIN_MROONGA=NO``.
* [UDF] Supported ``grn_ctx`` pool. It improves performance for
  calling UDF because Mroonga can reduce ``grn_ctx`` initialize cost.

Fixes
^^^^^

* Fixed a memory leak when ``ORDER BY LIMIT`` is used with multiple
  ``MATCH AGAINST``.
  [groonga-dev,03496] [Reported by Gosuke Yasufuku]
* Fixed a bug that ``ORDER BY LIMIT`` and multiple ``MATCH AGAINST``
  returns wrong result.
  [groonga-dev,03496] [Reported by Gosuke Yasufuku]

Thanks
^^^^^^

* Gosuke Yasufuku

.. _release-5-06:

Release 5.06 - 2015-08-31
-------------------------

Improvements
^^^^^^^^^^^^

* Supported MariaDB 10.1 that is built as embedded server.
  [MDEV-8508][GitHub#66] [Reported by Sergei Golubchik]
* [rpm][mysql55-mroonga] Updated build target MySQL version.
  [groonga-dev,03421] [Reported by Hiroshi Kagami]

Fixes
^^^^^

* [rpm][percona-server-56-mroonga] Fixed a crash bug.
  [GitHub#70] [Patch by GMO Media, Inc.]
* Fixed a crash bug when any opening table exists on shutdown.
  [GitHub#71] [Reported by GMO Media, Inc.]

Thanks
^^^^^^

* GMO Media, Inc.
* Sergei Golubchik
* Hiroshi Kagami

.. _release-5-05:

Release 5.05 - 2015/07/29
-------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/install/ubuntu`] Dropped Ubuntu 14.10 (Utopic Unicorn) support. It had been
  End of Life on July 23, 2015.
* [rpm][:doc:`/install/centos`] Updated to the latest MySQL 5.6 (5.6.26-2) on CentOS 6/7.
* [rpm][:doc:`/install/centos`] Updated to the latest Percona Server 5.6 (5.6.25-rel73.1)
  on CentOS 6.

Fixes
^^^^^

* Fixed a bug that search results are reduced for multiple column index with range
  condition (e.g. ``<`` in ``WHERE date < '2015-07-01'``).
  [`groonga-dev,03332 <http://osdn.jp/projects/groonga/lists/archive/dev/2015-July/003334.html>`_]
  [GitHub#65] [Reported by foamcentime]
* [storage] Fixed memory leaks.
  [`MDEV-8520 <https://mariadb.atlassian.net/browse/MDEV-8520>`_]
  [Report by Elena Stepanova]

Thanks
^^^^^^

* foamcentime
* Elena Stepanova
* Sergei Golubchik

.. _release-5-04:

Release 5.04 - 2015/06/29
-------------------------

Improvements
^^^^^^^^^^^^

* [rpm][mysql56-community-mroonga][percona-server-56-mroonga]
  Start mysqld when mysqld is not running within rpm-installation.
  (This topic is at 5.03-2) [GitHub#58] [Patch by GMO Media, Inc.]
* [mariadb10.1] Followed recent API changes.
* [mariadb] Supported custom parameters in DDL.
  This feature can be used only with MariaDB.

  * Supported ``TOKENIZER`` parameter for ``FULLTEXT IDNEX`` ::

      CREATE TABLE diaries (
        id int PRIMARY KEY AUTO_INCREMENT,
        body text,
        FULLTEXT INDEX body_index (body) TOKENIZER='TokenBigramSplitSymbolAlphaDigit'
      ) ENGINE = Mroonga DEFAULT CHARSET = utf8;

  * Supported ``NORMALIZER`` parameter for ``FULLTEXT IDNEX`` and normal ``INDEX`` ::

      CREATE TABLE memos (
        id INT NOT NULL PRIMARY KEY,
        content TEXT NOT NULL,
        FULLTEXT INDEX (content) NORMALIZER='NormalizerAuto'
      ) ENGINE = Mroonga DEFAULT CHARSET = utf8;

  * Supported ``TOKEN_FILTERS`` parameter for ``FULLTEXT IDNEX`` ::

      CREATE TABLE memos (
        content VARCHAR(64) NOT NULL,
        FULLTEXT INDEX (content) TOKEN_FILTERS='TokenFilterStopWord,TokenFilterStopWord'
      ) ENGINE = Mroonga COMMENT = 'engine "InnoDB"' DEFAULT CHARSET = utf8;

  * Supported ``FLAGS`` parameter for ``FULLTEXT INDEX`` and normal ``INDEX`` ::

      CREATE TABLE memos (
        content VARCHAR(64) NOT NULL,
        FULLTEXT INDEX (content) FLAGS='WITH_POSITION|WITH_WEIGHT'
      ) ENGINE = Mroonga DEFAULT CHARSET = utf8;


  * Supported ``GROONGA_TYPE`` parameter for field ::

      CREATE TABLE tags (
        name VARCHAR(64) PRIMARY KEY
      ) ENGINE = Mroonga DEFAULT CHARSET = utf8 COLLATE = utf8_bin;

      CREATE TABLE bugs (
        id INT UNSIGNED PRIMARY KEY,
        tag VARCHAR(64) GROONGA_TYPE='tags'
      ) ENGINE = Mroonga DEFAULT CHARSET = utf8;

* [storage] Report error for invalid datetime related value on ``STRICT_TRANS_TABLES``.
  [groonga-dev,03299] [Suggested by GMO Media, Inc.]

  * It's backward incompatible change. For example:

    * Prepare (common) ::

        mysql> CREATE TABLE timestamps (
            ->   id INT PRIMARY KEY AUTO_INCREMENT,
            ->   create_dt DATETIME
            -> ) ENGINE = Mroonga DEFAULT CHARSET = utf8;
        Query OK, 0 rows affected (0.09 sec)

        mysql> SET sql_mode='';
        Query OK, 0 rows affected (0.01 sec)

        mysql> INSERT INTO timestamps (create_dt) VALUES ("2001-00-00 00:00:00");
        Query OK, 1 row affected, 1 warning (0.00 sec)

        mysql> SHOW WARNINGS;
        +---------+------+------------------------------------------------+
        | Level   | Code | Message                                        |
        +---------+------+------------------------------------------------+
        | Warning | 1265 | Data truncated for column 'create_dt' at row 1 |
        +---------+------+------------------------------------------------+
        1 row in set (0.00 sec)

        mysql> SELECT * FROM timestamps;
        +----+---------------------+
        | id | create_dt           |
        +----+---------------------+
        |  1 | 2001-01-01 00:00:00 |
        +----+---------------------+
        1 row in set (0.00 sec)

        mysql> SET sql_mode='STRICT_TRANS_TABLES';
        Query OK, 0 rows affected (0.01 sec)

    * Before (5.03 or earlier) ::

        mysql> INSERT INTO timestamps (create_dt) VALUES ("2002-00-00 00:00:00");
        ERROR 1265 (01000): Data truncated for column 'create_dt' at row 1

        mysql> SELECT * FROM timestamps;
        +----+---------------------+
        | id | create_dt           |
        +----+---------------------+
        |  1 | 2001-01-01 00:00:00 |
        |  2 | 2002-01-01 00:00:00 |
        +----+---------------------+
        2 rows in set (0.00 sec)

    * After (5.04 or later) ::

        mysql> INSERT INTO timestamps (create_dt) VALUES ("2002-00-00 00:00:00");
        ERROR 22003: Out of range value for column 'create_dt' at row 1

        mysql> SELECT * FROM timestamps;
        +----+---------------------+
        | id | create_dt           |
        +----+---------------------+
        |  1 | 2001-01-01 00:00:00 |
        +----+---------------------+
        1 row in set (0.00 sec)

* Changed keyword to use custom tokenizer to ``tokenizer`` from ``parser``.

  * In index comment: ``parser`` -> ``tokenizer``.
  * Server variable: ``mroonga_default_parser`` -> ``mroonga_default_tokenizer``.
  * ``parser`` and ``mroonga_default_parser`` are deprecated but they are
    available at least Mroonga 6.XX.

* Renamed parameter name for flags of index column.

  * ``index_flags`` -> ``flags``.
  * ``index_flags`` is deprecated but it will be usable on Mroonga 6.XX. It
    may be removed at Mroonga 7.00.

* [storage] Show error message when nonexistent Groonga type is specified to column.
* [storage] Renamed parameter name for column's Groonga type.

  * ``type`` -> ``groonga_type``.
  * ``type`` is deprecated but it will be usable on Mroonga 6.XX. It may be
    removed at Mroonga 7.00.

Thanks
^^^^^^

* GMO Media, Inc.

.. _release-5-03:

Release 5.03 - 2015/05/29
-------------------------

Improvements
^^^^^^^^^^^^

* mariadb10.1: Followed recent API changes.
* Supported ``FT_SORTED`` flag which is internally used in MySQL. It improves compatibility with
  MySQL and can reduces redundant sorting in MySQL.
* mysql57: Followed recent API changes.

Fixes
^^^^^

* [storage] Fixed a bug that unique index doesn't work for invalid datetime.
  This bug is occurred when invalid datetime is inserted. The unique index is created for invalid
  datetime instead of actual truncated datetime.
  [groonga-dev,03219] [Reported by Hiroshi Kagami]
* [multiple column key] Fixed a potential bug that decoding value is broken. This bug may occurs
  because proper byte order conversion between network and host is missing on decoding.
* [windows] Fixed a bug that needless groonga-normalizer-mysql plugin search is ran
  when it is embedded into Mroonga. [GitHub#53] [Reported by torinky]
* Fixed wrong keyword length when query includes :ref:`boolean-mode-pragma`. This bug causes that it doesn't hit
  expected search results. [GitHub#54] [Patch by Naoya Murakami]
* [storage] Fixed a bug that unique check is ignored for multiple connections.
  [groonga-dev,03243] [Reported by Hiroshi Kagami]

Thanks
^^^^^^

* Hiroshi Kagami
* torinky
* Naoya Murakami

.. _release-5-02:

Release 5.02 - 2015/04/29
-------------------------

Improvements
^^^^^^^^^^^^

* [doc] Updated :doc:`/developer/release` procedure documentation
* [storage] Improved performance to estimate the number of records in range.
  In the previous versions, Mroonga counts real the number of records in range.
  There is a performance penalty if huge number of records exists.
  Note that it requires Groonga 5.0.2 or later.
  [groonga-dev,03150] [Reported by Masato Shimada]
* [experimental] Added
  :ref:`server-variable-mroonga-max-n-records-for-estimate` variable
  to limit the max number of records to estimate. It reduces the
  estimation cost when there are many target records.
* [rpm][centos] Updated to build against the latest MySQL 5.6.
* [wrapper] Supported fast order by limit optimization for primary key sort.
  [Reported by Tsugunori Nashiro]
* Supported Debian 8.0 (Jessie)
* Supported Ubuntu 15.04 (Vivid Vervet)

Fixes
^^^^^

* [doc] Fixed old links to Windows package [Reported by METAL_GEAR_mkII]
* [storage][mysql56] Fixed a crash bug by duplicated ``ORDER BY``
  columns.  It's occurred when "fast order limit" optimization is
  detected with duplicated ``ORDER BY`` columns in SQL. Note that this
  bug doesn't affect to MySQL 5.5. [GitHub#50] [Reported by GMO Media, Inc.]

Thanks
^^^^^^

* Masato Shimada
* Tsugunori Nashiro
* METAL_GEAR_mkII
* GMO Media, Inc.

.. _release-5-01:

Release 5.01 - 2015/03/29
-------------------------

Improvements
^^^^^^^^^^^^

* [storage] Supported ``PARTITION BY RANGE``. Note that this feature is not supported on MariaDB 10.
  This limitation is derived from MariaDB 10 architecture about removing .par file.
* [mysql56] Disabled in-place ``ALTER TABLE`` for ``PRIMARY KEY``. This change is derived from Groonga's limitation because Groonga doesn't support to change table key type. [Reported by Hiromitsu KATO]
* Dropped Visual Studio 2010 and Visual Studio 2012 support. Use Visual Studio 2013 or later to build Mroonga. [GitHub#45]
* [windows] Added Visual Studio 2015 build support.
* Supported to specify options by table comment when primary key is using hash by ``PRIMARY KEY (...) USING HASH``.
  [GitHub#46] [Patch by Naoya Murakami]
* Supported index column flags by index comment. You can specify ``NONE``, ``WITH_POSITION``, ``WITH_SECTION`` and ``WITH_WEIGHT`` as index column flags. Use ``FULLTEXT INDEX (...) COMMENT 'index_flags "WITH_POSITION|WITH_SECTION"'`` for example. [GitHub#47] [Patch by Naoya Murakami]
* Supported to build with MySQL 5.7.
* [rpm][centos] Supported Percona Server 5.6.x on CentOS 6/7. [Tested on CentOS 6 by Yoshino]
* Supported ``utf8_unicode_520_ci`` and ``utf8mb4_unicode_520_ci``. To support these collations, Mroonga now requires `groonga-normalizer-mysql <https://github.com/groonga/groonga-normalizer-mysql>`_ 1.0.9.

Fixes
^^^^^

* Changed to store score in float. This backward incompatible change is derived from the Groonga DB API change in Groonga 5.0.1. There may be a case the value of score is different.
* Added missing error check when failed to create multiple column index in in-place ``ALTER TABLE``.
* [mariadb] Fixed crash bug when ``SET`` variable is used. This bug depends on MariaDB version. (<= 5.5.41 and <= 10.0.16)
* [rpm][centos] Fixed release number for CentOS 7. [GitHub#44] [Reported by CharAz]
* [mariadb55] Fixed install failure after installing and uninstalling Mroonga.

Thanks
^^^^^^

* Hiromitsu KATO
* Naoya Murakami
* Yoshino
* CharAz

.. _release-5-00:

Release 5.00 - 2015/02/09
-------------------------

* Bump version to 5.00!

Improvements
^^^^^^^^^^^^

* [appveyor] Supported Windows CI on `AppVeyor <http://www.appveyor.com/>`_.
  We can get notification about build failure on Windows at once.
  Subscribe to groonga-mysql-commit@lists.sourceforge.jp if you want build status.
* [rpm][centos] Build against MySQL 5.6.23-2 on MySQL yum repository.
  [groonga-dev,03083][Reported by Kohei Aochi]

Fixes
^^^^^

* [cmake] Disabled big endian support explicitly.

Thanks
^^^^^^

* Kohei Aochi

Past releases
-------------

.. toctree::
   :maxdepth: 2

   news/4.x
   news/3.x
   news/2.x
   news/1.x
   news/0.x
