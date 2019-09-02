.. highlightlang:: none

Release procedure (XXX not yet translated)
==========================================

前提条件
--------

リリース手順の前提条件は以下の通りです。

* ビルド環境は Debian GNU/Linux (sid)
* コマンドラインの実行例はzsh

作業ディレクトリ例は以下を使用します。

* MROONGA_DIR=$HOME/work/mroonga
* MROONGA_CLONE_DIR=$HOME/work/mroonga/mroonga.clean
* MROONGA_GITHUB_COM_PATH=$HOME/work/mroonga/mroonga.github.com
* CUTTER_DIR=$HOME/work/cutter
* CUTTER_SOURCE_PATH=$HOME/work/cutter/cutter


ビルド環境の準備
----------------

以下にMroongaのリリース作業を行うために事前にインストール
しておくべきパッケージを示します。

なお、ビルド環境としては Debian GNU/Linux (sid)を前提として説明しているため、その他の環境では適宜読み替えて下さい。::

    % sudo apt-get install -V ruby mecab libmecab-dev gnupg2 dh-autoreconf python-sphinx bison

Debian系（.deb）やRed Hat系（.rpm）パッケージのビルドには `Vagrant <https://www.vagrantup.com/>`_ を使用します。apt-getでインストールできるのは古いバージョンなので、Webサイトから最新版をダウンロードしてインストールすることをおすすめします。

Vagrantで使用する仮想化ソフトウェア（VirtualBox、VMwareなど）がない場合、合わせてインストールしてください。なお、VirtualBoxはsources.listにcontribセクションを追加すればapt-getでインストールできます。::

    % cat /etc/apt/sources.list
    deb http://ftp.jp.debian.org/debian/ sid main contrib
    deb-src http://ftp.jp.debian.org/debian/ sid main contrib
    % sudo apt-get update
    % sudo apt-get install virtualbox

Windows版パッケージのビルド環境準備
-----------------------------------

MroongaのWindows版パッケージを作成するための環境を構築する手順を以下に示します。
ビルド環境は、Windows7を前提として説明しているため、その他の環境では適宜読み替えて下さい。

必要ソフトウェアのインストール
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

* VisualStudio 2015 Community をインストールします。
   * VisualStudio 2015 は、現行のバージョンではなく、古いバージョンのため、通常のダウンロードページからはダウンロードすることができません。そのため、以下のページからダウンロードを行う必要があります。:

    `Visual Studio Dev Essensials <https://www.visualstudio.com/ja/dev-essentials/>`_

ダウンロードするためには、上記サイトへの登録が必要となります。登録には、MicrosoftAccountが必要となります。

* Bison for Windows をインストールします。
   * 以下のサイトからダウンロードします。また、インストール後にパスを通しておく必要があります。インストールパスには、空白スペースを含まないパスを指定してください。:

    `Bison for Windows <http://gnuwin32.sourceforge.net/packages/bison.htm>`_

* CMake をインストールします。
   * CMakeのバージョンは3.1以上をインストールします。
   * 以下のサイトからダウンロードします。また、インストール時にパスを通しておく必要があります。:

    `CMake <https://cmake.org/download/>`_

* Wix をインストールします。
   * 以下のサイトからダウンロードします。:

    `Wix <http://wix.codeplex.com/>`_

* Windows Management Framework 4.0(Windows6.1-KB2819745-x64-MultiPkg.msu) をインストールします。
   * Windows版パッケージを作成するスクリプトはPowerShell 3.0以上向けに書かれているため、Windows7に同梱されているPowerShellでは正常に動作しません。Windows7のPowerShellのバージョンを4.0にするためにインストールする必要があります。Windows 8.1以降では標準でインストールされているPowerShellのバージョンで要件を満たしています。:

    `Windows Management Framework 4.0 <https://www.microsoft.com/ja-jp/download/details.aspx?id=40855>`_

* Patch for Windows をインストールします。
   * 以下のサイトからダウンロードします。:

    `Patch for Windows <http://gnuwin32.sourceforge.net/packages/patch.htm>`_

変更点の記述
------------

前回リリース時からの変更点を``doc/source/news.txt``にまとめます。
ここでまとめた内容についてはリリースアナウンスにも使用します。

前回リリースからの変更履歴を参照するには以下のコマンドを実行します。::

   % git log -p --reverse $(git tag | tail -1)..

ログを^commitで検索しながら、以下の基準を目安として変更点を追記していきます。

含めるもの

* ユーザへ影響するような変更
* 互換性がなくなるような変更

含めないもの

* 内部的な変更(変数名の変更やらリファクタリング)


configureスクリプトの生成
-------------------------

Mroongaのソースコードをcloneした時点ではconfigureスクリプトが含まれておらず、そのままmakeコマンドにてビルドすることができません。

Mroongaをcloneしたディレクトリでautogen.shを以下のように実行します。::

    % sh autogen.sh

このコマンドの実行により、configureスクリプトが生成されます。


configureスクリプトの実行
-------------------------

Makefileを生成するためにconfigureスクリプトを実行します。

リリース用にビルドするためには以下のオプションを指定してconfigureを実行します。::

    % ./configure \
        --enable-document \
        --prefix=/tmp/local \
        --with-launchpad-uploader-pgp-key=(Launchpadに登録したkeyID) \
        --with-mroonga-github-com-path=$HOME/work/mroonga/mroonga.github.com \
        --with-mysql-source=(MySQLのソースコードがあるディレクトリー) \
        --with-mysql-build=(MySQLのソースコードをビルドしたディレクトリー) \
        --with-mysql-config=(mysql_configコマンドのパス)

--with-mysql-sourceなどのオプションについては、イントールドキュメントの :doc:`/install/others` を参照してください。


アップロード権限の確認
----------------------

あらかじめpackagesユーザでpackages.groonga.orgにsshログインできることを確認しておいてください。

ログイン可能であるかの確認は以下のようにコマンドを実行して行います。::

    % ssh packages@packages.groonga.org

ログインできない場合、SSHの公開鍵を登録してもらってください。


make update-latest-releaseの実行
--------------------------------

make update-latest-releaseコマンドでは、OLD_RELEASE_DATEに前回のリリースの日付を、NEW_RELEASE_DATEに次回リリースの日付を指定します。

5.01のリリースを行った際は以下のコマンドを実行しました。::

    % make update-latest-release OLD_RELEASE=5.00 OLD_RELEASE_DATE=2015-02-09 NEW_RELEASE_DATE=2015-03-29

これにより、clone済みのMroongaのWebサイトのトップページのソース(index.html,ja/index.html)やRPMパッケージのspecファイルのバージョン表記などが更新されます。


リリースタグの設定
------------------

リリース用のタグを打つには以下のコマンドを実行します。::

    % make tag
    % git push --tags origin

.. note::
   タグを打った後にconfigureを実行することで、ドキュメント生成時のバージョン番号に反映されます。


配布用ファイルのアップロード
----------------------------

次に、配布用の ``tar.gz`` ファイルを作成します。 ::

    % make dist

.. note::

   以前はGitHubのアーカイブ機能でtar.gzを配布していましたが、その機能が廃止( https://github.com/blog/1302-goodbye-uploads )されるため、2012年12月のリリースよりpackages.groonga.orgでの配布に切り替えました。

packages/sourceディレクトリへと移動します。 ::

    % cd packages/source

make downloadでアップストリームと同期します。 ::

    % make download

必要に応じて、アーカイブに含めるGroongaやgroonga-normalizer-mysql、MariaDBのバージョンを更新します。更新する場合、autogen.shを実行してMakefile.amの変更をMakefile.inに反映する必要があります。 ::

    % editor Makefile.am
    % cd ../..
    % sh autogen.sh
    % cd packages/source

アーカイブを作成します。 ::

    % make archive

アーカイブをアップロードします。 ::

    % make upload

これで、 http://packages.groonga.org/source/mroonga/ から ``tar.gz`` のダウンロードが行えるようになります。


パッケージの作成
----------------

Linux と Windows 用にパッケージを作成する必要があります。

Linux 用のパッケージは以下の 2 種類に分けることが可能です。

1. Debian 系
2. Red Hat 系

.. note::

   現在のところ、パッケージの作成は Debian GNU/Linux (Ubuntu も可) でしか行えません。

Debian 系
^^^^^^^^^

まず apt ディレクトリに移動します。 ::

    % cd apt

その後、次のようにすれば一連のリリース作業（download build sign-packages update-repository sign-repository upload）が行われますが、途中で失敗することもあります。 ::

    % make release

そのため head コマンドなどで Makefile.am の内容を確認し、順番に作業を行っていくほうが良いこともあります。 ::

    % make download
    % make build
    % make sign-packages
    % make update-repository
    % make sign-repository
    % make upload

make build に PARALLEL=yes とするとビルドが並列に走り、作業がより高速に行えます。

また make build CODES=lucid などとすると、ビルド対象を指定することができます。

このように Makefile.am を書き換えずにコマンドライン引数でビルドの挙動を変更する方法は、知っておいて損はないでしょう。

Red Hat 系
^^^^^^^^^^

まず yum ディレクトリに移動する。

その後、次のようにすれば一連のリリース作業（download build sign-packages update-repository upload）が行われますが、途中で失敗することもあります。 ::

    % make release

そのため head コマンドなどで Makefile.am の内容を確認し、順番に作業を行っていくほうが良いこともあります。 ::

    % make download
    % make build
    % make sign-packages
    % make update-repository
    % make upload

Windows
^^^^^^^

MariaDB 本体を `多少変更しないといけない
<https://github.com/mroonga/mroonga/tree/master/packages/source/patches>`_
ため、Windows 版は MariaDB に mroonga/groonga/groonga-normalizer-mysql
をバンドルしたパッケージとして作成します。

Windows 上で作業を行います。

`こちら <https://github.com/cosmo0920/PowerShell-for-Mroonga-building>`_
からWindows版パッケージ作成用のPowerShellをダウンロードします。

PowerShellを管理者権限で起動し、 `powershell\build-vc2015.ps1` を実行します。
`powershell\build-vc2015.ps1` を実行すると、自動的にWindows版のパッケージまで作成します。
32bit版、64bit版のパッケージ作成にそれぞれ30分くらいずつかかります。そのため、合計で1時間くらいかかります。
完了するとworkディレクトリに以下のようなファイルができます。

* mariadb-10.0.2-with-mroonga-3.04-win32.zip
* mariadb-10.0.2-with-mroonga-3.04-winx64.zip

これを Linux にコピーします。例えば、 Ruby で HTTP サーバーを立てて
Linux 側からダウンロードする場合は以下のようにします。::

 > ruby -run -e httpd -- --do-not-reverse-lookup --port 10080 .

ドキュメントのアップロード
--------------------------

1. GitHub からドキュメントアップロード用のリポジトリ (mroonga.github.com) を clone
2. clone済みmroongaディレクトリ内でmake update-documentを実行し、clone したドキュメントアップロード用のリポジトリへ反映する
3. mroonga.github.com へコミットを行い GitHub へ push

Homebrewの更新
--------------

OS Xでのパッケージ管理方法として `Homebrew <http://brew.sh/>`_ があります。

Groongaの場合はHomebrewへpull requestを送りますが、Mroongaの場合は別途用意してあるhomebrewリポジトリを更新します。

  https://github.com/mroonga/homebrew

mroonga/homebrewをcloneして、Formula更新用のシェルスクリプトを実行します。update.shの引数にはリリース時のバージョンを指定します。例えば、3.06のリリースのときは以下を実行しました。 ::

    % ./update.sh 3.06

実行すると、FormulaのソースアーカイブのURLとsha256チェックサムを更新します。
あとは、変更内容をコミットすればHomebrewの更新作業は完了です。

リリースメールの送信
--------------------

各種メーリングリストにリリースメールを流します。

* ml@mysql.gr.jp 日本語アナウンス
* mysql@lists.mysql.com 英語アナウンス (http://lists.mysql.com/mysql から登録できる)
* groonga-dev@lists.osdn.me 日本語アナウンス
* groonga-talk@lists.sourceforge.net 英語アナウンス

メッセージ内容のテンプレートを以下に示します。 ::

    ドキュメント(インストールガイド含む)
      http://mroonga.org/

    ダウンロード
      http://packages.groonga.org/source/mroonga

    Mroongaとは、全文検索エンジンであるGroongaをベースとした
    MySQLのストレージエンジンです。Tritonnの後継プロジェクトとな
    ります。


    最近のトピックス
    ================

    # <<<ユーモアを交えて最近のトピックスを>>>

    先月開催されたMySQL Conference 2011でMroongaについて発表して
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

    今日リリースしたGroonga 1.2.2でマルチスレッド・マルチプロセ
    ス時にデータ破損してしまう問題を修正しているので、最新の
    Groongaと組み合わせると問題が解決しているかもしれません。

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

Mroongaブログのリリースエントリには「リンクをあなたのフォロワーに共有する」ためのツイートボタンがあるので、そのボタンを使ってリリースアナウンスします。(画面下部に配置されている)

このボタンを経由する場合、ツイート内容に自動的にリリースタイトル(「Mroonga 2.08リリース」など)とMroongaブログのリリースエントリのURLが挿入されます。

この作業はMroongaブログの英語版、日本語版それぞれで行います。
あらかじめgroongaアカウントでログインしておくとアナウンスを円滑に行うことができます。

リリース後にやること
---------------------

リリースバージョンを以下のようにして更新します。::

    % make update-version NEW_VERSION_MAJOR=2 NEW_VERSION_MINOR=0 NEW_VERSION_MICRO=7






