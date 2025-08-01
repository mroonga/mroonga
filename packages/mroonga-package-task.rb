require "English"
require "json"
require "open-uri"
require "pathname"
require "pp"
require "zlib"

groonga_repository = ENV["GROONGA_REPOSITORY"]
if groonga_repository.nil?
  raise "Specify GROONGA_REPOSITORY environment variable"
end
require "#{groonga_repository}/packages/packages-groonga-org-package-task"

class MroongaPackageTask < PackagesGroongaOrgPackageTask
  def initialize(package_name)
    super(package_name, detect_version, detect_release_time)
    if package_name.end_with?("-mroonga")
      @mysql_package = package_name.gsub(/-mroonga\z/, "")
    else
      @mysql_package = nil
    end
    @original_archive_base_name = "mroonga-#{@version}"
    @original_archive_name = "#{@original_archive_base_name}.tar.gz"
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

  def detect_mysql_version_oracle(distribution, code_name)
    repository_name = @mysql_package.gsub(/-community/, "")
    unless repository_name == "mysql-8.0"
      repository_name += "-lts"
    end
    sources_gz_url =
      "https://repo.mysql.com/apt/#{distribution}/dists/#{code_name}/" +
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
      message << "<#{@mysql_package}>: <#{distribution}>/<#{code_name}>:\n"
      message << sources
      raise message
    end
  end

  def detect_mysql_version_debian_debian(code_name)
    source_package_name = @mysql_package
    if code_name == "bookworm" and @mysql_package.start_with?("mariadb-")
      source_package_name = "mariadb"
    end
    package_info_url =
      "https://sources.debian.org/api/src/#{source_package_name}/"
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
      detect_mysql_version_oracle("debian", code_name)
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
    case code_name
    when "jammy"
    else
      # "mariadb-server-10.11" -> "mariadb-server"
      # New Ubuntu don't include version number in package name.
      ubuntu_package_name = ubuntu_package_name.gsub(/-[\d.]+\z/, "")
    end
    source_names = [code_name, "#{code_name}-updates"]
    source_names.each do |source_name|
      all_packages_url =
        "https://packages.ubuntu.com/#{source_name}/allpackages?format=txt.gz"
      n_remained_tries = 3
      begin
        URI.open(all_packages_url) do |all_packages|
          all_packages.each_line do |line|
            case line
            when /\A#{Regexp.escape(ubuntu_package_name)} \((.+?)[\s)]/o
              version = $1
            end
          end
        end
      rescue OpenURI::HTTPError => error
        raise if error.io.status[0] != "500"
        n_remained_tries -= 1
        raise if n_remained_tries.zero?
        retry
      end
    end
    if version.nil?
      raise "No version information: <#{package_name}>: <#{code_name}>"
    end
    @ubuntu_package_versions[cache_key] = version
    version
  end

  def detect_mysql_version_ubuntu(code_name)
    if @mysql_package.start_with?("mysql-community-")
      detect_mysql_version_oracle("ubuntu", code_name)
    else
      detect_ubuntu_package_version(code_name, @mysql_package)
    end
  end

  def detect_required_groonga_version
    (top_directory + "required_groonga_version").read.chomp
  end

  def original_archive_path
    top_directory + @original_archive_name
  end

  def define_archive_task
    file original_archive_path.to_s do
      source_archive_url = built_package_url(:source, @original_archive_name)
      download(source_archive_url, original_archive_path.to_s)
    end

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
    return false if @mysql_package.start_with?("mysql-community-")
    versionless_mysql_package = @mysql_package.gsub(/\-[\d.]+\z/, "")
    detect_ubuntu_package_version(code_name, @mysql_package) !=
      detect_ubuntu_package_version(code_name, versionless_mysql_package)
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

  def detect_mariadb_release_yum
    "1" # We can detect this when MariaDB starts using other value.
  end

  def detect_mariadb_version_yum
    series = @mysql_package.split("-", 2)[1]
    api_url = "https://downloads.mariadb.org/rest-api/mariadb/#{series}"
    releases = URI.open(api_url) do |response|
      JSON.parse(response.read)["releases"]
    end
    releases.first[0]
  end

  def split_mysql_package
    *names, series = @mysql_package.split("-")
    [names.join("-"), series]
  end

  def target_version(series, trailing_slash: false, minimal: false)
    path = minimal ? "docker/el/9/SRPMS" : "el/9/SRPMS"
    path += "/" if trailing_slash
    srpms_url =
      "https://repo.mysql.com/yum/mysql-#{series}-community/#{path}"
    pattern =  trailing_slash ? "SRPMS\/mysql-community-" : "mysql-community-"
    pattern += "minimal-" if minimal

    index_html = URI.open(srpms_url) do |response|
      response.read
    end
    latest_target_srpm =
      index_html.
        scan(/href="(.+?)"/i).
        flatten.
        grep(/\A#{pattern}/).
        last
    latest_target_srpm[/\A#{pattern}(\d+\.\d+\.\d+-\d+)/, 1]
  end

  def latest_target_version(version1, version2)
    version1 = Gem::Version.new(version1)
    version2 = Gem::Version.new(version2)
    version1 >= version2 ? version1 : version2
  end

  def detect_mysql_community_rpm_version
    series = split_mysql_package[1]
    # Use the URL returns newer version because the CDN caches both “…/SRPMS”
    # and "…/SRPMS/", but one of caches may return old version.
    latest_target_version(
      target_version(series, trailing_slash: true),
      target_version(series, trailing_slash: false)
    )
  end

  def mysql_community_rpm_version
    @mysql_community_rpm_version ||= detect_mysql_community_rpm_version
  end

  def detect_mysql_community_minimal_rpm_version
    series = split_mysql_package[1]
    # Use the URL returns newer version because the CDN caches both “…/SRPMS”
    # and "…/SRPMS/", but one of caches may return old version.
    latest_target_version(
      target_version(series, trailing_slash: true, minimal: true),
      target_version(series, trailing_slash: false, minimal: true)
    )
  end

  def mysql_community_minimal_rpm_version
    @mysql_community_minimal_rpm_version ||=
      detect_mysql_community_minimal_rpm_version
  end

  def detect_percona_server_rpm_version
    series = split_mysql_package[1]
    short_series = series.gsub(".", "")
    if short_series == "84"
      short_series = "84-lts"
    end
    srpms_url =
      "https://repo.percona.com/ps-#{short_series}/yum/release/9/SRPMS"
    index_html = URI.open(srpms_url) do |response|
      response.read
    end
    latest_target_srpm =
      index_html.
        scan(/href="(.+?)"/i).
        flatten.
        grep(/\Apercona-server-/).
        last
    latest_target_srpm[/\Apercona-server-(\d+\.\d+\.\d+-\d+\.\d+)/, 1]
  end

  def percona_server_rpm_version
    @percona_server_rpm_version ||= detect_percona_server_rpm_version
  end

  def detect_mysql_release_yum
    name, series = split_mysql_package
    case name
    when "mysql-community"
      mysql_community_rpm_version.split("-")[1]
    when "mysql-community-minimal"
      mysql_community_minimal_rpm_version.split("-")[1]
    end
  end

  def detect_mysql_version_yum
    name, series = split_mysql_package
    case name
    when "mysql-community"
      mysql_community_rpm_version.split("-")[0]
    when "mysql-community-minimal"
      mysql_community_minimal_rpm_version.split("-")[0]
    when "percona-server"
      percona_server_rpm_version.split("-")[0]
    end
  end

  def detect_percona_server_release_yum
    percona_server_rpm_version[/\.(\d+)\z/, 1]
  end

  def detect_percona_server_version_yum
    percona_server_rpm_version[/-(\d+)\.\d+\z/, 1]
  end

  def yum_expand_variable(key)
    case key
    when "MARIADB_RELEASE"
      detect_mariadb_release_yum
    when "MARIADB_VERSION"
      detect_mariadb_version_yum
    when "MYSQL_RELEASE"
      detect_mysql_release_yum
    when "MYSQL_VERSION"
      detect_mysql_version_yum
    when "PERCONA_SERVER_RELEASE"
      detect_percona_server_release_yum
    when "PERCONA_SERVER_VERSION"
      detect_percona_server_version_yum
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

  def docker_image(os, architecture)
    "ghcr.io/mroonga/package-#{super}"
  end

  def built_package_url(target_namespace, target)
    url = "https://github.com/mroonga/mroonga/releases/download/v#{@version}/"
    if target_namespace == :source
      url << target
    else
      url << "#{@package}-#{target}.tar.gz"
    end
    url
  end

  def built_package_n_split_components
    1
  end

  def github_repository
    "mroonga/mroonga"
  end

  def github_actions_workflow_file_name(target_namespace, target)
    case target_namespace
    when :apt, :yum
      "linux.yml"
    when :windows
      "windows.yml"
    else
      super
    end
  end

  def github_actions_artifact_name(target_namespace, target)
    case target_namespace
    when :apt, :yum
      "packages-#{@mysql_package}-#{target}"
    else
      raise NotImplementedError
    end
  end

  def tag_name
    "v#{@version}"
  end
end
