Ubuntu
======

This section describes how to install Mroonga related deb packages on
Ubuntu. You can install them by ``apt``.

.. note::

   The PPA (Personal Package Archive, ``ppa:groonga/ppa``) is deprecated.
   Use the Groonga APT repository (``packages.groonga.org``) described below instead.

Here are supported Ubuntu versions:

* 22.04 Jammy Jellyfish
* 24.04 Noble Numbat

Here are Ubuntu versions that supports MySQL:

* 22.04 Jammy Jellyfish
* 24.04 Noble Numbat

Here are Ubuntu versions that supports MariaDB:

* 22.04 Jammy Jellyfish
* 24.04 Noble Numbat

Groonga APT Repository
------------------------

Enable the universe repository and the security update repository to
install Mroonga::

  $ sudo apt install -y -V software-properties-common lsb-release
  $ sudo add-apt-repository -y universe
  $ sudo add-apt-repository -y "deb http://security.ubuntu.com/ubuntu $(lsb_release --short --codename)-security main restricted"

Add the Groonga APT repository to your system::

  $ sudo apt update
  $ sudo apt install -y -V ca-certificates wget
  $ wget https://packages.groonga.org/ubuntu/groonga-apt-source-latest-$(lsb_release --codename --short).deb
  $ sudo apt install -y -V ./groonga-apt-source-latest-$(lsb_release --codename --short).deb
  $ rm -f groonga-apt-source-latest-$(lsb_release --codename --short).deb
  $ sudo apt update

Install Mroonga for MySQL::

  $ sudo apt install -y -V mysql-mroonga

Install Mroonga for MariaDB::

  $ sudo apt install -y -V mariadb-mroonga

If you want to use `MeCab <https://taku910.github.io/mecab/>`_ as a tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  $ sudo apt install -y -V groonga-tokenizer-mecab
