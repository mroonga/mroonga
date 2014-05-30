.. highlightlang:: none

Developer's guide
=================

.. toctree::
   :maxdepth: 2

   developer/debug
   developer/release

How we are developing
---------------------

We, the Mroonga development project, are doing Ticket Driven Development by using a BTS named Redmine.

The location of our Redmine site is the following.

http://redmine.groonga.org/projects/show/mroonga

Developments are done per ticket.

We would like to ask developers to register on the site above.

Roadmaps
--------------------

You can see our roadmaps in the following page. Generally speaking, we develop based on the roadmap.

http://redmine.groonga.org/projects/mroonga/roadmap

We decide our roadmaps by discussing in off-line or on-line meetings.

Roadmaps specify the list of adding features in each version.

Source code management
----------------------

We manage the source code using git on github.

http://github.com/mroonga/mroonga

If you want to contribute, please make clone in read/write mode, and then commit and push please.

If you still do not have your account on github.com, please register.

Process of development
----------------------

We would like to develop in the following procedure.

1. Create a new ticket and add a description
2. Discuss in the development meeting and update roadmaps
3. Assign the ticket, and the assignee starts development
4. Implement features or fix bugs, then push to the repository if needed
5. Add and run test codes, then push to the repository
6. Review the implementation and tests, and go back to 4. if needed
7. Add or update documents, then push to the repository
8. Close the ticket

We welcome your ideas about new features or changes of specifications.
Please create a ticket first and describe your idea there.

For adding and running test codes, please refer the description below.

Generally speaking, we want to ask the assignee to handle whole the process, like design, implementation, test and documentations.

Then you ask other developers to review (especially in case of adding new features), and when it passes you add or update documents and the ticket can be closed.

For adding and updating documents, please refer the description below.

Release
-------

We release the software when all tickets of the roadmap are closed.

We do the following to release.

* Create source packages
* Create binary packages
* Update documents on http://mroonga.org
* Announce the release

(This section will be moved to :doc:`developer/release`).

Development environment
-----------------------

Currently we are assuming the following development environments.

* Linux x86_64
* glibc 2.5
* MySQL 5.5
* groonga 1.2.8
* Cutter 1.1 (for C/C++ unit tests)
* Sphinx 1.1 (for documents)

glibc 2.5 is used in Red Hat Enterprise Linux 5.

Contents of the source tree
---------------------------

There are just a few source files for now, and we would like to keep it simple as possible.

ha_mroonga.h
 The header file of mroonga.

ha_mroonga.cc
 The implementation of mroonga.

mrnsys.h
 The header file of utility functions

mrnsys.c
 The implementation of utility functions

test/sql/
 The directory for tests by SQL

test/sql/t/
 SQL test scripts, that are also the definition of currently available SQL statements

test/sql/r/
 The expected results of SQL tests, that are also the definition of the current specification of supported SQL

test/unit/
 The directory for per-function unit tests by C/C++

doc/en/
 English documents in Sphinx format

doc/ja/
 Japanese documents in Sphinx format

Since we are still in the early stage of the development of mroonga, we will not make the documents of the specification of SQL queries for now.

Alternatively we consider our SQL tests and its expected results as the list of features and the definition of their specifications.

Adding and running tests
------------------------

We use two kinds of regression tests to manage the quality of Mroonga.

SQL tests
 When you add features or fix bugs that can be confirmed by SQL queries, please always add SQL tests. You might think that performance improvements cannot be confirmed with SQL, but some can be still well tested by using status variables or information_schema plugin etc.

C/C++ unit tests
 They are function level regression tests using Cutter. When you add features that cannot differ SQL queries' results, like utility functions, please add tests here.

Before pushing to the repository, please always run the regression tests and confirm that you don't introduce any degradation.

You can invoke these two kinds of tests by "make check".

SQL tests are implemented as "sub test suite" for "mysql-test" in MySQL's regression tests. For the detail about how to add test cases or how to modify expected result files, please refer the following MySQL document.

http://dev.mysql.com/doc/mysqltest/2.0/en/index.html

For the detail about C/C++ unit tests, please refer the following cutter document.

http://cutter.sourceforge.net/

Adding and updating documents
-----------------------------

We use Sphinx for the documentation of Mroonga.

We write documents in ReStructuredText format and we convert them to HTML etc.

The source files of documents are files having .rst extension in "doc/source" directory.

When you add or modify them, please try to build by "make html" etc. and confirm that no syntax error happens.

Documents are published in http://mroonga.org . Since we are using github's web site feature, they will be updated each time when HTML files are pushed to http://github.com/mroonga/mroonga.github.com repository. We push to mroonga.github.com repository after confirming the consistency between documents and current released version.

So you can just push to mroonga repository to push documents for each ticket.

The details are as follows.

Installing Sphinx
^^^^^^^^^^^^^^^^^

You don't need to install Sphinx by yourself because Mroonga clones the latest Sphinx from Sphinx repository automatically. You just need to install Mercurial.

Build Mroonga
^^^^^^^^^^^^^

In order to enables documentation generation, you should run configure script with "--enable-document" option as follows. ::

  % ./configure --enable-document --with-mysql-source=(your mysql source directory)
  % make
  % make install

Adding a document
^^^^^^^^^^^^^^^^^

Write a document in reStructuredText format, and save the document with .rst extension in "doc/source" directory.

See reStructuredText (reST) `concepts and syntax <http://sphinx-doc.org/latest/rest.html>`_ about how to
write a document in ReStructuredText format.

Confirm generated document
^^^^^^^^^^^^^^^^^^^^^^^^^^

You can generate HTML files by the following command. ::

  % cd doc/locale/en
  % make html

You can confirm generated HTML document in web browser, after generating HTML files.

Here we show an example of using Firefox. ::

  % firefox html/characteristic.html

