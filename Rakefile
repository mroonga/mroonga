# -*- ruby -*-
#
# Copyright(C) 2024  Horimoto Yasuhiro <horimoto@clear-code.com>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License version 2.1 as published by the Free Software Foundation.
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
  ENV["VERSION"] || File.read("version_full")
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
