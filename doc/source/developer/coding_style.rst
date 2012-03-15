.. highlightlang:: none

TODO: Translate.

コーディングスタイル
===================

一般的に1つのコードベースに複数のスタイルがまざっているとソースコードが読みづらくなります。たとえ、それぞれのスタイル単独では読みやすいスタイルあっても、まざると読みづらくなります。そのため、mroongaプロジェクトでもスタイルを統一して、読みやすいソースコードになるようにします。

読みやすいソースコードにする理由は以下の通りです。

  * 機能を拡張するときのコストを下げる。
  * 問題を修正するときのコストを下げる。

どちらの場合も周辺のソースコードを読んで、それをベースにコードを追加・変更します。このとき、ソースコードが読みやすい状態だと周辺のソースコードの把握をスムーズに行うことができ、スムーズにその後の作業に移れます。

TODO: 読みやすさの他にデバッグのしやすさ（gdbでの追いやすさ）も考慮に入れたほうがよさそうだがどうしよう。

言語
----

基本的にすべてC++で記述します。よほどのことがない限りCは使いません。

よい例:

    ha_mroonga.cpp

悪い例（C言語を使っている）:

    mrn_sys.c

ファイル名
----------

ソースコードのファイル名は全て小文字にします。また、単語ごとに"_"で区切ります。

よい例:

    ha_mroonga.cpp

悪い例（大文字を使っている）:

    HA_MROONGA.cpp

悪い例（単語を"_"で区切らずにくっつけている）:

    hamroonga.cpp

悪い例（単語を"-"で区切っている）:

    ha-mroonga.cpp

ソースコードの拡張子 ``.cpp`` にします。

よい例:

    ha_mroonga.cpp

悪い例（ ``.cc`` を使っている）:

    ha_mroonga.cc

ヘッダーファイルの拡張子は ``.hpp`` にします。

よい例:

    ha_mroonga.hpp

悪い例（ ``.h`` を使っている）:

    ha_mroonga.h

名前空間
--------

ヘッダーファイルでは ``using namespace`` を使わない。ソースコードでは ``using namespace std`` であれば使ってもよい。他の名前空間は使ってはいけない。

よい例:

    ha_mroonga.cpp:
      using namespace std;

悪い例（ヘッダーファイル内で ``using namespace`` を使っている）:

    ha_mroonga.hpp:
      using namespace std;

悪い例（ ``std`` 以外の名前空間に対して ``using namespace`` を使っている）:

    ha_mroonga.cpp:
      using namespace zmq;

文字列
------

文字列はポインタと長さで表現する。 ``\0`` での終端を仮定しない。

よい例（本当はもっとすっきりした例がよいけど。。。）:

    char *raw_data = "table_name column_name column_value"
    char *column_name;
    size_t column_name_size;
    column_name = raw_data + strlen("table_name ");
    column_name_size = strlen("column_name");

悪い例（無理やり ``\0`` 終端にしている）:

    char *raw_data = "table_name column_name column_value"
    char *column_name;
    column_name = strndup(raw_data + strlen("table_name "), strlen("column_name"));

ただし、ファイル名など ``\0`` が前提であるものに関しては ``\0`` 終端を仮定してよい。

よい例:

    char *database_path = "db/test.mrn";

悪い例（ ``\0`` 終端を仮定せず、長さも管理している）:

    char *database_path = "db/test.mrn";
    size_t database_path_size = strlen("db/test.mrn");

``std::string`` は内部でメモリ確保などの処理が発生するので多用しない。

よい例:

    char database_path[MAX_PATH];

悪い例（最大サイズがわかっているのに ``std::string`` を使っている）:

    std::string database_path;

バッファ
--------

TODO: ちゃんと考える。

何度も繰り返し使う領域（バッファ）が必要な場合は ``GRN_BULK`` を使う。例えば、カラムの値を取得する領域などである。
