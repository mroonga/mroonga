:orphan:

.. highlightlang:: none

News
====

.. _release-9-09:

Release 9.09 - 2019-10-29
-------------------------

.. note::

    Maybe performance decreases from this version.
    Therefore, If performance decreases than before, please report us with reproducible steps.

Fixes
^^^^^

* Fixed a build error a package for MySQL 8.

.. _release-9-08:

Release 9.08 - 2019-09-27
-------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/install/centos`] Added support for MariaDB 5.5.64, 10.2.27, 10.3.18, and 10.4.8

* [:doc:`/install/centos`] Dropped 32-bit package support on CentOS 6.

Fixes
^^^^^

* [:doc:`/install/debian`] Fixed that can't install mariadb-server-10.3-mroonga in Debian 10(buster).

Thanks
^^^^^^

* kajiys

* bizlevel

.. _release-9-07:

Release 9.07 - 2019-08-29
-------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/install/debian`] Added a install document for Debian 10(buster).

* [:doc:`/install/centos`] Added support for MariaDB 10.1.41, 10.2.26, 10.3.17, and 10.4.7.

* [:doc:`/reference/server_variables`] Added a document for ``mroonga_query_log_file``.

* [:doc:`/install/others`] Added a document about how to uninstall Mroonga. [GitHub#135][Patched by ryfjwr]

* [:doc:`/tutorial/storage`] Added a document about how to use regular expression search.

* Dropped support for MariaDB 10.0

* [:doc:`/install/centos`] Added support for Percona Server 5.6.45 and
  5.7.27

* [:doc:`/install/centos`] Dropped support for MariaDB 10.x on CentOS 6

Thanks
^^^^^^

* ryfjwr

.. _release-9-05:

Release 9.05 - 2019-07-30
-------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/install/centos`] Added a description about current limitation with MySQL 8.0.

* [:doc:`/install/centos`] Added support for MySQL 5.6.45, 5.7.27 and 8.0.17.

Fixes
^^^^^

* [:doc:`/install/centos`] Fixed a wrong ``mysql80-comunity-release`` package name.
  [groonga-dev,04759][Reported by Kagami Hiroshi]

* [:doc:`/tutorial/storage`] Fixed an unique index update bug. This bug causes duplicated key error when the following conditions are met.

  * An unique index is created against multiple column index
  * partial unique key column is updated

  Note that if you already created an unique index, you must recreate target tables because unique index may have garbage entries. We recommend to recreate an target table with dump and restore, or execute ``ALTER TABLE (TABLE_NAME) FORCE``.

* [mysql8.0] Added a support for ``TIMESTAMP``
  [groonga-dev,04763][Reported by Kagami Hiroshi]

Thanks
^^^^^^

* Kagami Hiroshi

.. _release-9-04:

Release 9.04 - 2019-06-29
-------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/install/centos`] Added support for Percona Server 5.7.26.

* [:doc:`/install/centos`] Added support for MariaDB 10.2.25/10.3.16.

* [:doc:`/install/centos`][experimental] Added support for MySQL 8.0.16.

Fixes
^^^^^

* Fixed a infinite loop bug.
  This bug is occurred when invalid flag is specified such as ``FULLTEXT INDEX (...) COMMENT 'index_flags "INVALID|WITH_SECTION"'``.

* [windows] Fixed a inappropriate pdb path with MariaDB 10.2/10.3.

* [:doc:`/install/others`] Fixed a typo about missing appropriate account name in plugin install instruction.

* Fixed a crash bug with ``((MATCH OR MATCH) AND (MATCH))`` query.

.. _release-9-03:

Release 9.03 - 2019-05-29
-------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/tutorial/storage`] Updated example to use ``tokenizer "XXX"` in ``COMMENT`` because ``default_tokenizer "XXX"`` is deprecated since 9.01.

* [:doc:`/install/windows`] Added support to provide MariaDB 10.1, 10.2 zip package again.

* [:doc:`/install/centos`] Added support for MariaDB 10.3.14 and 10.3.15.

* [:doc:`/install/debian`] Updated install instruction for copy and paste friendly.

* Added support for ``INDEX_LARGE`` flag such as ``COMMENT 'flags "INDEX_LARGE"'`` syntax.

* [:doc:`/install/centos`] Added support for MariaDB 10.2.24.

* [:doc:`/install/centos`] Added support for MariaDB 10.1.40.

* [:doc:`/install/ubuntu`] Added support for Ubuntu 19.04 (Disco Dingo)

* [:doc:`/install/centos`] Added support for Percona Server 5.6.44.

* [:doc:`/install/centos`] Added support for MySQL 5.6.44 and 5.7.26.

.. _release-9-01:

Release 9.01 - 2019-03-29
-------------------------

Improvements
^^^^^^^^^^^^

* [storage] Added support for tokenizer options.

  * For example, you can specify tokenizer options in COMMENT section such as ``CREATE TABLE foo (...) COMMENT='tokenizer "TokenNgram(''n'', 4)"'``.

* [mariadb] Added support for "tokenizer" table parameter.

  * For example, you can specify "tokenizer" such as ``CREATE TABLE foo (...) TOKENIZER='TokenNgram("n", 4)'``.

* [storage] Added support for tokenizer parameter in comment.

  * Note that ``default_tokenizer`` is deprecated.

* [mariadb] Added support for "normalizer" table parameter.

  * For example, you can specify "normalizer" such as ``CREATE TABLE foo (...) NORMALIZER='NormalizerNFKC100("unify_kana", true)'``.

* [mariadb] Added support for "token_filters" table parameter.

  * For example, you can specify "token_filters" such as ``CREATE TABLE foo (...)  TOKEN_FILTERS='TokenFilterNFKC100("unify_katakana_v_sounds", true)'``.

* Added support for "LEXICON" index parameter.

  * For example, you can specify ``FULLTEXT INDEX foo (bar) LEXICON='terms'`` or ``FULLTEXT INDEX foo (bar) COMMENT 'lexicon "terms"'``.

* [appveyor] Improved support to building Mroonga enabled package [GitHub#230]

* [:doc:`/install/centos`] Added support for Percona Server 5.7.25-28.

* [:doc:`/install/centos`] Added support for MariaDB 10.3.13.

* [:doc:`/install/centos`] Added support for MariaDB 10.2.23.

.. _release-9-00:

Release 9.00 - 2019-02-09
-------------------------

This is a major version up! But It keeps backward compatibility.
You can upgrade to 9.00 without rebuilding database.

In Groonga 9.0.0, ``TokenPattern``, ``TokenTable`` tokenizer and
``remove_blank`` for ``NormalizerNFKC100`` is supported.
If you upgrade to Groonga 9.0.0, you can use them from Mroonga 9.00!

* ref: http://groonga.org/docs/news.html#release-9-0-0-2019-02-09

Improvements
^^^^^^^^^^^^

* [:doc:`/install/centos`] Supported Percona Server 5.7.24-27.

* [:doc:`/install/centos`] Supported Percona Server 5.6.43 rel84.3.

* [rpm][centos] Supported MariaDB 10.3.12.

* [rpm][centos] Supported MariaDB 10.2.21.

* [rpm][centos] Supported Percona Server 5.7.24-27.

* [rpm][centos] Supported Percona Server 5.6.43 rel84.3.

* [rpm][centos] Supported MySQL 5.7.25.

* [rpm][centos] Supported MySQL 5.6.43.

.. _release-8-09:

Release 8.09 - 2018-11-29
-------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/install/ubuntu`] Supported Ubuntu 18.10 (Cosmic Cuttlefish).

* [:doc:`/install/windows`] Supported MariaDB 10.3.10.

* [:doc:`/install/centos`] Supported MariaDB 10.2.19

* [:doc:`/install/centos`] Supported MariaDB 10.1.37

* [:doc:`/install/centos`] Supported Percona Server 5.7.23-25.

* [rpm][centos] Supported MariaDB 10.3.11.

* [rpm][centos] Supported MySQL 5.6.42.

* [rpm][centos] Supported MySQL 5.7.24.

Revision:

We deleted information as below.

"Supported MySQL 8"

Sorry, There was wrong release information in Mroonga 8.09.
The MySQL 8 is not supported. That is still being handled.

.. _release-8-07:

Release 8.07 - 2018-09-29
-------------------------

Improvements
^^^^^^^^^^^^

* Deprecated tokenizer ``off`` option. Use tokenizer ``none`` option instead.
* Dropped support for MariaDB 10.2.2 which is shipped at Sep 27, 2016 and older series.

Fixes
^^^^^

* [:doc:`/install/centos`] Supported MariaDB 10.1.36.

.. _release-8-06:

Release 8.06 - 2018-08-29
-------------------------

In this version, MySQL will be automatically restarted if you had
already installed Mroonga and not installed Groonga 8.0.4 or later.
Because Mroonga 8.06 requires Groonga 8.0.4 or later but it will not
reloaded until MySQL is restarted.

Improvements
^^^^^^^^^^^^

* Updated required Groonga version to 8.0.4 or later.

* Updated required groonga-normalizer-mysql version to 1.1.3 or later.

* Supported utf8mb4_0900 family collation.

  * ref: https://github.com/groonga/groonga-normalizer-mysql#description

* Supported tokenizer options.

  * e.g.: ``tokenizer "TokenNgram(\'loose_symbol\', true)"``

  * ref: http://groonga.org/docs/news.html#release-8-0-2-2018-04-29

* Use the Groonga's default logger.

* [:doc:`/install/windows`] Updated bundled MariaDB to 10.3.9 from 10.1.33.

  * NOTICE: Before upgrading to MariaDB 10.3, you need to dump existing MariaDB 10.1 databases. Then restore it after upgrading.

* [:doc:`/install/debian`] Dropped Debian 8 (jessie) support.

* [:doc:`/install/ubuntu`] Dropped Ubuntu 17.10 (Artful Aardvark) support.

* [WIP] Working on supporting MySQL 8.

  * The storage mode is almost done (JSON type doesn't work yet).

  * The wrapper mode is in progress.

Fixes
^^^^^

* [storage] Fixed a bug that wrong result may be returned on multi range read.
  [GitHub#211][Reported by colt27]

Thanks
^^^^^^

* colt27

.. _release-8-03:

Release 8.03 - 2018-05-29
-------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/install/docker`] Added quick start guide link to Docker Hub.

* [:doc:`/install/centos`] Supported MariaDB 10.3.7.

* [:doc:`/install/centos`] Supported MariaDB 10.2.15 (backported to 8.02).

* [:doc:`/install/centos`] Supported MariaDB 10.1.33 (backported to 8.02).

Fixes
^^^^^

* [:doc:`/install/ubuntu`] Fixed install failure on Ubntu 14.04 LTS (Trusty)
  (backported to 8.02). [GitHub#202,#205][Reported by Masato Hirai]

Thanks
^^^^^^

* Masato Hirai

.. _release-8-02:

Release 8.02 - 2018-04-29
-------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/install/ubuntu`] Supported Ubuntu 18.04 LTS (Bionic Beaver).

* [:doc:`/install/debian`] Supported i386 for Jessie.

* Suppress meaningless "duplicated unique id" error log. [GitHub#197]

* [developer][test] Supported `--record` option.

* [:doc:`/install/centos`] Use `groonga-release-latest` instead of `groonga-release-X.X.X`.

* [:doc:`/tutorial/installation_check`] Added version check howto.

* [:doc:`/install/centos`][percona] Supported upgrading from "< 5.6.34" and "< 5.7.21".
  [groonga-dev,04599][Reported by Takashi Kinoshita][Looked into by Satoshi Mitani]

* [:doc:`/install/centos`] Supported MySQL 5.6.40 and 5.7.22.

* [:doc:`/install/centos`] Supported Percona Server 5.7.21-21.

Fixes
^^^^^

* Fixed a crash bug when some complex condition in `ORDER BY` such as `ORDER BY 1 + 1, id, content`.

* Fixed a bug that `MATCH AGAINST` condition is ignored if SQL containing such as
  `AND (x = 1 OR x = 2)` when condition push down is enabled.
  [Gitter/ja:5ae014842b9dfdbc3ac7ce1f][Reported by colt27]

* Fixed a memory leak for column caches.

Thanks
^^^^^^

* Takashi Kinoshita

* Satoshi Mitani

* colt27

.. _release-8-01:

Release 8.01 - 2018-03-29
-------------------------

In this version, MySQL will be automatically restarted if you had
already installed Mroonga. This is because Mroonga requires newer
version of Groonga (8.0.1) to fix bugs, but it will not reloaded until
MySQL is restarted.

Improvements
^^^^^^^^^^^^

* [rpm][centos] Supported Percona Server 5.6.39. [Reported by @iiiiyyyy]

* [rpm][centos] Supported Percona Server 5.7.21.

* [rpm][centos] Supported MariaDB 10.2.13. [GitHub#198] [Reported by
  shota suzuki]

* [rpm][centos] Supported MariaDB 10.2.14.

Fixes
^^^^^

* Fixed a bug that wrong cache for other database is used. If you
  create multiple database and use `mroonga_command()` against one of
  them, wrong cache is returned unexpectedly. To fix this issue,
  Groonga 8.0.1 or later is required.

* Fixed a bug that "NOT IN" query returns empty result. This bug
  occurs when "NOT IN" is used with multiple arguments such as "NOT IN
  (xxx, xxx)"

* Fixed a bug that specified "NOT IN" can't exclude correctly when
  condition push down is enabled.

* Fixed a bug that "ORDER BY RAND()" query returns wrong result. This
  bug occurs when "ORDER BY RAND()" and "LIMIT" is specified at the
  same time.

* Fixed a bug that "fast order limit" optimization is applied
  unexpectedly to "ORDER BY function()".

Thanks
^^^^^^

* @iiiiyyyy

* shota suzuki

.. _release-8-00:

Release 8.00 - 2018-02-09
-------------------------

This is a major version up! But It keeps backward compatibility.
You can upgrade to 8.0.0 without rebuilding database.

Improvements
^^^^^^^^^^^^

* When create hash index, used tiny hash for reducing used resource.

* [percona57] Added gap lock detection support.
  [GitHub#188][Reported by Iwo]

Thanks
^^^^^^

* Iwo

.. _release-7-11:

Release 7.11 - 2018-01-29
-------------------------

Improvements
^^^^^^^^^^^^

* [test] Added a test case for sub query and order limit optimization.
  [GitHub#184] [Reported by Kazuki Hamasaki]

* [rpm][centos] Supported  10.3.

* [deb][ubuntu] Supported MariaDB 10.1 for Ubuntu 17.10 (Artful Aadvark)

Fixes
^^^^^

* [:ref:`status-variable-mroonga-n-pooling-contexts`] Fixed a bug that
  value is reset unexpectedly by ``FLUSH STATUS``.

* [maradb10.3] Fixed a build error which is caused by renamed constant
  variable (``HA_MUST_USE_TABLE_CONDITION_PUSHDOWN`` is renamed to
  ``HA_CAN_TABLE_CONDITION_PUSHDOWN``).

* [:doc:`/install/macos`] Removed obsolete install guide and updated
  link to latest documentation [Reported by Ryuji AMANO]

* [rpm][centos] Supported MariaDB 10.2.12. [GitHub#186] [Reported by
  Shota Suzuki]

* [rpm][centos] Supported Percona Server 5.7.20-19.

* [:doc:`/install/ubuntu`] Dropped Ubuntu 17.04 (Zesty Zapus) support.
  It has reached EOL at Jan 13, 2018.

Thanks
^^^^^^

* Kazuki Hamasaki
* Ryuji AMANO
* Shota Suzuki

.. _release-7-10:

Release 7.10 - 2017-12-29
-------------------------

Improvements
^^^^^^^^^^^^

* Updated required Groonga version to 7.1.0. You need to restart MySQL
  after you upgrade to Mroonga 7.10.

* [mariadb10.3] Supported MariaDB 10.3.

* [rpm][centos] Supported MariaDB 10.2.11.

* [rpm][centos] Supported MariaDB 10.1.30.

* [rpm][centos] Supported Percona Server 5.7.20.

* [rpm][centos] Supported Percona Server 5.6.38.

* [:doc:`/reference/optimizations`] Enable count_skip optimization for multi-column index.

* Supported condition push down and added related new variables.

  * [:ref:`status-variable-mroonga-condition-push-down`] Added ``Mroonga_condition_push_down``.

  * [:ref:`server-variable-mroonga-condition-push-down-type`] Added
    ``mroonga_condition_push_down_type``. The default value is
    ``ONE_FULL_TEXT_SEARCH``. It means that condition push down is
    enabled only when ``WHERE`` clause has one ``MATCH AGAINST``
    condition.  If the value ``ALL`` is set, condition push down is
    always used (``ALL`` is experimental for now. We are glad if you
    use it and tell us if it got faster or not).

* Supported column cache when to get fixed size column value to improve performance.

* Renamed a function ``last_insert_grn_id`` to ``mroonga_last_insert_grn_id`` to add missing ``mroonga_`` prefix.
  ``last_insert_grn_id`` is deprecated.
  [GitHub#177] [Reported by Ian Gilfillan]

* [:ref:`status-variable-mroonga-n-pooling-contexts`] Added
  a new status variable to show the number of pooling contexts for
  :doc:`/reference/udf/mroonga_command`.

* [ ``FLUSH TABLES`` ] Added clearing pooling contexts for
  :doc:`/reference/udf/mroonga_command` support.

Thanks
^^^^^^

* Ian Gilfillan

.. _release-7-09:

Release 7.09 - 2017-11-29
-------------------------

Improvements
^^^^^^^^^^^^

* [rpm][centos] Supported MariaDB 10.2.10.

* [rpm][centos] Supported MariaDB 10.1.29.

* Fixed not to require sed to run tests. [Patch by Sergei Golubchik]

* [cmake] Changed to skip Mroonga related configurations on without
  Mroonga build. [Patch by Vladislav Vaintroub]

Thanks
^^^^^^

* Sergei Golubchik

* Vladislav Vaintroub

.. _release-7-08:

Release 7.08 - 2017-10-29
-------------------------

Improvements
^^^^^^^^^^^^

* Supported table level ``flags`` option.
  You can specify ``TABLE_HASH_KEY``, ``TABLE_PAT_KEY``, ``TABLE_DAT_KEY``, and ``KEY_LARGE``
  table options. [groonga-dev, 04494] [Reported by Masanori Miyashita]

* [rpm][centos] Supported MySQL 5.6.38-2 and 5.7.20-1.

* [:doc:`/install/ubuntu`] Supported Ubuntu 17.10 (Artful Aardvark).

Thanks
^^^^^^

* Masanori Miyashita

.. _release-7-07:

Release 7.07 - 2017-10-12
-------------------------

Improvements
^^^^^^^^^^^^

* [mroonga_query_expand] Added ``mroonga_query_expand`` UDF. If you
  prepare synonyms table in advance, you can get expanded synonym in
  your query by ``mroonga_query_expanded``. Note that Groonga 7.0.6 or
  later version is required to use this function.

* [rpm][centos] Supported Percona Server 5.7.19-17.1. [Reported by
  tigersun2000]

* [rpm][centos] Supported MariaDB 5.5.56-2. [Reported by akiko_pusu]

* [rpm][centos] Supported MariaDB 10.1/10.2 provided by MariaDB.

Fixes
^^^^^

* Fixed a bug that wrong database may be used on "DROP DATABASE".
  This bug may cause a crash because internal "mroonga_operations"
  table is removed unexpectedly. It may happen when the following two conditions are true:

  1. There are multiple databases that uses Mroonga.
  2. "DROP DATABASE" against no longer Mroonga tables exist.

  As unexpected result, "DROP DATABASE x" may remove
  "mroonga_operations" table on existing "y" database.

* Fix a crash bug after CHECK TABLE is used. [GitHub#167] [Reported by
  GMO Media, Inc.]

* [deb][mariadb10] Added missing dependency to lsb-release package for
  preinst and postrm maintainer script. [GitHub#169] [Patch by Tatsuki
  Sugiura]

Thanks
^^^^^^

* @tigersun2000
* @akiko_pusu
* GMO Media, Inc.
* Tatsuki Sugiura

.. _release-7-06:

Release 7.06 - 2017-08-29
-------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/reference/udf/mroonga_highlight_html`] Added usage about
  ``mroonga_highlight_html``.

* Supported generated column. It is useful to full-text search for
  partially extracted content from JSON column. [GitHub#159,#160,#163]
  [Patch by Naoya Murakami]

* Added :ref:`server-variable-mroonga-enable-operations-recording`. variable.
  [GitHub#158] [Patch by Naoya Murakami]

* Supported virtual column for MariaDB 10.2 and MySQL 5.7. It supports
  ``VIRTUAL`` type.  [GitHub#161,#162] [Patch by Naoya Murakami]

* Supported MariaDB 10.1.26.

* [rpm][centos] Supported Percona Server 5.6.36 rel82.1 and 5.7.18-16.
  [Gitter/ja:59894500bc46472974622cbd] [Reported by
  @tigersun2000_twitter]

* [rpm][centos] Supported MySQL 5.6.37 and 5.7.19 on CentOS 7.
  [groonga-dev,04441] [Reported by Kagami Hiroshi]

Thanks
^^^^^^

* Naoya Murakami
* @tigersun2000_twitter
* Kagami Hiroshi

.. _release-7-05:

Release 7.05 - 2017-07-29
-------------------------

Improvements
^^^^^^^^^^^^

* Supported Groonga query log. Use ``mroonga_query_log_file`` variable
  to use this feature. [GitHub#148]

* Supported MariaDB 10.2.7. [groonga-dev,04397] [Reported by Tomohiro
  'Tomo-p' KATO]

* [:doc:`/reference/udf/mroonga_command`] Supported database name that
  has special name such as ``db-1`` for example. It contains special
  character ``-``.

* [:doc:`/reference/udf/mroonga_command`] Supported auto command
  syntax escape feature. It makes easy to use Groonga functionality
  from Mroonga.

* Supported MariaDB 5.5.57.

* [rpm][centos] Supported MySQL 5.6.37-2 and MySQL 5.7.19-1 on
  CentOS 6. [groonga-dev,04403] [Reported by Kagami Hiroshi]

* [:doc:`/install/ubuntu`] Dropped Ubuntu 16.10 (Yekkety Yak) support.
  It has reached EOL at July 20, 2017.

* [:doc:`/reference/udf/mroonga_highlight_html`] Supported a function
  to highlight target column or text.

Fixes
^^^^^

* Fixed a crash bug when there is no active index internally.
  [Gitter:groonga/ja:596714a5c101bc4e3a7db4e5] [Reported by K
  Torimoto]

Thanks
^^^^^^

* K Torimoto
* Tomohiro 'Tomo-p' KATO
* Kagami Hiroshi

.. _release-7-04:

Release 7.04 - 2017-06-29
-------------------------

Improvements
^^^^^^^^^^^^

* Supported to show error message when failed to create a table for
  matched records. This kind of error occurs when indexes are
  broken. This error message helps to identify problem.

* [:doc:`/install/debian`] Supported Debian 9 (stretch).

Fixes
^^^^^

* Fixed a crash bug that missing ``NULL`` check before calling
  ``grn_table_setoperation`` causes. Such a crash bug occurs when
  indexes are broken.

.. _release-7-03:

Release 7.03 - 2017-05-29
-------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/reference/limitations`] Updated limitations about NULL in
  column. [Gitter/ja] [Reported by @bignum_twitter]

* Supported ``INDEX_MEDIUM`` and ``INDEX_SMALL`` flags. [GitHub#141]
  [Patch by Naoya Murakami]

* [:doc:`/install/centos`] Supported recent Percona Server 5.6.36 and
  5.7.18. [Reported by @pinpikokun]

Thanks
^^^^^^

* @bignum_twitter
* @pinpikokun
* Naoya Murakami

.. _release-7-02:

Release 7.02 - 2017-04-29
-------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/install/centos`] Dropped Ubuntu 12.04 (Precise Pangolin)
  support because of EOL.

* [:doc:`/install/ubuntu`] Added Zesty Zapus (Ubuntu 17.04) support.

Fixes
^^^^^

* [:doc:`/install/centos`] Fixed build error with MySQL 5.6.36 and
  MySQL 5.7.18.

* [cmake] Fixed missing link to ``libgroonga`` when mroonga is bundled
  and ``libgroonga`` isn't bundled. [GitHub#137] [Patch by Naoya
  Murakami]

Thanks
^^^^^^

* Naoya Murakami

.. _release-7-01:

Release 7.01 - 2017-03-29
-------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/install/centos`] Dropped CentOS 5 support because of EOL.

* [:doc:`/tutorial/storage`] Supported ``ALTER TABLE ADD/DROP FOREIGN
  KEY``.

* [:doc:`/tutorial/storage`] Supported fast ``ORDER LIMIT`` with
  ``ENUM``.  [groonga-dev,04277] [Reported by murata satoshi]

* Supported ``COMPRESS_ZSTD`` column compression flag. [GitHub#133]
  [Patch by Naoya Murakami]

* [:doc:`/reference/server_variables`] Added documentation about
  :ref:`server-variable-mroonga-libgroonga-support-zstd`
  variable. [GitHub#133] [Patch by Naoya Murakaimi]

* [:doc:`/install`] Changed to recommend ``https://packages.groonga.org``
  for downloading resources.

Fixes
^^^^^

* [:doc:`/tutorial/storage`] Fixed update error for log-bin and
  ``UPDATE`` with collated ``PRIMARY KEY``. [GitHub#132] [Reported by
  kitora]

* Fixed a bug that ``FOREIGN KEY`` is dumped unexpectedly even though
  you didn't set it. [groonga-dev,04276] [Reported by murata satoshi]

Thanks
^^^^^^

* kitora
* murata satoshi
* Naoya Murakami

.. _release-7-00:

Release 7.00 - 2017-02-09
-------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/install/centos`] Changed to ensure enabling EPEL to install
  package.

* Supported ``FOREIGN KEY`` constrain on ``UPDATE`` and ``DELETE``
  parent row. In the previous versions, only ``FOREIGN KEY`` constrain
  on ``INSERT`` is supported.

* [:doc:`/tutorial/storage`] Supported updating row even though its
  table has primary key with ROW binlog format. In the previous
  version, it causes update statement error. [GitHub#130] [Reported by
  kitora]

Thanks
^^^^^^

* kitora

Past releases
-------------

.. toctree::
   :maxdepth: 2

   news/6.x
   news/5.x
   news/4.x
   news/3.x
   news/2.x
   news/1.x
   news/0.x
