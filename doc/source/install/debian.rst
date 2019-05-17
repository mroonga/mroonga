.. highlightlang:: none

Debian GNU/Linux
================

This section describes how to install Mroonga related deb packages on
Debian GNU/Linux. You can install them by ``apt``.

.. include:: 32bit-note.inc

stretch (MariaDB)
-----------------

Install::

  % sudo apt update
  % sudo apt install -y -V apt-transport-https
  % sudo wget -O /usr/share/keyrings/groonga-archive-keyring.gpg https://packages.groonga.org/debian/groonga-archive-keyring.gpg
  % sudo tee /etc/apt/sources.list.d/groonga.list <<APT_LINE
  deb [signed-by=/usr/share/keyrings/groonga-archive-keyring.gpg] https://packages.groonga.org/debian/ stretch main                        
  deb-src [signed-by=/usr/share/keyrings/groonga-archive-keyring.gpg] https://packages.groonga.org/debian/ stretch main                    
  APT_LINE
  % sudo apt update
  % sudo apt install -y -V mariadb-server-10.1-mroonga

If you want to use `MeCab <http://mecab.sourceforge.net/>`_ as a tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo apt install -y -V groonga-tokenizer-mecab

