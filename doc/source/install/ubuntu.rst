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

  * 12.04 LTS Precise Pangolin
  * 14.04 LTS Trusty Tahr
  * 15.10 Wily Werewolf
  * 16.04 Xenial Xerus

Enable the universe repository and the security update repository to
install Mroonga::

  % sudo apt-get install -y -V software-properties-common lsb-release
  % sudo add-apt-repository -y universe
  % sudo add-apt-repository "deb http://security.ubuntu.com/ubuntu $(lsb_release --short --codename)-security main restricted"

Add the ``ppa:groonga/ppa`` PPA to your system::

  % sudo add-apt-repository -y ppa:groonga/ppa
  % sudo apt-get update

Install::

  % sudo apt-get install -y -V mysql-server-mroonga

If you want to use `MeCab <http://mecab.sourceforge.net/>`_ as a tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo apt-get install -y -V groonga-tokenizer-mecab
