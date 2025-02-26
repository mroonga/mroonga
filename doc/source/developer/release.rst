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

    $ sudo apt-get install -V ruby mecab libmecab-dev gnupg2 dh-autoreconf bison

Describe the changes
--------------------

Summarize recent changes since the previous release into ``doc/source/news/xx.md``.

Should be included

* The changes which affect to users
* The changes which broke compatibility

Shoud not be included

* The changes which doesn't affect to users (Internal source code changes or refactoring)

Execute the following command to create HTML for news::

    $ cmake -S ${MARIADB_SOURCE_DIR} -B ${MARIADB_BUILD_DIR} -GNinja -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} -DPLUGIN_CASSANDRA=NO
    $ cmake --build ${MARIADB_BUILD_DIR}
    $ cmake --install ${MARIADB_BUILD_DIR}
    $ cmake -S ${GROONGA_SOURCE_DIRECTORY} -B ${GROONGA_BUILD_DIR} --preset=release-default --fresh -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}"
    $ cmake --build ${GROONGA_BUILD_DIR}
    $ cmake --install ${GROONGA_BUILD_DIR}
    $ PKG_CONFIG_PATH=${INSTALL_DIR}/lib/pkgconfig cmake -S ${MROONGA_SOURCE_DIRECTORY} -B ${MROONGA_BUILD_DIR} --fresh --preset=doc -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" -DMYSQL_SOURCE_DIR=~${MARIADB_SOURCE_DIR} -DMYSQL_BUILD_DIR=${MARIADB_BUILD_DIR} -DMYSQL_CONFIG=${INSTALL_DIR}/bin/mariadb_config
    $ cmake --build ${MROONGA_BUILD_DIR}

Execute ``release`` task
------------------------

Execute ``rake release``::

    $ rake release

About ``release`` task
^^^^^^^^^^^^^^^^^^^^^^

``release`` task executes the three tasks.

1. ``release:version:update``

   * Append the changelog of the new version to the spec file of the RPM package, and so on

   * You can also specify a release date with ``NEW_RELEASE_DATE``

2. ``release:tag``

   * Push the tag for release

   * This will start the automatic release

3. ``dev:version:bump``

   * We will bump up the version for the next release

   * You can also specify a version with ``NEW_VERSION``

Confirm CI
----------

We confirm whether below CIs green or not.

* `GitHub Actions <https://github.com/mroonga/mroonga/actions>`_

We use CI to do automatic releases, so if it fails, we retry.

Update Docker images
--------------------

Execute the following commands::

    $ git clone git@github.com:mroonga/docker.git mroonga-docker
    $ cd mroonga-docker
    $ ./update.sh
    $ git push
    $ git push --tags

Update documents
----------------

Execute the following commands::

    $ export GROONGA_ORG_REPOSITORY=${HOME}/work/groonga.org
    $ git clone git@github.com:groonga/groonga.org.git ${GROONGA_ORG_REPOSITORY}
    $ rake -C ${MROONGA_ORG_DIR} release:version:update

Announce release for X(Twitter)
-------------------------------

Click Tweet link in Mrooga blog entry. You can share tweet about latest release.
If you use tweet link, title of release announce and URL is embedded into your tweet.

Execute sharing tweet in Japanese and English version of blog entry.
Note that this tweet should be done when logged in by ``groonga`` account.

Announce release for Facebook
-----------------------------

We announce release from Mroonga group in Facebook.

https://www.facebook.com/mroonga/
