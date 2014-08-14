.. highlightlang:: none

OS X
====

This section describes how to install Mroonga on OS X. You can install
Mroonga by `Homebrew <http://mxcl.github.com/homebrew/>`_.

.. _homebrew:

Homebrew
--------

Install::

  % brew install https://raw.github.com/mroonga/homebrew/master/mroonga.rb --use-homebrew-mysql

If you want to use `MeCab <http://mecab.sourceforge.net/>`_ as a
tokenizer, install with ``--with-mecab`` option.

Install with MeCab support::

  % brew install https://raw.github.com/mroonga/homebrew/master/mroonga.rb --use-homebrew-mysql --with-mecab

.. seealso:: `mroonga/homebrew on GitHub <https://github.com/mroonga/homebrew>`_
