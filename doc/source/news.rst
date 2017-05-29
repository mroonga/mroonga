:orphan:

.. highlightlang:: none

News
====

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
  ``mroonga_libgroonga_support_zstd`` variable. [GitHub#133]
  [Patch by Naoya Murakaimi]

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
