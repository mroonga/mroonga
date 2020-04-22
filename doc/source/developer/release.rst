.. highlightlang:: none

Release procedure
=================

Requirements
------------

Here is the requirements about release procedure.

* Debian GNU/Linux (sid)
* Use zsh as user shell

Use the following working directories.

* MROONGA_DIR=$HOME/work/mroonga
* MROONGA_CLONE_DIR=$HOME/work/mroonga/mroonga.clean
* MROONGA_GITHUB_COM_PATH=$HOME/work/mroonga/mroonga.github.com
* CUTTER_DIR=$HOME/work/cutter
* CUTTER_SOURCE_PATH=$HOME/work/cutter/cutter
* GROONGA_SOURCE_PATH=$HOME/work/groonga/groonga.clean
* PACKAGES_GROONGA_ORG_REPOSITORY=$HOME/work/groonga/packages.groonga.org

Setup build environment
-----------------------

Install the following packages::

    % sudo apt-get install -V ruby mecab libmecab-dev gnupg2 dh-autoreconf python-sphinx bison

We use `Docker <https://www.docker.com/>`_ for building deb or rpm packages.
If it is not installed, please install it according to the official documentation of Docker.

Bump version
------------

Bump version to the latest release::

    % make update-version NEW_VERSION_MAJOR=9 NEW_VERSION_MINOR=1 NEW_VERSION_MICRO=0

Describe the changes
--------------------

Summarize recent changes since the previous release into ``doc/source/news.txt``.
This summary is also used in release note.

Execute the following command to collect change logs since the previous version::

   % git log -p --reverse $(git tag | tail -1)..

Search the logs by ``^commit``, and pick up collect remarkable changes.

Should be included

* The changes which affect to users
* The changes which broke compatibility

Shoud not be included

* The changes which doesn't affect to users (Internal source code changes or refactoring)


Generate configure script
-------------------------

Because of master branch doesn't include configure script, then it is required to generate configure script for building.

Execute ``autogen.sh`` with the following command::

    % sh autogen.sh

It generates ``configure`` script.

Execute configure script
------------------------

For generating ``Makefile``, execute the ``configure`` script.

Execute with the following options for the release::

    % ./configure \
        --enable-document \
        --prefix=/tmp/local \
        --with-launchpad-uploader-pgp-key=(The key ID that is registered Launchpad) \
        --with-mroonga-github-com-path=$MROONGA_GITHUB_COM_PATH \
        --with-cutter-source-path=$CUTTER_SOURCE_PATH \
        --with-groonga-source-path=$GROONGA_SOURCE_PATH \
        --with-mysql-source=(The directory of MySQL source code) \
        --with-mysql-build=(The build directory of MySQL) \
        --with-mysql-config=(The path to mysql_config command)

See :doc:`/install/others`  for the details of ``--with-mysql-source`` option.

.. note::
   We can not upload and override the same name packages to PPA repository.
   Therefore, we upload packages to a stable repository after confirming the successful build of they in the nightly repository.

Check whether you can upload packages
-------------------------------------

Check whether you can login to packages.groonga.org as packages user.

You can check with the following command whether you can login::

    % ssh packages@packages.groonga.org

If you can't login to packages.groonga.org, you must be registered ssh public key.

Confirm the results of each test
------------------------------

We confirm the results of all the below tests before setting the tag to Mroonga.
Because if we will find problems in Mroonga after setting the tag to it, we must release it again.

* `GitHub Actions <https://github.com/mroonga/mroonga/actions>`_
* `Travis CI <https://travis-ci.org/github/mroonga/mroonga>`_
* `AppVeyor <https://ci.appveyor.com/project/groonga/mroonga>`_

Execute make update-latest-release
----------------------------------

Execute ``make update-latest-release`` command with OLD_RELEASE_DATE, NEW_RELEASE_DATE.

When 9.09 release, we executed the following command::

    % make update-latest-release OLD_RELEASE=9.09 OLD_RELEASE_DATE=2019-09-27 NEW_RELEASE_DATE=2019-10-30

This command updates some html files (which is used for web sites of Mroonga - index.html,ja/index.html) and the version of spec file or debian/changelog entry.

Tagging for release
-------------------

Execute the following command for tagging::

    % make tag
    % git push --tags origin

.. note::
   After tagging for the release, execute ``configure`` script. This tag information is reflected when generating the documents.

Upload archive files
--------------------

Then, create archive file (``tar.gz``) for distribution::

    % make dist

Change working directory to ``packages/source``::

    % cd packages/source

Execute ``make download`` for syncing with the upstream::

    % make download

Execute ``make archive`` for generating source archive::

    % make archive

Execute ``make upload`` for uploading archive file::

    % make upload

As a result, ``tar.gz`` archive file is available from http://packages.groonga.org/source/mroonga/.


Create packages for the release
-------------------------------

Create Linux and Windows packages.

Debian
^^^^^^

Change working directory to ``packages``::

    % cd packages

Execute the following command::

    % rake apt:release

Now we finish build and upload packages to https://packages.groonga.org/.
However, these packages are unsigned. We sign packages by executing the below commands::

    % cd $PACKAGES_GROONGA_ORG_REPOSITORY
    % rake apt

Debian derivatives(Ubuntu)
^^^^^^^^^^^^^^^^^^^^^^^^^^

For Ubuntu, packages are provided by PPA on launchpad.net.

Change working directory to ``packages`` and execute ``rake ubuntu:upload`` command::

    % cd packages
    % rake ubuntu:upload

When upload packages was succeeded, package build process is executed on launchpad.net. Then build result is notified via E-mail.
You can install packages via Groonga PPA on launchpad.net::

* https://launchpad.net/~groonga/+archive/ubuntu/ppa

Red Hat derivatives
^^^^^^^^^^^^^^^^^^^

Change working directory to ``packages`` ::

    % cd packages

Execute the following command::

    % rake yum:release

Now we finish build and upload packages to https://packages.groonga.org/.
However, these packages are unsigned. We sign packages by executing the below commands::

    % cd $PACKAGES_GROONGA_ORG_REPOSITORY
    % rake yum

Windows
^^^^^^^

For windows packages, we use `AppVeyor CI <https://ci.appveyor.com/project/groonga/mroonga>`_ artifacts files.
We upload them to `GitHub release page <https://github.com/mroonga/mroonga/releases>`_ .

Upload documents
----------------

1. Clone mroonga.github.com repository
2. Execute ``make update-document`` in ``$MROONGA_CLONE_DIR``
3. Commit changes in mroonga.github.com repository && push them

Update blog(Mroonga blog)
-------------------------

We update the below files.

* $MROONGA_GITHUB_COM_PATH/ja/_posts/(the date of release)-mroonga-(version).md
* $MROONGA_GITHUB_COM_PATH/en/_posts/(the date of release)-mroonga-(version).md

We can confirm contents of blog on Web browser by using Jekyll.::

  % jekyll serve --watch

We access http://localhost:4000 on our web browser for confirming contents.

.. note::
   If we want private to blog contents, we set ``false`` on ``published:`` in ``.md`` file.::

     ---
     layout: post.en
     title: Mroonga 10.01 has been released!
     description: Mroonga 10.01 has been released!
     published: false
     ---

Announce release for mailing list
---------------------------------

Send release announce for each mailing list

* ml@mysql.gr.jp for Japanese
* groonga-dev@lists.osdn.me for Japanese
* groonga-talk@lists.sourceforge.net for English

Announce release for twitter
----------------------------

Click Tweet link in Mrooga blog entry. You can share tweet about latest release.
If you use tweet link, title of release announce and URL is embedded into your tweet.

Execute sharing tweet in Japanese and English version of blog entry.
Note that this tweet should be done when logged in by ``groonga`` account.

Announce release for Facebook
-----------------------------

We announce release from Mroonga group in Facebook.

https://www.facebook.com/mroonga/
