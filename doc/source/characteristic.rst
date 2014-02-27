.. highlightlang:: none

The characteristics of Mroonga
==============================

What is Mroonga?
----------------

Mroonga is a MySQL storage engine based on Groonga, the full text search engine.

In MySQL 5.1 or later, Pluggable Storage Engine interface is introduced, and we can use custom storage engines easily. So we implement Mroonga, so that we can use Groonga through MySQL.

By using Mroonga, you can use Groonga with SQL.

The successor of Tritonn
------------------------

To support Japanese full text search, Tritonn was developed by embedding Senna, the predecessor of Groonga, in MySQL.
Mroogna is its successor.

Running as a MySQL plugin
-------------------------

Since Tritonn was the modified version of MySQL, we need to build it by ourselves or use binary files provided by Tritonn project, thus we cannot use the official binary files provided by MySQL.

On the other hand, Mroonga is an independent program (shared library) using Pluggable Storage Engine interface, and we can dynamically load it on MySQL's official binary.
So we can use it more easily than Tritonn.

Faster index update
-------------------

Comparing to Senna, Groonga has much better throughput in adding or updating index.

Mroonga also has the benefit of this performance improvement.

Faster search
-------------

In Tritonn, we use MyISAM storage engine, thus we have a exclusive table lock by updating data (and index), and it prevents the performance of search.

But in Mroonga, we no longer have this issue, and the performance of search is better especially in frequent data update cases.

Geolocation search
------------------

Groonga supports not only the full text search, but also the fast geolocation search using index.
And MySQL also has the syntax for geolocation search.
With Mroonga, you can use Groonga's fast geolocation search by using MySQL's geolocation SQL syntax.

Sharing the same Groonga storage
--------------------------------

Mroonga stores the data by using Groonga's DB API.
And its storage file's format is same as that of the file that is managed by Groonga itself only.
Therefore you can share the same Groonga storage like below.

* Store data through Mroonga (MySQL) and search from Groonga server.
* Store data through Groonga server and search from Mroonga (MySQL).

And Groonga's storage file can be shared with multi-processes and multi-threads, so that we can invoke several search queries to the same storage file simultaneously.

Associate with other storage engines
------------------------------------

Mroonga has two running modes.

One is "storage mode", that is the default mode, and we use Groonga for both storing data and searching.
With this mode, you can have full benefits of Groonga described above, like fast data update, lock-free full text search and geolocation search.
But it does not support transactions.

Another one is "wrapper mode", that adds full text search function on other storage engines like MyISAM or InnoDB.
With this mode, you can use Groonga's fast full text search with having the benefits of the storage engine, ex. transaction in InnoDB.
But you cannot have benefits from Groonga's read-lock free characteristic.
And you might have the performance bottle neck in the storage engine in updating data.

Supported platforms
-------------------

We currently support the following platforms.

* Linux x86_64 (Intel64/AMD64)
