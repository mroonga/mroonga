.. -*- rst -*-

.. highlightlang:: none

How to contribute in documentation topics
=========================================

We use Sphinx for documentation tool.

具体的にどんなことをやればいいのかを説明します。基本的にはドキュメントのソースファイルごとにpull requestを送ってもらうと進めやすいです。

対象となるファイルはdoc/sourceディレクトリ以下の拡張子が「.rst」となっているファイルです。

あまりGitHubでの作業に慣れていなくてもできるように、「最初にやること」と「作業ごとにやること」、「ファイルごとにやること」に分けて順に説明します。

* The things you must do at first
* The things you need to do every tasks
* The things you need to do every files

The things you must do at first
-------------------------------

以下では、最初に一度だけ実施しておけば良いことを説明します。

Required softwares
^^^^^^^^^^^^^^^^^^

TODO

Git configuration
^^^^^^^^^^^^^^^^^

まずは、gitの設定をしましょう。すでにある程度gitを使っている場合には初期設定はすでに完了しているかも知れません。その場合には飛ばして構いません。::

  % git config --global user.name "Your Name"
  % git config --global user.email "Email address"

上記はコミットログに使われます。公開しても差し支えないユーザ名もしくはメールアドレスを設定します。

Fork on GitHub
^^^^^^^^^^^^^^

First, create GitHub account. If your GitHub account is ready, login to GitHub and access following URL.

* `Fork the Mroonga repository <https://github.com/mroonga/mroonga/fork>`_

Fork リポジトリ選択画面でご自分のリポジトリへとforkしてください。

Initial configuration for working repository
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Clone Mroonga repository to working directory. Don't forget to do "Git configuration".::

  % git clone git@github.com:(YOUR_GITHUB_ACCOUNT)/mroonga.git
  % cd mroonga
  % git remote add upstream git@github.com:mroonga/mroonga

Initial configuration for building documentation
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Execute following commands to prepare for generating Mroonga documentation::

  % ./autogen.sh
  % ./configure --enable-document --with-mysql-source=(SOURCE_DIRECTORY_OF_MySQL)

Next step is "The things you need to do every tasks".

The things you need to do every tasks
------------------------------------------

以下では作業ごとにやることを説明します。

Follow the upstream
^^^^^^^^^^^^^^^^^^^

Mroonga本家の最新状態に追従して、作業がかぶらないようにします。::

  % git fetch --all
  % git checkout master
  % git rebase upstream/master

最新の状態に追従できたら、「ファイルごとにやること」へと進みます。

The things you need to do every files
-------------------------------------

以下では、例えば http://mroonga.org/docs/characteristic.html を更新する場合で説明します。作業対象となるファイルは、リポジトリのdoc/source/ディレクトリ以下にあり拡張子が.rstなファイルです。今回は、doc/source/characteristic.rstを変更する例で説明します。

Create working branch
^^^^^^^^^^^^^^^^^^^^^

Create a working branch. Use meaningful branch name.

  % git checkout -b use-capitalized-notation-characteristic

Editing text
^^^^^^^^^^^^

Fix typos, styles or write a new document for Mroonga.

Confirm generated document
^^^^^^^^^^^^^^^^^^^^^^^^^^

マークアップに問題がないか、HTMLを確認します。HTMLを生成するには以下のコマンドを実行します。::

  % cd doc/locale/en
  % make html

いつも使っているブラウザで該当ファイルを確認して、変更した内容が反映されていればOKです。::

  % firefox html/characteristic.html


Commit
^^^^^^

HTMLに問題がないことを確認できたら、コミットします。::

  % cd ${cloneしたディレクトリーのトップディレクトリー}
  % git add doc/source/characteristic.rst
  % git commit

コミットするときのメッセージについては、例えば以下のようにします。::

  doc: use "Mroonga" notation

Push and pull request
^^^^^^^^^^^^^^^^^^^^^

Publish your changes to your own GitHub repository::

  % git push -u origin use-capitalized-notation-characteristic

Note that ``use-capitalized-notation-characteristic`` is already created branch in advance.

ブラウザで https://github.com/(GitHubのアカウント)/mroonga を開くと「 @use-capitalized-notation-characteristic@ 」ブランチをpull requestする！みたいなUIができているので、そこのボタンを押してpull requestしてください。入力フォームがでてきますが、コミットしたときメッセージで十分なのでそのままpull requestしてOKです！

これで、ひととおりの作業は完了しました。

