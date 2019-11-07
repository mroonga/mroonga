require "json"
require "pathname"

apache_arrow_repository = ENV["APACHE_ARROW_REPOSITORY"]
if apache_arrow_repository.nil?
  raise "Specify APACHE_ARROW_REPOSITORY environment variable"
end
require "#{apache_arrow_repository}/dev/tasks/linux-packages/package-task"

class MroongaPackageTask < PackageTask
  class << self
    def apache_arrow_repository=(repository)
      @@apache_arrow_repository = repository
    end
  end

  def initialize(mysql_package)
    @apache_arrow_repository = Pathname(@@apache_arrow_repository)
    @mysql_package = mysql_package
    super("#{@mysql_package}-mroonga", detect_version, Time.now.utc)
    @original_archive_base_name = "mroonga-#{@version}"
    @original_archive_name = "#{@original_archive_base_name}.tar.gz"
  end

  def define
    super
    define_debian_control_task
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
    package_info_url = "https://sources.debian.org/api/src/#{@mysql_package}/"
    URI.open(package_info_url) do |response|
      info = JSON.parse(response.read)
      info["versions"].each do |version|
        next unless version["suites"].include?(code_name)
        return version["version"]
      end
      message = "No version information: <#{@mysql_package}>: <#{code_name}>: "
      message << PP.pp(info, "")
      raise message
    end
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

  def define_debian_control_task
    control_paths = []
    debian_directory = package_directory + "debian"
    control_in_path = debian_directory + "control.in"
    apt_targets.each do |target|
      distribution, code_name = target.split("-", 2)
      target_debian_directory = package_directory + "debian.#{target}"
      control_path = target_debian_directory + "control"
      control_paths << control_path.to_s
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
        control_path.open("w") do |file|
          file.puts(control_content)
        end
      end
    end
    namespace :apt do
      task :build => control_paths
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

MroongaPackageTask.apache_arrow_repository = apache_arrow_repository
