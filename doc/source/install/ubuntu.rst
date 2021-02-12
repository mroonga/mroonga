.. highlightlang:: none

Ubuntu
======

This section describes how to install Mroonga related deb packages on
Ubuntu. You can install them by ``apt``.

.. include:: 32bit-note.inc

PPA (Personal Package Archive)
------------------------------

The Mroonga APT repository for Ubuntu uses PPA (Personal Package
Archive) on Launchpad. You can install Mroonga by APT from the PPA.

Here are supported Ubuntu versions:

  * 16.04 LTS Xenial Xerus
  * 18.04 Bionic Beaver
  * 20.04 Focal Fossa
  * 20.10 Groovy Gorilla

Here are Ubuntu versions that supports MySQL:

  * 16.04 LTS Xenial Xerus
  * 18.04 Bionic Beaver

Here are Ubuntu versions that supports MariaDB:

  * 20.04 Focal Fossa
  * 20.10 Groovy Gorilla

Enable the universe repository and the security update repository to
install Mroonga::

  % sudo apt-get install -y -V software-properties-common lsb-release
  % sudo add-apt-repository -y universe
  % sudo add-apt-repository "deb http://security.ubuntu.com/ubuntu $(lsb_release --short --codename)-security main restricted"

Add the ``ppa:groonga/ppa`` PPA to your system::

  % sudo add-apt-repository -y ppa:groonga/ppa
  % sudo apt-get update

Install Mroonga for MySQL::

  % sudo apt-get install -y -V mysql-server-mroonga

Install Mroonga for MariaDB::

  % sudo apt-get install -y -V mariadb-server-mroonga

If you want to use `MeCab <https://taku910.github.io/mecab/>`_ as a tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo apt-get install -y -V groonga-tokenizer-mecab
