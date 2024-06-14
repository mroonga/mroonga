# -*- ruby -*-
#
# Copyright(C) 2024  Horimoto Yasuhiro <horimoto@clear-code.com>
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
  major_in_hex = new_version_major.to_i(16)
  minor_micro_in_hex = new_version.split(".")[1].to_i(16)
  '0x' + major_in_hex.to_s + minor_micro_in_hex.to_s
end

def new_plugin_version
  minor_micro = new_version.split('.')[1].to_i(10)
  "#{new_version_major}" + '.' + minor_micro.to_s
end

namespace :release do
  namespace :version do
    desc "Update versions for a new release"
    task :update do
      new_release_date = env_var("NEW_RELEASE_DATE")
      cd("packages") do
        ruby("-S",
             "rake",
             "version:update")
      end
      sh("git",
         "add",
         *Dir.glob("packages/**/*.spec.in"),
         *Dir.glob("packages/**/changelog"))
      sh("git",
         "commit",
         "-m",
         "doc package: update version info to #{version} (#{new_release_date})")
    end
  end
end

namespace :dev do
  namespace :version do
    desc "Bump version for new development"
    task :bump do
      File.write("plugin_version", "#{new_plugin_version}")
      File.write("version_full", env_var("NEW_VERSION"))
      File.write("version_major", "#{new_version_major}")
      File.write("version_minor", "#{new_version_minor}")
      File.write("version_micro", "#{new_version_micro}")
    end
  end
end
