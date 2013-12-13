.. highlightlang:: none

Mode
====

Mroonga has the following two modes.

  * storage mode
  * wrapper mode

With the storage mode, we use groonga for both of the full text search function and the data storage.
Since all functions of storage engine are realised with groonga, aggregations are fast, that is one of groonga's advantages, and you can manage the database directly by ``groonga`` command.

The structure of the storage mode is the following. You use it instead of existing storage engines like MyISAM or InnoDB

.. figure:: /images/storage-mode.png
   :alt: storage mode
   :align: center

With the wrapper mode, groonga is used for full text search function only, and another existing storage engine like InnoDB is used for storing data.
By using wrapper mode, you combine InnoDB that is well-use as the storage engine and mroonga that is a proven full text search engine, and you can use it as the stable database having the fast full text search function.

The structure of the wrapper mode is the following. Full text search related operations are done by mroonga, and other operations are done by existing storage engines like MyISAM, InnoDB etc.
Mroonga is located between SQL Handler that processes SQL and an existing storage engine, thus all data goes through mroonga
With this way, full text search indexing etc. are done transparently.

.. figure:: /images/wrapper-mode.png
   :alt: wrapper mode
   :align: center
