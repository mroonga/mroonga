.. highlightlang:: none

Release procedure (XXX not yet translated)
==========================================

変更点の記述
------------

まず ``doc/ja/source/news.rst`` に変更点をまとめます。

次に、バージョンを確かめます。

例えば ``git tag`` の結果が次のようになっていたとき ::

 $ git tag
 v0.1
 v0.2
 v0.3
 v0.3a
 v0.4
 v0.5
 v0.6

次のようにすると、タグ ``v0.6`` 以降のコミットを閲覧できます。 ::

 $ git log -p --reverse v0.6..

配布用ファイルのアップロード
----------------------------

次に、配布用の ``tar.gz`` ファイルを作成します。 ::

 $ make dist

.. note::

   以下の作業は初回パッケージ作成時のみ必要です。初回パッケージ作成時にはパッケージ作成に必要なソフトウェアをインストールします。::

    $ sudo gem install github_api

作成された ``tar.gz`` ファイルのアップロードを行うには以下のコマンドを実行します。 ::

 $ make upload-to-github

これで、 https://github.com/mroonga/mroonga/downloads から ``tar.gz`` のダウンロードが行えるようになります。


パッケージの作成
----------------

Linux 用にパッケージを作成する必要があります。パッケージは以下の 2 種類に分けることが可能です。

1. Debian 系
2. Red Hat 系

.. note::

   現在のところ、パッケージの作成は Debian GNU/Linux (Ubuntu も可) でしか行えません。

Debian 系
^^^^^^^^^

.. note::

   以下の作業は初回パッケージ作成時のみ必要です。初回パッケージ作成時にはパッケージ作成に必要なソフトウェアをインストールします。::

    $ sudo apt-get install -y debootstrap

まず apt ディレクトリに移動します。 ::

 $ cd apt

その後、次のようにすれば一連のリリース作業（build update sign upload）が行われますが、途中で失敗することもあります。 ::

 $ make release

そのため head コマンドなどで Makefile.am の内容を確認し、順番に作業を行っていくほうが良いこともあります。 ::

 $ make build
 $ make update
 $ make sign
 $ make upload

make build に PARALLEL=yes とするとビルドが並列に走り、作業がより高速に行えます。

また make build CODES=lucid などとすると、ビルド対象を指定することができます。

このように Makefile.am を書き換えずにコマンドライン引数でビルドの挙動を変更する方法は、知っておいて損はないでしょう。

Red Hat 系
^^^^^^^^^^

.. note::

   以下の作業は初回パッケージ作成時のみ必要です。初回パッケージ作成時にはパッケージ作成に必要なソフトウェアをインストールします。::

    $ sudo apt-get install -y rinse createrepo rpm

まず yum ディレクトリに移動する。

その後、次のようにすれば一連のリリース作業（build sign update upload）が行われますが、途中で失敗することもあります。 ::

 $ make release

そのため head コマンドなどで Makefile.am の内容を確認し、順番に作業を行っていくほうが良いこともあります。 ::

 $ make build
 $ make sign
 $ make update
 $ make upload

タグを打つ
----------

``make tag`` とするとタグが打たれます。 ::

 $ make tag
 $ git push --tags origin

ドキュメントのアップロード
--------------------------

1. GitHub からドキュメントアップロード用のリポジトリ (mroonga.github.com) を clone
2. clone済みmroongaディレクトリ内でmake update-documentを実行し、clone したドキュメントアップロード用のリポジトリへ反映する
3. mroonga.github.com へコミットを行い GitHub へ push

リリースメールの送信
--------------------

各種メーリングリストにリリースメールを流します。

* ml@mysql.gr.jp 日本語アナウンス
* groonga-dev@lists.sourceforge.jp 日本語アナウンス
* groonga-talk@lists.sourceforge.net 英語アナウンス

メッセージ内容のテンプレートを以下に示します。 ::

 ドキュメント(インストールガイド含む)
   http://mroonga.github.com/

 ダウンロード
   http://github.com/mroonga/mroonga/downloads

 mroongaとは、全文検索エンジンであるgroongaをベースとした
 MySQLのストレージエンジンです。Tritonnの後継プロジェクトとな
 ります。


 最近のトピックス
 ================

 # <<<ユーモアを交えて最近のトピックスを>>>

 先月開催されたMySQL Conference 2011でmroongaについて発表して
 きました。（私じゃなくて開発チームのみなさんが。）英語ですが、
 以下の発表資料があるので興味がある方はご覧ください。

   http://groonga.org/ja/publication/


 いろいろ試してくれている方もいらっしゃるようでありがとうござ
 います。いちいさんなど使った感想を公開してくれていてとても参
 考になります。ありがとうございます。
   http://d.hatena.ne.jp/ichii386/20110427/1303852054

 （↓の変更点にあるとおり、今回のリリースからauto_increment機
 能が追加されています。）


 ただ、「REPLACE INTO処理が完了せずにコネクションを消費する」
 のようなバグレポートがあるように、うまく動かないケースもある
 ようなので、試していただける方は注意してください。
   http://redmine.groonga.org/issues/910

 今日リリースしたgroonga 1.2.2でマルチスレッド・マルチプロセ
 ス時にデータ破損してしまう問題を修正しているので、最新の
 groongaと組み合わせると問題が解決しているかもしれません。

 使ってみて、なにか問題があったら報告してもらえると助かります。

 # <<<<以下 news.rst に書かれている内容を貼り付ける>>>

 変更点
 ======

 0.5からの変更点は以下の通りです。
   http://mroonga.github.com/news.html#release-0-6

 改良
 ----

     auto_increment機能の追加。#670
     不必要な”duplicated _id on insert”というエラーメッセージを抑制。 #910（←は未修正）
     CentOSで利用しているMySQLのバージョンを5.5.10から5.5.12へアップデート。
     Ubuntu 11.04 Natty Narwhalサポートの追加。
     Ubuntu 10.10 Maverick Meerkatサポートの削除。
     Fedora 15サポートの追加。
     Fedora 14サポートの削除。

 修正
 ----

     ORDER BY LIMITの高速化が機能しないケースがある問題の修正。#845
     デバッグビルド時のメモリリークを修正。
     提供しているCentOS用パッケージをOracle提供MySQLパッケージと一緒に使うとクラッシュする問題を修正。

 感謝
 ----

     Mitsuhiro Shibuyaさん
     Hiroki Minetaさん
     @kodakaさん

Twitterでリリースアナウンスをする
---------------------------------

mroongaブログのリリースエントリには「リンクをあなたのフォロワーに共有する」ためのツイートボタンがあるので、そのボタンを使ってリリースアナウンスします。(画面下部に配置されている)

このボタンを経由する場合、ツイート内容に自動的にリリースタイトル(「mroonga 2.08リリース」など)とmroongaブログのリリースエントリのURLが挿入されます。

この作業はmroongaブログの英語版、日本語版それぞれで行います。
あらかじめgroongaアカウントでログインしておくとアナウンスを円滑に行うことができます。

freecode.comへリリース情報を登録
--------------------------------

mroongaプロジェクトではfreecode.comでもリリース情報を発信しています。

freecode.comのプロジェクトページは以下の通りです。

  https://freecode.com/projects/mroonga

プロジェクトの管理メニューから「Submit a release」をクリックし、
新規リリース情報を登録します。

  https://freecode.com/projects/mroonga/releases/new

.. note::
   投稿した内容に対するレビューが運営側で実施されるので、反映されるまでしばらく時間がかかります。

リリース後にやること
---------------------

リリースバージョンを以下のようにして更新します。::

  $ make update-version NEW_VERSION_MAJOR=2 NEW_VERSION_MINOR=0 NEW_VERSION_MICRO=7






