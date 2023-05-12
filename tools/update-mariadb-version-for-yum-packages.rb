#!/bin/ruby

require 'date'

def today
  Date.today.strftime(format="%a %b %d %Y")
end

def change_author
  ARGV[0]
end

def mroonga_latest_version
  ARGV[1]
end

def target_mariadb_versions
  ARGV[2..-1]
end

def spec_in_path(mariadb_version)
  mariadb_minor_version = mariadb_version.slice(/[0-9]+\.[0-9]+/)
  "../packages/mariadb-#{mariadb_minor_version}-mroonga/yum/mariadb-#{mariadb_minor_version}-mroonga.spec.in"
end

def current_package_release_number(mariadb_version)
  File.open("#{spec_in_path(mariadb_version)}") do |spec_in|
    spec_in.read.slice(/Release:	([0-9])%\{\?dist\}/,1).to_i
  end
end

def change_logs
  logs = Hash.new([])
  target_mariadb_versions.each do |version|
    next_package_release_number = current_package_release_number(version)+1
    log = <<CHANGE_LOG
* #{today} #{change_author} - #{mroonga_latest_version}-#{next_package_release_number}
- Rebuild against MariaDB #{version}.
CHANGE_LOG
    logs["#{version}"] = log
  end
  logs
end


#DATE=$(LC_TIME="en_US.UTF-8" date +"%a %b %d %Y")
#CHAGE_AUTHOR=$1
#MROONGA_VERSION=$2
#TARGET_MARIADB_VERSION=$3
