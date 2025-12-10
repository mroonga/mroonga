# -*- ruby -*-
#
# Copyright (C) 2024  Horimoto Yasuhiro <horimoto@clear-code.com>
# Copyright (C) 2024-2025  Sutou Kouhei <kou@clear-code.com>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

require "date"
require "json"
require "open-uri"
require "tmpdir"

VERSION_FULL = File.read("version_full").chomp

def env_var(name, default=nil)
  value = ENV[name] || default
  raise "${#{name}} is missing" if value.nil?
  value
end

def version
  env_var("VERSION", VERSION_FULL)
end

def new_version
  env_var("NEW_VERSION", version.succ)
end

def version_components(version)
  major, minor_micro = version.split(".")
  [major, minor_micro[0], minor_micro[1]]
end

def new_version_major
  version_components(new_version)[0]
end

def new_version_minor
  version_components(new_version)[1]
end

def new_version_micro
  version_components(new_version)[2]
end

def version_major
  version_components(version)[0]
end

def version_minor
  version_components(version)[1]
end

def version_micro
  version_components(version)[2]
end

def latest_groonga_version
  releases_url = "https://api.github.com/repos/groonga/groonga/releases/latest"
  URI.open(releases_url) do |response|
    Gem::Version.new(JSON.parse(response.read)["tag_name"].delete_prefix("v"))
  end
end

def latest_groonga_version_in_mroonga_style
  segments = latest_groonga_version.segments
  "#{segments[0]}.#{segments[1]}#{segments[2]}"
end

def new_version_in_hex
  major, minor_micro = new_version.split(".").collect {|x| Integer(x, 10)}
  "0x%02x%02x" % [major, minor_micro]
end

def new_plugin_version
  # 10.00 -> 10.0
  # 10.01 -> 10.1
  # 10.11 -> 10.11
  new_version.gsub(".0", ".")
end

namespace :release do
  namespace :document do
    desc "Update document"
    task :update do
      mroonga_org_path = File.expand_path(env_var("MROONGA_ORG_DIR"))
      build_dir = ENV["BUILD_DIR"]
      deploy = lambda do |output_dir, locale|
        if locale == "en"
          mroonga_org_docs = File.join(mroonga_org_path, "docs")
        else
          mroonga_org_docs = File.join(mroonga_org_path, locale, "docs")
        end
        rm_rf(mroonga_org_docs)
        mkdir_p(File.expand_path(File.join(mroonga_org_docs, "..")))
        mv(output_dir, mroonga_org_docs)
        rm_f(File.join(mroonga_org_docs, ".buildinfo"))
      end
      Dir.mktmpdir do |dir|
        if build_dir
          sh({"DESTDIR" => dir},
             "cmake",
             "--build", build_dir,
             "--target", "install")
          Dir.glob("#{dir}/**/share/doc/mroonga/*/") do |doc|
            locale = File.basename(doc)
            deploy.call(doc, locale)
          end
        else
          ["en", "ja"].each do |locale|
            output_dir = "#{dir}/html"
            sh({
                 "DOCUMENT_VERSION" => version,
                 "DOCUMENT_VERSION_FULL" => version,
                 "LOCALE" => locale,
                 "PACKAGE_MROONGA_VERSION" => version,
               },
               "sphinx-build",
               "--builder", "html",
               "--define", "language=#{locale}",
               "--doctree-dir", "#{dir}/doctrees/html",
               "--jobs", "auto",
               "--quiet",
               "doc/source",
               output_dir)
            deploy.call(output_dir, locale)
          end
        end
      end
    end
  end

  namespace :version do
    desc "Validate version for release"
    task :validate do
      if version != VERSION_FULL
        message = "You must NOT specify VERSION=#{ENV["VERSION"]} for 'rake release'. "
        message += "You must run 'rake dev:version:bump NEW_VERSION=#{ENV["VERSION"]}' "
        message += "before 'rake release'."
        raise message
      end

      mroonga_version = Gem::Version.new("#{version_major}.#{version_minor}.#{version_micro}")
      if mroonga_version < latest_groonga_version
        message = "Mroonga version (#{VERSION_FULL}) must be greater than or equal to "
        message += "the latest Groonga version (#{latest_groonga_version}). "
        message += "You must run 'rake dev:version:bump NEW_VERSION=#{latest_groonga_version_in_mroonga_style}'."
        raise message
      end
    end

    desc "Update versions for a new release"
    task update: :validate do
      new_release_date =
        ENV["NEW_RELEASE_DATE"] || Date.today.strftime("%Y-%m-%d")
      cd("packages") do
        ruby("-S",
             "rake",
             "version:update")
      end
      sh("git",
         "add",
         *Dir.glob("packages/*/debian/changelog"),
         *Dir.glob("packages/*/yum/*.spec.in"))
      sh("git",
         "commit",
         "-m",
         "package: update version info to #{version} (#{new_release_date})")
      sh("git", "push")
    end
  end

  desc "Tag"
  task tag: "version:validate" do
    sh("git",
       "tag",
       "v#{version}",
       "-a",
       "-m",
       "Mroonga #{version}!!!")
    sh("git", "push", "origin", "v#{version}")
  end
end

desc "Release"
task release: [
  "release:version:validate",
  "release:version:update",
  "release:tag",
  "dev:version:bump"
]

namespace :dev do
  namespace :version do
    desc "Bump version for new development"
    task :bump do
      {
        "plugin_version" => new_plugin_version,
        "version_full" => new_version,
        "version_in_hex" => new_version_in_hex,
        "version_major" => new_version_major,
        "version_minor" => new_version_minor,
        "version_micro" => new_version_micro,
      }.each do |path, value|
        File.write(path, value)
        sh("git", "add", path)
      end
      sh("git", "commit", "-m", "Bump version to #{new_version}")
      sh("git", "push", "origin")
    end
  end
end

desc "Lint"
task :lint do
  cd "packages" do
    ruby("-S", "rake", "version:lint")
  end
end
