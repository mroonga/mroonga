:orphan:

.. highlightlang:: none

News
====

.. _release-6-10:

Release 6.10 - 2016-10-29
-------------------------

Improvements
^^^^^^^^^^^^

* [deb][ubuntu] Supported MariaDB 10.x package for Ubuntu 16.04

* [deb][ubuntu] Supported Ubuntu 16.10 (Yakkety Yak)

* [:doc:`/install/ubuntu`] Added document how to install Mroonga for
  MariaDB on Ubuntu 16.04.

* [:doc:`/install`][:doc:`/tutorial`] Improved sample SQL format for
  easy to copy and paste.
  
* [rpm][centos] Supported latest MySQL 5.6.34, MySQL 5.7.16, Percona
  Server 5.6.33 and Percona Server 5.7.15. [groonga-dev,04169]
  [Reported by Hiroshi Kagami]

Fixes
^^^^^

* [mysql57][wrapper] Fixed a bug that ``COUNT(*)`` with InnoDB doesn't
  return correct number of records because behavior of InnoDB was
  changed since MySQL 5.7.  [Reported by koyama_wataru_7]

Thanks
^^^^^^

* Hiroshi Kagami
* koyama_wataru_7

.. _release-6-09:

Release 6.09 - 2016-09-29
-------------------------

Improvements
^^^^^^^^^^^^

* [storage] Supported nonexistent reference insert check for ``FOREIGN
  KEY``. [Gitter:https://gitter.im/groonga/ja?at=57d629477b9f8167113efb04]
  [Reported by mizutamazukki]

Thanks
^^^^^^

* mizutamazukki

.. _release-6-08:

Release 6.08 - 2016-08-29
-------------------------

Improvements
^^^^^^^^^^^^

* [rpm][centos] Supported latest MariaDB 5.5.50 on CentOS 7.

* [rpm][centos] Supported Percona Server 5.6/5.7 on CentOS 6/7.

* [:doc:`/reference/limitations`] Updated description about table
  limitations. There is a good news that the limitation about the
  maximum number of records are relaxed. In the past, it is described
  as 268,435,455 records, but it turns out that actually you can store
  more records.

* Supported MariaDB 10.2.1

Fixes
^^^^^

* Fixed compile error with GCC 6.1.1.

.. _release-6-07:

Release 6.07 - 2016-08-04
-------------------------

Improvements
^^^^^^^^^^^^

* [storage mysql57] Supported ``COUNT(*)`` skip optimization for
  ``BETWEEN`` again.

* [:doc:`/install/ubuntu`] Dropped Ubuntu 15.10 (Wily Werewolf)
  support. It had been reached to End of Life on July 28, 2016.

* [windows] Updated bundled MariaDB to 10.1.16.

Fixes
^^^^^

* [mariadb55] Fixed to disable ``COUNT(*)`` skip optimization when it
  can't be applied correctly. For example, ``SELECT COUNT(*) FROM
  users WHERE age = 29;`` is such a case on MariaDB 5.5.

.. _release-6-06:

Release 6.06 - 2016-06-30
-------------------------

Fixes
^^^^^

* [mariadb10] Fixed a crash bug on MariaDB 10.x.
  [Gitter:groonga/ja:57746049265214c130a655ed][Reported by yoyoshifgs]

Thanks
^^^^^^

* yoyoshifgs

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

Past releases
-------------

.. toctree::
   :maxdepth: 2

   news/5.x
   news/4.x
   news/3.x
   news/2.x
   news/1.x
   news/0.x
