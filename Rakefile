# -*- ruby -*-
#
# Copyright (C) 2024  Horimoto Yasuhiro <horimoto@clear-code.com>
# Copyright (C) 2024  Sutou Kouhei <kou@clear-code.com>
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
require "tmpdir"

def env_var(name, default=nil)
  value = ENV[name] || default
  raise "${#{name}} is missing" if value.nil?
  value
end

def version
  env_var("VERSION", File.read("version_full"))
end

def new_version
  env_var("NEW_VERSION")
end

def new_version_major
  new_version.split(".")[0]
end

def new_version_minor
  new_version.split(".")[1][0]
end

def new_version_micro
  new_version.split(".")[1][1]
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
    desc "Update versions for a new release"
    task :update do
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
  task :tag do
    sh("git",
       "tag",
       "v#{version}",
       "-a",
       "-m",
       "Mroonga #{version}!!!")
    sh("git", "push", "origin", "v#{version}")
  end
end

namespace :dev do
  namespace :version do
    desc "Bump version for new development"
    task :bump do
      File.write("plugin_version", new_plugin_version)
      File.write("version_full", new_version)
      File.write("version_in_hex", new_version_in_hex)
      File.write("version_major", new_version_major)
      File.write("version_minor", new_version_minor)
      File.write("version_micro", new_version_micro)
    end
  end
end

desc "Lint"
task :lint do
  cd "packages" do
    ruby("-S", "rake", "version:lint")
  end
end
