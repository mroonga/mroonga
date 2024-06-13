require 'optparse'
require 'fileutils'

option = {}
OptionParser.new do |opt|
  opt.on('--release-date=DATE', "YYYY-MM-DD") { |v| option[:release_date] = v }
  opt.on('--version=VERSION', "e.g. 12.1.1") { |v| option[:version] = v }
  opt.on('--previous-version=VERSION', "e.g. 12.1.0") { |v| option[:previous_version] = v }
  opt.on('--groonga-repository=PATH', "e.g. $HOME/work/groonga") { |v| option[:groonga_repository] = v }
  opt.on('--mroonga-org-repository=PATH', "e.g. $HOME/work/mroonga.org") { |v| option[:mroonga_org_repository] = v }
  opt.parse!(ARGV)
end

require "#{option[:groonga_repository]}/doc/announcement-article-generator"

class MroongaArticleGenerator < ArticleGenerator
  def initialize(release_date, version, previous_version, mroonga_org_repository)
    super(release_date, version, previous_version)
  end
end

class MarkdownEnArticleGenerator < MroongaArticleGenerator
  def initialize(release_date, version, previous_version, mroonga_org_repository)
    super(release_date, version, previous_version, mroonga_org_repository)
    @input_file_path = "./locale/en/markdown/news/#{version.split(".")[0]}.md"
    @release_headline_regexp_pattern = /^## Release.+$/
  end

  def generate_article
    <<-ARTICLE
## Mroonga #{@version} has been released!

Mroonga is a MySQL storage engine that supports fast fulltext search
and geolocation search. It is CJK ready. It uses Groonga as a storage
and fulltext search engine.

[Mroonga #{@version}](#{@link_prefix}/news.html#release-#{@version_in_link}) has been released!

* How to install: [Install](#{@link_prefix}/install.html)
* How to upgrade: [Upgrade guide](#{@link_prefix}/upgrade.html)

### Changes

Here are important changes in this release:

#{extract_latest_release_note}

### Conclusion

Please refer to [release note](#{@link_prefix}/news.html#release-#{@version_in_link}) about the detail of changes in this release.

Let's search by Mroonga!
    ARTICLE
  end
end

class MarkdownJaArticleGenerator < MroongaArticleGenerator
  def initialize(release_date, version, previous_version, mroonga_org_repository)
    super(release_date, version, previous_version, mroonga_org_repository)
    @input_file_path = "./locale/ja/markdown/news/#{version.split(".")[0]}.md"
    @release_headline_regexp_pattern = /^## .*リリース.+$/
  end

  def generate_article
    <<-ARTICLE
## Mroonga #{@version} リリース

[Mroonga #{@version}](#{@link_prefix}/news.html#release-#{@version_in_link})をリリースしました！

* インストール方法: [インストール](#{@link_prefix}/install.html)
* アップグレード方法: [アップグレードガイド](#{@link_prefix}/upgrade.html)

### 変更点
    
主な変更点は以下の通りです。

#{extract_latest_release_note}

### おわりに

今回のリリースの詳細については、 [リリースノート](#{@link_prefix}/news.html#release-#{@version_in_link}) または、 [リリース自慢会](https://www.youtube.com/watch?v=ov33wL5HBZg) を参照してください。

それでは、Mroongaでガンガン検索してください！
    ARTICLE
  end
end

class BlogEnArticleGenerator < MarkdownEnArticleGenerator
  def initialize(release_date, version, previous_version, mroonga_org_repository)
    super(release_date, version, previous_version, mroonga_org_repository)
    @output_file_path = "#{mroonga_org_repository}/en/_posts/#{release_date}-mroonga-#{version}.md"
    @link_prefix = "/docs"
  end

  def generate_article
    article_base = super
    prefix = <<-ARTICLE
---
layout: post.en
title: Mroonga #{@version} has been released!
description: Mroonga #{@version} has been released!
---

#{super}
    ARTICLE
  end
end

class BlogJaArticleGenerator < MarkdownJaArticleGenerator
  def initialize(release_date, version, previous_version, mroonga_org_repository)
    super(release_date, version, previous_version, mroonga_org_repository)
    @output_file_path = "#{mroonga_org_repository}/ja/_posts/#{release_date}-mroonga-#{version}.md"
    @link_prefix = "/ja/docs"
  end

  def generate_article
    article_base = super
    prefix = <<-ARTICLE
---
layout: post.ja
title: Mroonga #{@version}リリース！
description: Mroonga #{@version}をリリースしました！
---

#{super}
    ARTICLE
  end
end

class DiscussionsEnArticleGenerator < MarkdownEnArticleGenerator
  def initialize(release_date, version, previous_version, mroonga_org_repository)
    super(release_date, version, previous_version, mroonga_org_repository)
    @output_file_path = "./tmp/discussions-en-#{release_date}-mroonga-#{version}.md"
    @link_prefix = "https://mroonga.org/docs"
  end

  def generate
    FileUtils.mkdir_p("tmp")
    super
  end
end

class DiscussionsJaArticleGenerator < MarkdownJaArticleGenerator
  def initialize(release_date, version, previous_version, mroonga_org_repository)
    super(release_date, version, previous_version, mroonga_org_repository)
    @output_file_path = "./tmp/discussions-ja-#{release_date}-mroonga-#{version}.md"
    @link_prefix = "https://mroonga.org/ja/docs"
  end

  def generate
    FileUtils.mkdir_p("tmp")
    super
  end
end

class FacebookArticleGenerator < MroongaArticleGenerator
  def generate
    FileUtils.mkdir_p("tmp")
    super
  end
end

class FacebookEnArticleGenerator < FacebookArticleGenerator
  def initialize(release_date, version, previous_version, mroonga_org_repository)
    super(release_date, version, previous_version, mroonga_org_repository)
    @input_file_path = "./locale/en/text/news/#{version.split(".")[0]}.txt"
    @output_file_path = "./tmp/facebook-en-#{release_date}-mroonga-#{version}.txt"
    @release_headline_regexp_pattern = /^Release.*\n=.+$/
    @link_prefix = "https://mroonga.org/docs"
  end

  def generate_article
    <<-ARTICLE
Hi,
Mroonga #{@version} has been released!

Mroonga is a MySQL storage engine that supports fast fulltext search
and geolocation search. It is CJK ready. It uses Groonga as a storage
and fulltext search engine.

Mroonga #{@version} has been released!

* Release note: #{@link_prefix}/news.html#release-#{@version_in_link}
* How to install: #{@link_prefix}/install.html
* How to upgrade: #{@link_prefix}/upgrade.html

Changes
=======

The main changes are as follows.

#{extract_latest_release_note}

Conclusion
==========

Please refer to release note ( #{@link_prefix}/news.html#release-#{@version_in_link} ) about the detail of changes in this release.

Let's search by Mroonga!
    ARTICLE
  end
end

class FacebookJaArticleGenerator < FacebookArticleGenerator
  def initialize(release_date, version, previous_version, mroonga_org_repository)
    super(release_date, version, previous_version, mroonga_org_repository)
    @input_file_path = "./locale/ja/text/news/#{version.split(".")[0]}.txt"
    @output_file_path = "./tmp/facebook-ja-#{release_date}-mroonga-#{version}.txt"
    @release_headline_regexp_pattern = /^.*リリース.+\n=.+/
    @link_prefix = "https://mroonga.org/ja/docs"
  end

  def generate_article
    <<-ARTICLE
Mroonga #{@version} をリリースしました！

* リリースノート: #{@link_prefix}/news.html#release-#{@version_in_link}

* インストール方法: #{@link_prefix}/install.html

* アップグレード方法: #{@link_prefix}/upgrade.html

主な変更内容
============

主な変更点は以下の通りです。

#{extract_latest_release_note}

おわりに
========

今回のリリースの詳細については、 リリースノート( #{@link_prefix}/news.html#release-#{@version_in_link} ) または、リリース自慢会( https://www.youtube.com/playlist?list=PLLwHraQ4jf7PnA3GjI9v90DZq8ikLk0iN ) を参照してください。

それでは、Mroongaでガンガン検索してください！
    ARTICLE
  end
end

class TwitterEnArticleBaseGenerator < MroongaArticleGenerator
  def initialize(release_date, version, previous_version, mroonga_org_repository)
    super(release_date, version, previous_version, mroonga_org_repository)
    @output_file_path = "./tmp/twitter-en-#{release_date}-mroonga-#{version}-base.txt"
  end

  def generate_article
    "Mroonga #{@version} has been released!(#{@release_date}) " + 
    "https://mroonga.org/en/blog/#{@release_date_in_link}/mroonga-#{@version}.html"
  end
end

class TwitterJaArticleBaseGenerator < MroongaArticleGenerator
  def initialize(release_date, version, previous_version, mroonga_org_repository)
    super(release_date, version, previous_version, mroonga_org_repository)
    @output_file_path = "./tmp/twitter-ja-#{release_date}-mroonga-#{version}-base.txt"
  end

  def generate_article
    "Mroonga #{@version}をリリースしました！(#{@release_date}) " +
    "https://mroonga.org/ja/blog/#{@release_date_in_link}/mroonga-#{@version}.html"
  end
end

class MySQLMailingListJaArticleGenerator < MroongaArticleGenerator
  def initialize(release_date, version, previous_version, mroonga_org_repository)
    super(release_date, version, previous_version, mroonga_org_repository)
    @input_file_path = "./locale/ja/text/news/#{version.split(".")[0]}.txt"
    @output_file_path = "./tmp/mysql-mailinglist-ja-#{release_date}-mroonga-#{version}.txt"
    @release_headline_regexp_pattern = /^.*リリース.+\n=.+/
  end
  
  def generate
    FileUtils.mkdir_p("tmp")
    super
  end

  def generate_article
    <<-ARTICLE
こんにちは。

リリースアナウンス:
  https://mroonga.org/ja/blog/#{@release_date_in_link}/mroonga-#{@version_in_link}.html
    
MroongaはMySQLで日本語全文検索を実現するストレージエンジンです。高速で
あることや位置情報検索をサポートしていることなどが特徴です。詳細につい
はドキュメントをご覧ください。
    
  * Mroongaの特徴 ― Mroonga v#{@version} documentation
    https://mroonga.org/ja/docs/characteristic.html#what-is-mroonga

主な変更内容
============

主な変更点は以下の通りです。

https://mroonga.org/ja/docs/news.html#release-#{@version}

#{extract_latest_release_note}

以上です！ 
    ARTICLE
  end
end

generator_classes = [
  BlogEnArticleGenerator,
  BlogJaArticleGenerator,
  DiscussionsEnArticleGenerator,
  DiscussionsJaArticleGenerator,
  FacebookEnArticleGenerator,
  FacebookJaArticleGenerator,
  TwitterEnArticleBaseGenerator,
  TwitterJaArticleBaseGenerator,
  MySQLMailingListJaArticleGenerator
]

generator_classes.each do |generator_class|
  generator = generator_class.new(option[:release_date],
                                  option[:version], 
                                  option[:previous_version],
                                  option[:mroonga_org_repository])
  generator.generate
  puts generator.output_file_path
end

