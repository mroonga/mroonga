:orphan:

.. highlightlang:: none

News
====

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
