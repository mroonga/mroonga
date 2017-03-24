:orphan:

.. highlightlang:: none

News
====

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
