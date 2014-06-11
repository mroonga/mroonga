.. highlightlang:: none

Developer's guide
=================

.. toctree::
   :maxdepth: 2

   developer/debug
   developer/release

How we are developing
---------------------

The Mroonga project uses Redmine for Ticket Driven Development.

The location of our Redmine site is the following.

http://redmine.groonga.org/projects/mroonga

Developments are done per ticket.

We would like to ask developers to register on the site above.

Roadmap
--------------------

We develop based on the roadmap, and you can see our roadmap in the following page.

http://redmine.groonga.org/projects/mroonga/roadmap

We draw up our roadmap by discussing in off-line or on-line meetings.

The roadmap specifies the list of adding features in each version.

Source code management
----------------------

We manage the source code using Git on Github.

http://github.com/mroonga/mroonga

If you want to contribute, please make clone in read/write mode, and then commit and push.

If you still do not have your account on github.com, please register.

Development process
----------------------

We develop Mroonga using the following procedures.

1. Create a new ticket and add a description
2. Discuss in the development meeting and update roadmap
3. Assign the ticket, and the assignee starts development
4. Implement features or fix bugs, then push to the repository if needed
5. Add and run test codes, then push to the repository
6. Review the implementation and tests, and go back to 4. if needed
7. Add or update documents, then push to the repository
8. Close the ticket

We welcome your ideas about new features or changes of specifications.
Please create a ticket first and describe your idea there.

For adding and running test codes, please refer the description below.

We want to ask the assignee to handle whole the process, like design, implementation, test and documentations.

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
* glibc
* `MySQL <http://www.mysql.com/>`_
* `Groonga <http://groonga.org/>`_
* `Cutter <http://cutter.sourceforge.net/>`_ (for C/C++ unit tests)
* `Sphinx <http://sphinx-doc.org/>`_ (for documents)

Contents of the source tree
---------------------------

There are just a few source files for now, and we would like to keep it simple as possible.

ha_mroonga.hpp
 The header file of Mroonga.

ha_mroonga.cpp
 The implementation of Mroonga.

mrnsys.hpp
 The header file of utility functions

mrnsys.cpp
 The implementation of utility functions

mysql-test/
 The directory for tests by SQL

 SQL test scripts are included in each test case 't/' directory, that are also the definition of currently available SQL statements.

 The expected results of SQL tests are included in each test case 'r/' directory , that are also the definition of the current specification of supported SQL.

test/unit/
 The directory for per-function unit tests by C/C++

doc/source/
 English documents in Sphinx format

Since we are still in the early stage of the development of Mroonga, we will not make the documents of the specification of SQL queries for now.

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

For the detail about C/C++ unit tests, please refer the following Cutter document.

http://cutter.sourceforge.net/

Adding and updating documents
-----------------------------

We use Sphinx for the documentation of Mroonga.

Sphinx is a documentation generator which uses reStructuredText as its markup language, and converts reStructuredText files into HTML files.

reStructuredText uses .rst filename extensions.

We write documents in the reStructuredText format and convert them to HTML.

The source files of documents have .rst extension in the "doc/source" directory of the Mroonga repository.

When you add or update them, please try "make html" to confirm that there are no syntax errors.

Documents are published in http://mroonga.org. Since we are using GitHub Project Pages, the web site is updated when HTML files are pushed to the http://github.com/mroonga/mroonga.github.com repository.

We push added or updated HTML files to the repository after confirming the consistency between the documents and the current release version.

So you can just push to Mroonga repository to push documents for each ticket.

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

See reStructuredText (reST) `concepts and syntax <http://sphinx-doc.org/latest/rest.html>`_ about how to write a document in reStructuredText format.

Confirm generated document
^^^^^^^^^^^^^^^^^^^^^^^^^^

You can generate HTML files by the following command. ::

  % cd doc/locale/en
  % make html

You can confirm generated HTML document in web browser, after generating HTML files.

Here we show an example of using Firefox. ::

  % firefox html/characteristic.html
