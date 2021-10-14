require "English"
require "json"
require "pathname"
require "pp"
require "zlib"
require "octokit"

groonga_repository = ENV["GROONGA_REPOSITORY"]
if groonga_repository.nil?
  raise "Specify GROONGA_REPOSITORY environment variable"
end
require "#{groonga_repository}/packages/packages-groonga-org-package-task"

class MroongaPackageTask < PackagesGroongaOrgPackageTask
  def initialize(mysql_package)
    @mysql_package = mysql_package
    super("#{@mysql_package}-mroonga", detect_version, detect_release_time)
    @original_archive_base_name = "mroonga-#{@version}"
    @original_archive_name = "#{@original_archive_base_name}.tar.gz"
    @target_branch_name = ENV["TARGET_BRANCH_NAME"] || nil
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
    ENV["VERSION"] || (top_directory + "version_full").read.chomp
  end

  def detect_mysql_version(distribution, code_name)
    __send__("detect_mysql_version_#{distribution}", code_name)
  end

  def detect_mysql_version_debian_oracle(code_name)
    repository_name = @mysql_package.gsub(/-community/, "")
    sources_gz_url =
      "https://repo.mysql.com/apt/debian/dists/#{code_name}/" +
      "#{repository_name}/source/Sources.gz"
    URI.open(sources_gz_url) do |response|
      reader = Zlib::GzipReader.new(response)
      sources = reader.read
      sources.each_line do |line|
        case line.chomp
        when /\AVersion: /
          return $POSTMATCH
        end
      end
      message = "No version information: "
      message << "<#{@mysql_package}>: <#{code_name}>:\n"
      message << sources
      raise message
    end
  end

  def detect_mysql_version_debian_debian(code_name)
    package_info_url =
      "https://sources.debian.org/api/src/#{@mysql_package}/"
    URI.open(package_info_url) do |response|
      info = JSON.parse(response.read)
      info["versions"].each do |version|
        next unless version["suites"].include?(code_name)
        return version["version"]
      end
      message = "No version information: "
      message << "<#{@mysql_package}>: <#{code_name}>: "
      message << PP.pp(info, "")
      raise message
    end
  end

  def detect_mysql_version_debian(code_name)
    if @mysql_package.start_with?("mysql-community-")
      detect_mysql_version_debian_oracle(code_name)
    else
      detect_mysql_version_debian_debian(code_name)
    end
  end

  def detect_ubuntu_package_version(code_name, package_name)
    @ubuntu_package_versions ||= {}
    cache_key = [code_name, package_name]
    version = @ubuntu_package_versions[cache_key]
    return version if version
    # "mariadb-10.5" -> "mariadb-server-10.5"
    # "mysql-5.7" -> "mysql-server-5.7"
    ubuntu_package_name = package_name.sub(/-/, "-server-")
    source_names = [code_name, "#{code_name}-updates"]
    source_names.each do |source_name|
      all_packages_url =
        "https://packages.ubuntu.com/#{source_name}/allpackages?format=txt.gz"
      URI.open(all_packages_url) do |all_packages|
        all_packages.each_line do |line|
          case line
          when /\A#{Regexp.escape(ubuntu_package_name)} \((.+?)[\s)]/o
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

  def enable_windows?
    false
  end

  def use_built_package?
    true
  end

  def get_package_url_for_branch(target_branch_name, target_package_name)
    client = Octokit::Client.new
    client.access_token = ENV["GITHUB_ACCESS_TOKEN"]
    artifacts_response = nil
    workflow_runs_response = client.workflow_runs("mroonga/mroonga",
                                                  "linux.yml",
                                                  branch: target_branch_name)
    url = ""
    workflow_runs_response.workflow_runs.each do |workflow_run|
      artifacts_response =
        client.get("/repos/mroonga/mroonga/actions/runs/#{workflow_run.id}/artifacts")
      next if artifacts_response.total_count.zero?

      artifacts_response.artifacts.each do |artifact|
        if artifact.name == target_package_name
          url << artifact.archive_download_url
        end
      end
      break
    end
    url
  end

  def built_package_url(target_namespace, target)
    url = ""
    if @target_branch_name
      target_package_name =
        "packages-#{@package.gsub(/-mroonga/, "")}-#{target}"
      url = get_package_url_for_branch(@target_branch_name, target_package_name)
    else
      url = "https://github.com/mroonga/mroonga/releases/download/v#{@version}/"
      url << "#{@package}-#{target}.tar.gz"
    end
    url
  end

  def built_package_n_split_components
    1
  end

  def download(url, download_dir)
    if @target_branch_name
      client = Octokit::Client.new
      client.access_token = ENV["GITHUB_ACCESS_TOKEN"]
      artifacts_response = nil
      File.open("#{download_dir}/packages-#{@package.gsub(/-mroonga/, "")}.zip", "wb") do |output|
        output.print(client.get(url))
      end
      File.expand_path("#{download_dir}/packages-#{@package.gsub(/-mroonga/, "")}.zip")
    else
      super(url, download_dir)
    end
  end

  def download_packages(target_namespace)
    if @target_branch_name
      base_dir = __send__("#{target_namespace}_dir")
      repositories_dir = download_repositories_dir(target_namespace)
      mkdir_p(repositories_dir)
      download_dir = "#{base_dir}/tmp/downloads/#{@version}"
      mkdir_p(download_dir)
      __send__("#{target_namespace}_targets").each do |target|
        url = built_package_url(target_namespace, target)
        archive = download(url, download_dir)
        case target_namespace
        when :apt, :yum
          cd(repositories_dir) do
            sh("unzip", archive)
          end
        end
      end
    else
      super(target_namespace)
    end
  end
end
