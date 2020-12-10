require "json"
require "pathname"
require "pp"

begin
  require "octokit"
  require "veyor"
rescue LoadError
  warn("require octokit and veyor for uploading Windows packages: #{$!.message}")
end

groonga_repository = ENV["GROONGA_REPOSITORY"]
if groonga_repository.nil?
  raise "Specify GROONGA_REPOSITORY environment variable"
end
require "#{groonga_repository}/packages/packages-groonga-org-package-task"

class MroongaPackageTask < PackagesGroongaOrgPackageTask
  def initialize(mysql_package, rpm_package)
    @mysql_package = mysql_package
    super("#{@mysql_package}-mroonga", detect_version, detect_release_time)
    @rpm_package = rpm_package
    @original_archive_base_name = "mroonga-#{@version}"
    @original_archive_name = "#{@original_archive_base_name}.tar.gz"
  end

  def define
    super
    define_windows_upload_task
  end

  private
  def top_directory
    packages_directory.parent
  end

  def packages_directory
    Pathname(__dir__)
  end

  def package_directory
    packages_directory + @package
  end

  def detect_version
    ENV["VERSION"] || (top_directory + "version").read.chomp
  end

  def detect_mysql_version(distribution, code_name)
    __send__("detect_mysql_version_#{distribution}", code_name)
  end

  def detect_mysql_version_debian(code_name)
    mysql_source_package = @mysql_package.gsub(/-server/, "")
    package_info_url =
      "https://sources.debian.org/api/src/#{mysql_source_package}/"
    URI.open(package_info_url) do |response|
      info = JSON.parse(response.read)
      info["versions"].each do |version|
        next unless version["suites"].include?(code_name)
        return version["version"]
      end
      message = "No version information: "
      message << "<#{mysql_source_package}>: <#{code_name}>: "
      message << PP.pp(info, "")
      raise message
    end
  end

  def detect_ubuntu_package_version(code_name, package_name)
    @ubuntu_package_versions ||= {}
    cache_key = [code_name, package_name]
    version = @ubuntu_package_versions[cache_key]
    return version if version
    source_names = [code_name, "#{code_name}-updates"]
    source_names.each do |source_name|
      all_packages_url =
        "https://packages.ubuntu.com/#{source_name}/allpackages?format=txt.gz"
      URI.open(all_packages_url) do |all_packages|
        all_packages.each_line do |line|
          case line
          when /\A#{Regexp.escape(package_name)} \((.+?)[\s)]/o
            version = $1
          end
        end
      end
    end
    if version.nil?
      raise "No version information: <#{package_name}>: <#{code_name}>"
    end
    @ubuntu_package_versions[cache_key] = version
    version
  end

  def detect_mysql_version_ubuntu(code_name)
    detect_ubuntu_package_version(code_name, @mysql_package)
  end

  def detect_required_groonga_version
    (top_directory + "required_groonga_version").read.chomp
  end

  def original_archive_path
    top_directory + @original_archive_name
  end

  def define_archive_task
    [@archive_name, deb_archive_name, rpm_archive_name].each do |archive_name|
      file archive_name => original_archive_path.to_s do
        sh("tar", "xf", original_archive_path.to_s)
        archive_base_name = File.basename(archive_name, ".tar.gz")
        if @original_archive_base_name != archive_base_name
          mv(@original_archive_base_name, archive_base_name)
        end
        sh("tar", "czf", archive_name, archive_base_name)
        rm_r(archive_base_name)
      end
    end
  end

  def debian_remove_versionless_control_entry?(distribution, code_name)
    return false unless distribution == "ubuntu"
    versionless_mysql_package = @mysql_package.gsub(/\-[\d.]+\z/, "")
    detect_ubuntu_package_version(code_name, @mysql_package) !=
      detect_ubuntu_package_version(code_name, @mysql_package)
  end

  def apt_prepare_debian_control(control_in, target)
    distribution, code_name, architecture = target.split("-", 3)
    control =
      control_in
        .gsub(/@REQUIRED_GROONGA_VERSION@/,
              detect_required_groonga_version)
        .gsub(/@MYSQL_VERSION@/,
              detect_mysql_version(distribution, code_name))
    if debian_remove_versionless_control_entry?(distribution, code_name)
      in_mysql_server_mroonga = false
      replaced_control = ""
      control.each_line do |line|
        case line.chomp
        when ""
          if in_mysql_server_mroonga
            in_mysql_server_mroonga = false
          else
            replaced_control.print(line)
          end
        when "Package: mysql-server-mroonga"
          in_mysql_server_mroonga = true
        else
          next if in_mysql_server_mroonga
          replaced_control.print(line)
        end
      end
      control = replaced_control
    end
    control
  end

  def rpm_archive_name
    @original_archive_name
  end

  def yum_expand_variable(key)
    case key
    when "REQUIRED_GROONGA_VERSION"
      detect_required_groonga_version
    else
      super
    end
  end

  def define_windows_upload_task
    namespace :windows do
      desc "Upload packages"
      task :upload do
        mroonga_repository = "mroonga/mroonga"
        tag_name = "v#{@version}"

        github_token = env_value("GITHUB_TOKEN")
        client = Octokit::Client.new(:access_token => github_token)
        client.auto_paginate = true

        appveyor_url = "https://ci.appveyor.com/"
        appveyor_info = nil
        client.statuses(mroonga_repository, tag_name).each do |status|
          next unless status.target_url.start_with?(appveyor_url)
          case status.state
          when "success"
            match_data = /\/([^\/]+?)\/([^\/]+?)\/builds\/(\d+)\z/.match(status.target_url)
            appveyor_info = {
              account: match_data[1],
              project: match_data[2],
              build_id: match_data[3],
            }
            break
          when "pending"
            # Ignore
          else
            message = "Appveyor build isn't succeesed: #{status.state}\n"
            message << " #{status.target_url}"
            raise message
          end
        end
        if appveyor_info.nil?
          raise "No Appveyor build"
        end

        releases = client.releases(mroonga_repository)
        current_release = releases.find do |release|
          release.tag_name == tag_name
        end
        current_release ||= client.create_release(mroonga_repository, tag_name)

        start_build = appveyor_info[:build_id].to_i + 1
        build_history = Veyor.project_history(account: appveyor_info[:account],
                                              project: appveyor_info[:project],
                                              start_build: start_build,
                                              limit: 1)
        build_version = build_history["builds"][0]["buildNumber"]
        project = Veyor.project(account: appveyor_info[:account],
                                project: appveyor_info[:project],
                                version: build_version)
        project["build"]["jobs"].each do |job|
          job_id = job["jobId"]
          artifacts = Veyor.build_artifacts(job_id: job_id)
          artifacts.each do |artifact|
            file_name = artifact["fileName"]
            url = "#{appveyor_url}api/buildjobs/#{job_id}/artifacts/#{file_name}"
            sh("curl", "--location", "--output", file_name, url)
            options = {
              :content_type => "application/zip",
            }
            client.upload_asset(current_release.url, file_name, options)
            rm(file_name)
          end
        end
      end
    end
  end
end
