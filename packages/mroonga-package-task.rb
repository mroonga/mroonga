require "json"
require "pathname"
require "pp"

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
    define_debian_control_tasks
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
        if File.exist?(@original_archive_name)
          cp(@original_archive_name, archive_name)
        else
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
  end

  def debian_remove_versionless_control_entry?(distribution, code_name)
    return false unless distribution == "ubuntu"
    versionless_mysql_package = @mysql_package.gsub(/\-[\d.]+\z/, "")
    detect_ubuntu_package_version(code_name, @mysql_package) !=
      detect_ubuntu_package_version(code_name, @mysql_package)
  end

  def define_debian_control_task(target)
    debian_directory = package_directory + "debian"
    control_in_path = debian_directory + "control.in"
    distribution, code_name, _architecture = target.split("-", 3)
    target_debian_directory = package_directory + "debian.#{target}"
    control_path = target_debian_directory + "control"
    file control_path.to_s => control_in_path.to_s do |task|
      control_in_content = control_in_path.read
      control_content =
        control_in_content
          .gsub(/@REQUIRED_GROONGA_VERSION@/,
                detect_required_groonga_version)
          .gsub(/@MYSQL_VERSION@/,
                detect_mysql_version(distribution, code_name))
      rm_rf(target_debian_directory)
      cp_r(debian_directory, target_debian_directory)
      if debian_remove_versionless_control_entry?(distribution, code_name)
        in_mysql_server_mroonga = false
        replaced_control_content = ""
        control_content.each_line do |line|
          case line.chomp
          when ""
            if in_mysql_server_mroonga
              in_mysql_server_mroonga = false
            else
              replaced_control_content.print(line)
            end
          when "Package: mysql-server-mroonga"
            in_mysql_server_mroonga = true
          else
            next if in_mysql_server_mroonga
            replaced_control_content.print(line)
          end
        end
        control_content = replaced_control_content
      end
      control_path.open("w") do |file|
        file.puts(control_content)
      end
    end
    control_path.to_s
  end

  def define_debian_control_tasks
    apt_control_paths = []
    apt_targets.each do |target|
      apt_control_paths << define_debian_control_task(target)
    end
    namespace :apt do
      task :build => apt_control_paths
    end

    namespace :ubuntu do
      namespace :upload do
        ubuntu_targets.each do |code_name, version|
          control_path = define_debian_control_task("ubuntu-#{code_name}")
          task code_name => control_path
        end
      end
    end
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
end
