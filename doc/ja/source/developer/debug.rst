.. highlightlang:: none

デバッグ方法
============

デバッグ用ビルド方法
--------------------

デバッグ用にビルドすることにより、gdb上でのシンボル解決など開発時に得られる情報が多くなります。そのため、開発時はデバッグ用にMySQLとgroongaストレージエンジンをビルドします。

.. note::

   片方だけデバッグ用ビルドにすると構造体のサイズなどが異なってしまうため、groongaストレージエンジンがロードできなかったり、実行時にassertに引っかかったりしてうまく動作しません。

MySQLのデバッグ用ビルド方法
^^^^^^^^^^^^^^^^^^^^^^^^^^^

`MySQL :: MySQL 5.5 Reference Manual :: 2.9.2 Installing MySQL from a Standard Source Distribution`_ にある通り、CMakeのオプションに ``-DWITH_DEBUG=yes`` オプションを渡すことでデバッグ用にビルドすることができます。

ダウンロードからビルドまでの流れは以下の通りです。::

  % mkdir -p ~/work/
  % cd ~/work/
  % wget http://ftp.jaist.ac.jp/pub/mysql/Downloads/MySQL-5.5/mysql-5.5.13.tar.gz
  % tar xvzf mysql-5.5.13.tar.gz
  % cd mysql-5.5.13
  % cmake . -DCMAKE_INSTALL_PREFIX=/tmp/local -DWITH_DEBUG=yes
  % make

.. _`MySQL :: MySQL 5.5 Reference Manual :: 2.9.2 Installing MySQL from a Standard Source Distribution`: http://dev.mysql.com/doc/refman/5.5/en/installing-source-distribution.html

groongaストレージエンジンのデバッグ用ビルド方法
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

groongaストレージエンジンはconfigureのオプションに ``--with-debug`` を渡すことでデバッグ用にビルドすることができます。

リポジトリのcloneからビルドまでの流れは以下の通りです。::

  % cd ~/work/
  % git clone git@github.com:mroonga/mroonga.git
  % cd mroonga
  % ./autogen.sh
  % ./configure CFLAGS="-ggdb3 -O0" CXXFLAGS="-ggdb3 -O0" --with-debug --prefix=/tmp/local --with-mysql-source=$HOME/work/mysql-5.5.13 --with-mysql-config=$HOME/work/mysql-5.5.13/scripts/mysql_config
  % make

無事にビルドができたら以下のようにテストを実行してください。すべてのテストが ``[pass]`` になればデバッグ用ビルドは成功しています。::

  % test/run-sql-test.sh

run-sql-test.sh を使いこなす
----------------------------

run-sql-test.sh はデバッグの友。ここでは、その便利な使い方の一例をご紹介します。

指定したテストを実行する
^^^^^^^^^^^^^^^^^^^^^^^^

何もオプションを渡さずに run-sql-test.sh を実行すると ``test/sql/t/`` 以下にある全てのテスト (``*.test``) が実行されてしまいます。

特定のテストだけを実行したい、という場合は次のようにしてテスト名を --do-test オプションに渡します。 ::

  ./test/run-sql-test.sh --do-test=foobar

トレースを見る
^^^^^^^^^^^^^^

次のようにして ``--debug`` オプションをつけてテストを実行すると、関数の呼び出しなどが記録されます。この呼び出しは ``${MySQLの作業ディレクトリ}/${MySQLのバージョン}/mysql-test/var/log/mysqld.1.trace`` に格納されます。

新しく関数を作成した場合は MRN_DBUG_ENTER_FUNCTION 関数の先頭に配置し、関数の呼び出しを記録するようにすると良いでしょう。

GDB を立ち上げる
^^^^^^^^^^^^^^^^

``--gdb`` オプションを指定することで、テストを実行する際に GDB を用いてデバッグを行うことができます。 ::

  ./test/run-sql-test.sh --gdb
