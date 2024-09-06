Release procedure
=================

Requirements
------------

Here is the requirements about release procedure.

* Debian GNU/Linux (sid)
* Use zsh as user shell

Use the following working directories.

* MROONGA_SOURCE_DIR=$HOME/work/mroonga
* MROONGA_BUILD_DIR=$HOME/work/build-dir/mroonga
* MROONGA_ORG_DIR=$HOME/work/mroonga.org
* MARIADB_SOURCE_DIR=$HOME/work/mariadb
* MARIADB_BUILD_DIR=$HOME/work/build-dir/mariadb
* GROONGA_SOURCE_DIR=$HOME/work/groonga
* GROONGA_BUILD_DIR=$HOME/work/build-dir/groonga
* INSTALL_DIR=/tmp/local

Setup build environment
-----------------------

Install the following packages::

    % sudo apt-get install -V ruby mecab libmecab-dev gnupg2 dh-autoreconf python-sphinx bison
    % pip3 install -U sphinx myst-parser linkify-it-py

Describe the changes
--------------------

Summarize recent changes since the previous release into ``doc/source/news/xx.md``.

Should be included

* The changes which affect to users
* The changes which broke compatibility

Shoud not be included

* The changes which doesn't affect to users (Internal source code changes or refactoring)

Execute the following command to create HTML for news::

    % cd ${MROONGA_SOURCE_DIR}
    % cmake -S . -B ${MARIADB_BUILD_DIR} -GNinja -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} -DPLUGIN_CASSANDRA=NO
    % cmake --build ${MARIADB_BUILD_DIR}
    % cmake --install ${MARIADB_BUILD_DIR}
    % cd ..
    % cd ${GROONGA_SOURCE_DIRECTORY}
    % cmake -S . -B ${GROONGA_BUILD_DIR} --preset=release-default --fresh -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}"
    % cmake --build ${GROONGA_BUILD_DIR}
    % cmake --install ${GROONGA_BUILD_DIR}
    % cd ..
    % cd ${MROONGA_SOURCE_DIRECTORY}
    % PKG_CONFIG_PATH=${INSTALL_DIR}/lib/pkgconfig cmake -S . -B ${MROONGA_BUILD_DIR} --fresh --preset=doc -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" -DMYSQL_SOURCE_DIR=~${MARIADB_SOURCE_DIR} -DMYSQL_BUILD_DIR=${MARIADB_BUILD_DIR} -DMYSQL_CONFIG=${INSTALL_DIR}/bin/mariadb_config
    % cmake --build ${MROONGA_BUILD_DIR}

Check whether you can upload packages
-------------------------------------

Check whether you can login to packages.groonga.org as packages user.

You can check with the following command whether you can login::

    % ssh packages@packages.groonga.org

If you can't login to packages.groonga.org, you must be registered ssh public key.

Execute make update-latest-release
----------------------------------

Execute ``rake release:version:update`` command with OLD_RELEASE_DATE, NEW_RELEASE_DATE.

When 14.07 release, we executed the following command::

    % rake release:version:update OLD_RELEASE=14.04 OLD_RELEASE_DATE=2024-06-12 NEW_RELEASE_DATE=2024-09-06

This command updates the version of spec file or debian/changelog entry.

Confirm the results of each test
--------------------------------

We confirm the results of all the below tests and build before setting the tag to Mroonga.
Because if we will find problems in Mroonga after setting the tag to it, we must release it again.

* `GitHub Actions <https://github.com/mroonga/mroonga/actions>`_
* `Launchpad <https://launchpad.net/~groonga/+archive/ubuntu/nightly/+packages>`_

How to build packages for Ubuntu on Nightly::

    Download source archive from GitHub actions.
    % mv mroonga-14.07.tar.gz mroonga/
    % cd mroonga/packages
    % rake ubuntu DPUT_CONFIGURATION_NAME=groonga-ppa-nightly DPUT_INCOMING="~groonga/ubuntu/nightly" LAUNCHPAD_UPLOADER_PGP_KEY=xxxxxxx

Tagging for release
-------------------

Execute the following command for tagging::

    % rake release:tag

Upload archive files
--------------------

Execute the following command for uploading source archive::

    % cd packages/source
    % rake source

As a result, ``tar.gz`` archive file is available from https://packages.groonga.org/source/mroonga/.

Create packages for the release
-------------------------------

Create Linux and Windows packages.

Debian
^^^^^^

Change working directory to ``packages``::

    % cd packages

Execute the following command::

    % rake apt

Now we finish build and upload packages to https://packages.groonga.org/.
However, these packages are unsigned. We sign packages by executing the below commands::

    % cd $PACKAGES_GROONGA_ORG_REPOSITORY
    % rake apt

Debian derivatives(Ubuntu)
^^^^^^^^^^^^^^^^^^^^^^^^^^

For Ubuntu, packages are provided by PPA on launchpad.net.

Change working directory to ``packages`` and execute ``rake ubuntu:upload`` command::

    % cd packages
    % rake ubuntu

When upload packages was succeeded, package build process is executed on launchpad.net. Then build result is notified via E-mail.
You can install packages via Groonga PPA on launchpad.net::

  https://launchpad.net/~groonga/+archive/ubuntu/ppa

Red Hat derivatives
^^^^^^^^^^^^^^^^^^^

Change working directory to ``packages`` ::

    % cd packages

Execute the following command::

    % rake yum

Now we finish build and upload packages to https://packages.groonga.org/.
However, these packages are unsigned. We sign packages by executing the below commands::

    % cd $PACKAGES_GROONGA_ORG_REPOSITORY
    % rake yum

Windows
^^^^^^^

For windows packages, we use artifacts of `GitHub release page <https://github.com/mroonga/mroonga/releases>`_ .

Update Docker images
--------------------

TODO

Upload documents
----------------

Execute the following command::

    % rake release:document:update BUILD_DIR=${MROONGA_BUILD_DIR} MROONGA_ORG_DIR=${MROONGA_ORG_DIR}

Commit changes in mroonga.org repository && push them.

Update blog(Mroonga blog)
-------------------------

We update the below files.

* ``$MROONGA_GITHUB_COM_PATH/ja/_posts/(the date of release)-mroonga-(version).md``
* ``$MROONGA_GITHUB_COM_PATH/en/_posts/(the date of release)-mroonga-(version).md``

We can confirm contents of blog on Web browser by using Jekyll.::

  % jekyll serve

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

Bump version
------------

Bump version to the latest release::

    % rake dev:version:bump NEW_VERSION=xx.xx
