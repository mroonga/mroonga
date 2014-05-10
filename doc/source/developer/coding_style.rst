.. highlightlang:: none

TODO: Translate.

コーディングスタイル
====================

一般的に1つのコードベースに複数のスタイルがまざっているとソースコードが読みづらくなります。たとえ、それぞれのスタイル単独では読みやすいスタイルあっても、まざると読みづらくなります。そのため、Mroongaプロジェクトでもスタイルを統一して、読みやすいソースコードになるようにします。

読みやすいソースコードにする理由は以下の通りです。

  * 機能を拡張するときのコストを下げる。
  * 問題を修正するときのコストを下げる。

どちらの場合も周辺のソースコードを読んで、それをベースにコードを追加・変更します。このとき、ソースコードが読みやすい状態だと周辺のソースコードの把握をスムーズに行うことができ、スムーズにその後の作業に移れます。

TODO: 読みやすさの他にデバッグのしやすさ（gdbでの追いやすさ）も考慮に入れたほうがよさそうだがどうしよう。

言語
----

基本的にすべてC++で記述します。よほどのことがない限りCは使いません。

よい例::

    ha_mroonga.cpp

悪い例（C言語を使っている）::

    mrn_sys.c

ファイル名
----------

ソースコードのファイル名は全て小文字にします。また、単語ごとに"_"で区切ります。

よい例::

    ha_mroonga.cpp

悪い例（大文字を使っている）::

    HA_MROONGA.cpp

悪い例（単語を"_"で区切らずにくっつけている）::

    hamroonga.cpp

悪い例（単語を"-"で区切っている）::

    ha-mroonga.cpp

ソースコードの拡張子 ``.cpp`` にします。

よい例::

    ha_mroonga.cpp

悪い例（ ``.cc`` を使っている）::

    ha_mroonga.cc

ヘッダーファイルの拡張子は ``.hpp`` にします。

よい例::

    ha_mroonga.hpp

悪い例（ ``.h`` を使っている）:

    ha_mroonga.h

名前空間
--------

ヘッダーファイルでは ``using namespace`` を使わない。ソースコードでは ``using namespace std`` であれば使ってもよい。他の名前空間は使ってはいけない。

よい例::

    ha_mroonga.cpp:
      using namespace std;

悪い例（ヘッダーファイル内で ``using namespace`` を使っている）::

    ha_mroonga.hpp:
      using namespace std;

悪い例（ ``std`` 以外の名前空間に対して ``using namespace`` を使っている）::

    ha_mroonga.cpp:
      using namespace zmq;

``include`` ガード
------------------

2重 ``include`` を防ぐためのマクロは、ヘッダーファイルの名前をすべて大文字にし、単語の区切りをアンダースコアにしたものにし、最後にアンダースコアをつけて ``HEADER_FILE_NAME_HPP_`` という名前にする。

よい例::

    mrn_db_path.hpp:
      #ifndef MRN_DB_PATH_HPP_
      #define MRN_DB_PATH_HPP_
      ...
      #endif // MRN_DB_PATH_HPP_

悪い例（小文字になっている）::

    mrn_db_path.hpp:
      #ifndef mrn_db_path_hpp_
      #define mrn_db_path_hpp_
      ...
      #endif // mrn_db_path_hpp_

悪い例（アンダースコアが最後ではなく先頭についている）::

    mrn_db_path.hpp:
      #ifndef _MRN_DB_PATH_HPP
      #define _MRN_DB_PATH_HPP
      ...
      #endif // _MRN_DB_PATH_HPP

代入
----

``=`` の前後に1つスペースを入れる。

よい例::

    int i = 0;

悪い例（スペースが入っていない）::

    for (i=0; i<10; ++i) {...}

悪い例（2つスペースが入っている）::

    int i  = 0;

文字列
------

文字列はポインタと長さで表現する。 ``\0`` での終端を仮定しない。

よい例（本当はもっとすっきりした例がよいけど。。。）::

    char *raw_data = "table_name column_name column_value"
    char *column_name;
    size_t column_name_size;
    column_name = raw_data + strlen("table_name ");
    column_name_size = strlen("column_name");

悪い例（無理やり ``\0`` 終端にしている）::

    char *raw_data = "table_name column_name column_value"
    char *column_name;
    column_name = strndup(raw_data + strlen("table_name "), strlen("column_name"));

ただし、ファイル名など ``\0`` が前提であるものに関しては ``\0`` 終端を仮定してよい。

よい例::

    char *database_path = "db/test.mrn";

悪い例（ ``\0`` 終端を仮定せず、長さも管理している）::

    char *database_path = "db/test.mrn";
    size_t database_path_size = strlen("db/test.mrn");

``std::string`` は内部でメモリ確保などの処理が発生するので多用しない。

よい例::

    char database_path[MAX_PATH];

悪い例（最大サイズがわかっているのに ``std::string`` を使っている）::

    std::string database_path;

バッファ
--------

TODO: ちゃんと考える。

何度も繰り返し使う領域（バッファ）が必要な場合は ``GRN_BULK`` を使う。例えば、カラムの値を取得する領域などである。

命名規則
--------

クラス名
^^^^^^^^

クラスの名前は ``UpperCamelCase`` とする。

よい例::

    class MyClass
    {
    }

悪い例（ ``snail_case`` である）::

    class my_class
    {
    }

ただし、 ``ha_mroonga`` などMySQLとのインターフェイスとなるクラスでかつ他の類似のモジュールに命名規則がある場合はそれに従う。

よい例::

    class ha_mroonga: public handler
    {
    }

悪い例（ ``UpperCamelCase`` になっている）::

    class HaMroonga: public handler
    {
    }

メンバー変数名
^^^^^^^^^^^^^^

メンバー変数名は ``snail_case`` とし、末尾にアンダースコア（ ``_`` ）を付ける。

よい例::

     class MyClass
     {
       char *my_name_;
     }

悪い例（ ``UpperCamelCase`` である）::

     class MyClass
     {
       char *MyName_;
     }

悪い例（末尾にアンダースコアがない）::

     class MyClass
     {
       char *my_name;
     }

読み込み用アクセサ名
^^^^^^^^^^^^^^^^^^^^

メンバー変数の値を読み込むメソッドの名前はメンバー変数名の末尾のアンダースコアを除いたものにする。

よい例::

    class MyClass
    {
      char *my_name_;
      const char *my_name() {return my_name_;};
    }

悪い例（末尾にアンダースコアが残っている）::

    class MyClass
    {
      char *my_name_;
      const char *my_name_() {return my_name_;};
    }

悪い例（先頭に ``get_`` を付けている）::

    class MyClass
    {
      char *my_name_;
      const char *_my_name() {return my_name_;};
    }

書き込み用アクセサ名
^^^^^^^^^^^^^^^^^^^^

メンバー変数の値を設定するメソッドの名前は、メンバー変数名の末尾のアンダースコアを除き、先頭に ``set_`` を加えたものにする。

よい例::

    class MyClass
    {
      unsigned int age_;
      void set_age(unsigned int age)
      {
        age_ = age;
      };
    }

悪い例（末尾にアンダースコアが残っている）::

    class MyClass
    {
      unsigned int age_;
      void set_age_(unsigned int age)
      {
        age_ = age;
      };
    }

悪い例（先頭に ``set_`` ではなく ``update_`` を付けている）::

    class MyClass
    {
      unsigned int age_;
      void update_age(unsigned int age)
      {
        age_ = age;
      };
    }

コピーコンストラクター
----------------------

基本的にコピーコンストラクターの使用を禁止する。よほどのことがなければ使用しないこと。

コピーコンストラクターは暗黙的に無駄なコピーが発生する可能性があるためパフォーマンス上の問題がある。コピーではなくポインターやリファレンスを用いること。

また、デフォルトのコピーコンストラクター実装はメンバー変数のポインターの値をそのままコピーするため、デコンストラクターで二重に解放してしまう危険性がある。そのため、明示的にコピーコンストラクターを定義しない場合は無効にする。

よい例::

    class MyClass
    {
    private:
      MyClass(const MyClass &);
    }

悪い例（コピーコンストラクターを禁止していない）::

    class MyClass
    {
    }

悪い例（カスタムコピーコンストラクターを使っている）::

    class MyClass
    {
      unsigned int age_;
      MyClass(const MyClass &object)
      {
        age_ = object.age_;
      }
    }

クラスの代入
------------

基本的に定義したクラスの代入を禁止する。よほどのことがなければ使用しないこと。

代入演算子は暗黙的に無駄なコピーが発生する可能性があるためパフォーマンス上の問題がある。コピーではなくポインターやリファレンスを用いること。

また、デフォルトの代入演算子の実装はメンバー変数のポインターの値をそのままコピーするため、デコンストラクターで二重に解放してしまう危険性がある。そのため、明示的に代入演算子を定義しない場合は無効にする。

よい例::

    class MyClass
    {
    private:
      MyClass &operator=(const MyClass &);
    }

悪い例（代入を禁止していない）::

    class MyClass
    {
    }

悪い例（代入を使っている）::

    class MyClass
    {
      unsigned int age_;
      MyClass &operator=(const MyClass &object)
      {
        age_ = object.age_;
        return *this;
      }
    }

引数
----

voidを省略
^^^^^^^^^^

引数がない場合は ``void`` を省略する。

よい例::

    class MyClass
    {
      unsigned int age_;
      unsigned int age()
      {
         return age_;
      };
    }

悪い例（ ``void`` を省略していない）::

    class MyClass
    {
      unsigned int age_;
      unsigned int age(void)
      {
         return age_;
      };
    }

入力用引数にはconstを付ける
^^^^^^^^^^^^^^^^^^^^^^^^^^^

入力のみに用いる引数には ``const`` を付ける。これは、入力のみに用いる引数である事を明示するためと、間違って引数を変更してしまわないためである。

よい例::

    class Table
    {
      void insert(unsigned int id, const char *column_name, const char *value)
      {
         Record *record = records[i];
         Column *column = columns[column_name];
         column.set_value(value);
      }
    }

悪い例（入力のみに用いているのに ``const`` が付いていない）::

    class Table
    {
      void insert(unsigned int id, char *column_name, char *value)
      {
         Record *record = records[i];
         Column *column = columns[column_name];
         column.set_value(value);
      }
    }

定数
----

フラグやサイズなどを示す定数には ``const`` オブジェクトを用いる。これはデバッガー上でプログラムを走らせているときに名前で値を参照できるようにするためである。

よい例::

    const char *MRN_LOG_FILE_PATH = "groonga.log";

悪い例（ ``#define`` を用いている）::

    #define MRN_LOG_FILE_PATH "groonga.log"

真偽値
------

bool型を用いる
^^^^^^^^^^^^^^

真偽値には ``bool`` 型を用いる。

よい例::

    bool is_searching;

悪い例（ ``int`` 型を用いている）::

    int is_searching;

真偽値のリテラルには ``true`` または ``false`` を用いる
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

真偽値の値として ``true`` と ``false`` はより完結で説明的だからである。

よい例::

    bool is_searching = true;

悪い例（ ``0`` 以外の値を真の値として用いている）::

    bool is_searching = 1;

条件式
------

真偽値は比較しない
^^^^^^^^^^^^^^^^^^

真偽値の値は ``boolean_value == true`` などとせず、 ``boolean_value`` として条件式に使用する。すでに真偽値の値を真偽値のリテラルと比較することは重複したコードだからである。

よい例::

    boolean is_searching = true;
    if (!is_searching) { ... }

悪い例（真偽値のリテラルと比較している）::

    boolean is_searching = true;
    if (is_searching == false) { ... }

``NULL`` と比較しない
^^^^^^^^^^^^^^^^^^^^^^

``NULL`` かどうかを条件式に使う場合は ``value == NULL`` ではなく ``!value`` というように書く。多くの言語で ``NULL`` に相当する値（たとえばLispの ``nil`` ）は偽を表すため、明示的に ``NULL`` と比較しなくても意図は伝わるからである。

よい例::

    char *name = NULL;
    if (!name) { ... }

悪い例（ ``NULL`` と比較している）::

    char *name = NULL;
    if (name == NULL) { ... }

数値は比較する
^^^^^^^^^^^^^^

CやC++では ``0`` は偽、 ``0`` 以外は真の値となるが、条件式に数値を使う場合は ``strcmp(...) == 0`` などというように明示的に比較する。

C++では真偽値に ``bool`` を使うためこのような状況は発生しないが、C言語由来のAPIでは ``int`` で真偽値を表現している場合が多い。しかし、 ``int`` だけでは真偽値として使っているか本当に数値として使っているかがわかりにくいため、 ``int`` のときはすべて数値として扱う。

よい例::

     if (memcmp(value1, value2, value_size) == 0) {
       printf("same value!\n");
     }

悪い例（ ``0`` を偽の値として扱っている）::

     if (!memcmp(value1, value2, value_size)) {
       printf("same value!\n");
     }

初期化
------

メンバー変数の初期化には初期化リストを用いる
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

無駄な処理を省くためにコンストラクターでのメンバー変数の初期化には初期化リストを用いる。初期化リストを用いないとコンストラクターの処理とコピーコンストラクター・代入処理が行われたりなど非効率である。（後述）

よい例::

    class Table
    {
      Table(const char *name);
      std::string name_;
    }

    Table::Table(const char *name) :
      name_(name)
    {
    }

悪い例（ ``std::string(name)`` のところでコンストラクターが動き、 ``name_ = ...`` のところで代入演算子が動いて2回初期化している）::

    class Table
    {
      Table(const char *name);
      std::string name_;
    }

    Table::Table(const char *name)
    {
      name_ = std::string(name);
    }

変数宣言と同時に初期化する
^^^^^^^^^^^^^^^^^^^^^^^^^^

変数を宣言したときに同時に初期化する。宣言時に初期化せずに代入して初期化すると、無駄な処理が発生する可能性があるため非効率である。（後述）

よい例::

    std::string name("users");

悪い例（ ``std::string()`` のところでコンストラクターが動き、 ``name = ...`` のところで代入演算子が動いて2回初期化している）::

    std::string name;
    name = std::string("users");

インクリメント・デクリメント
----------------------------

前置形式を用いる
^^^^^^^^^^^^^^^^

後置形式ではオブジェクトのコピーをしなければいけないため非効率である。そのため、できるだけ前置形式を用いる。

よい例（ ``int`` だと効率は変わらないので本当はあんまりよい例ではない）::

    for (int i = 0; i < 10; ++i) {
    }

悪い例（後置形式を用いている）::

    for (int i = 0; i < 10; ++i) {
    }

キャスト
--------

C++のスタイルを用いる
^^^^^^^^^^^^^^^^^^^^^

Cスタイルのキャストはなんでもキャストできてしまうため、意図しないキャストにも気付かない可能性がある。例えば、単に ``const`` を外したいだけなのに、間違って違う型に変換していても気付けない。C++のキャストでは ``const`` を外したいときは ``const_cast`` を使用し、型を変換するときは ``static_cast`` を指定する。こうすれば、 ``static_cast`` で間違って ``const`` を外してしまっている場合も気付ける。 ``reinterpret_cast`` はどうしても必要なときのみ注意して使う。

よい例（ ``const_cast`` を使っている）::

    uchar *to_key;
    const ucahr *from_key;
    KEY *key_info;
    uint key_length;
    key_copy(to_key, const_cast<uchar *>from_key, key_info, key_length);

よい例（ ``static_cast`` を使っている）::

    int n_hits = 1;
    int n_documents = 10;
    float hit_ratio = (float)(n_hits) / n_documents;

よい例（ ``static_cast`` では無理なので ``reinterpret_cast`` を使っている）::

    THD *thread = current_thd;
    my_hash_delete(&mrn_allocated_thds, reinterpret_cast<uchar *>(thread));

悪い例（Cスタイルのキャストを使っている）::

    int n_hits = 1;
    int n_documents = 10;
    float hit_ratio = (float)(n_hits) / n_documents;

悪い例（ ``static_cast`` で十分なのに ``reinterpret_cast`` を使っている）::

    void *value = get_value(key);
    char *name;
    name = reinterpret_cast<char *>(value);

変数宣言
--------

ポインタ型を示す ``*`` とリファレンス型を示す ``&`` は変数名に寄せる
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Cと同様にポインタ型を示す ``*`` は型名ではなく変数名に寄せる。これは、以下のように複数の変数を一度に宣言したときに一貫性がなくなるためである。2つめ以降の変数は近くに型名がないため ``*`` を寄せる場所がない。

例::

    char* key, *value;

同様に、リファレンス型を示す ``&`` も変数名に寄せる。

なお、 ``*`` や ``&`` と型名の間にはスペースを入れない。

よい例::

    char *key;

よい例::

    bool is_exist(const std::string &file_name);

悪い例（型名に寄せている）::

    char* key;

その他
------

  * ここに書いていないものについては特にスタイルを定めないが、
    プロジェクト内で常に一貫性のあるスタイルを使用すること。
    同じような事をするときは同じような書き方にすること。
    複数の書き方で同じようなことを実現している場合は1つの方法に合わせること。

以下、具体例が必要。

  * ビルド時にできることを実行時に延ばさない(静的チェックを活用)
  * なるべく局所的に変数を定義し、同時に初期化する
  * 長い関数や深いブロックのネストを避ける
    * 2つに分けた方がよさそう。あと目安があるといいかも。
      100行以上は長いよねーとか3段以上はデンジャーとか。
  * 必要以上にオブジェクトを複製しない
  * 暗黙の型変換はなるべく避ける
  * assertを積極的に使う

メモ
----

  * 動的テンプレートを使わない。（要追加情報。implicit
    template instantiationのことであれば、これはふつうに使わ
    れているものなので特に禁止しないんでいいんじゃない説？）
  * typeidを使わない。
  * 例外はMySQLで問題がないようであればOK。Mroongaから外の世
    界（MySQLの世界）にはださないようにする。
