#!/usr/bin/env ruby

require 'open-uri'
require 'nokogiri/class_resolver'
require 'nokogiri'

target_database_names = [
  "mysql-5.7",
  "mysql-8.0",
#  "mariadb-10.11",
#  "mariadb-10.10",
#  "mariadb-10.9",
#  "mariadb-10.8",
#  "mariadb-10.7",
#  "mariadb-10.6",
#  "mariadb-10.5",
#  "mariadb-10.4",
#  "percona-server-5.7",
#  "percona-server-8.0",
]

if ARGV.size < 1
  puts("Usage: #{$0} TARGET_DATABASE_NAME")
  puts("Target databases list:: ")
  target_database_names.each do |target_database_name|
    puts(target_database_name)
  end
  exit(false)
end


def local_latest_version_mysql(target)
  target_package_name = "mysql-community-#{target}-mroonga"
  File.open("#{__dir__}/../packages/#{target_package_name}/yum/#{target_package_name}.spec.in",
            "r") do |file|
    Gem::Version.new(file.read.slice(/mysql_version_default (\d+\.\d+\.\d+)/, 1))
  end
end

def latest_version(release_versions)
  latest_version = Gem::Version.new("0.0.0")
  release_versions.each do |release_version|
    next if Gem::Version.new(release_version) <= latest_version
    latest_version = Gem::Version.new(release_version)
  end
  latest_version
end

def upstream_latest_version_mysql(target)
  uri = "https://dev.mysql.com/doc/relnotes/mysql/5.7/en/" if target == "5.7"
  uri = "https://dev.mysql.com/doc/relnotes/mysql/8.0/en/" if target == "8.0"

  release_note_html = URI.open(uri).read
  release_note_doc = Nokogiri::HTML(release_note_html)
  release_versions_raw = Array.new()
  release_note_doc.xpath("//*[contains(text(), 'Changes in MySQL')]/text()").each do |release_version|
    next if release_version.to_s.include?("Not yet released, General Availability")
    release_versions_raw << release_version
  end
  release_version_list = release_versions_raw.to_s.scan(/\d+\.\d+\.\d+/).uniq
  latest_version(release_version_list)
end

target_database = ARGV.shift
case target_database
when "mysql-5.7"
  if local_latest_version_mysql("5.7") < upstream_latest_version_mysql("5.7")
    exit 1
  end
when "mysql-8.0"
  if local_latest_version_mysql("8.0") < upstream_latest_version_mysql("8.0")
    exit 1
  end
end

