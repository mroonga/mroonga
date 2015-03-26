:orphan:

.. highlightlang:: none

News
====

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
