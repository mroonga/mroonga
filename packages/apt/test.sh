#!/bin/bash

set -exu

package=$1

echo "::group::Prepare repository"

mysql_version=$(echo "${package}" | grep -o '[0-9]*\.[0-9]*')

echo "debconf debconf/frontend select Noninteractive" | \
  sudo debconf-set-selections

sudo sed -i'' -e 's/in\.archive/archive/g' /etc/apt/sources.list

sudo apt update
sudo apt purge -V -y grub-pc
sudo apt install -V -y apparmor lsb-release wget

distribution=$(lsb_release --id --short | tr 'A-Z' 'a-z')
case ${distribution} in
  debian)
    repository=main
    ;;
  ubuntu)
    repository=universe
    sudo apt -y install software-properties-common
    sudo add-apt-repository -y universe
    sudo add-apt-repository -y ppa:groonga/ppa
    ;;
esac
code_name=$(lsb_release --codename --short)
architecture=$(dpkg --print-architecture)

wget \
  https://packages.groonga.org/${distribution}/groonga-apt-source-latest-${code_name}.deb
sudo apt install -V -y ./groonga-apt-source-latest-${code_name}.deb

case ${distribution}-${code_name} in
  ubuntu-*)
    :
    ;;
  *)
    wget \
      https://apache.jfrog.io/artifactory/arrow/$(lsb_release --id --short | tr 'A-Z' 'a-z')/apache-arrow-apt-source-latest-$(lsb_release --codename --short).deb
    sudo apt install -y -V ./apache-arrow-apt-source-latest-$(lsb_release --codename --short).deb
    rm apache-arrow-apt-source-latest-$(lsb_release --codename --short).deb
    ;;
esac

sudo apt update

echo "::endgroup::"


echo "::group::Prepare local repository"

(echo "Key-Type: RSA"; \
 echo "Key-Length: 4096"; \
 echo "Name-Real: Test"; \
 echo "Name-Email: test@example.com"; \
 echo "%no-protection") | \
  gpg --full-generate-key --batch
GPG_KEY_ID=$(gpg --list-keys --with-colon test@example.com | grep fpr | cut -d: -f10)
gpg --export --armor test@example.com > keys
sudo gpg \
  --no-default-keyring \
  --keyring /usr/share/keyrings/${package}.gpg \
  --import keys

sudo apt install -V -y reprepro
repositories_dir=/host/packages/${package}/apt/repositories
pushd /tmp/
mkdir -p conf/
cat <<DISTRIBUTIONS > conf/distributions
Codename: ${code_name}
Components: main
Architectures: amd64 source
SignWith: ${GPG_KEY_ID}
DISTRIBUTIONS
reprepro includedeb ${code_name} \
  ${repositories_dir}/${distribution}/pool/${code_name}/${repository}/*/*/*_{${architecture},all}.deb
cat <<APT_SOURCES | sudo tee /etc/apt/sources.list.d/${package}.list
deb [signed-by=/usr/share/keyrings/${package}.gpg] file://${PWD} ${code_name} main
APT_SOURCES
popd

sudo apt update

echo "::endgroup::"


echo "::group::Install package"

mysql_community_install_mysql_apt_config() {
  # We need to specify DEBIAN_FRONTEND=noninteractive explicitly
  # because the mysql-apt-config's config script refers the
  # DEBIAN_FRONTEND environment variable directly. :<
  local mysql_server_version="mysql-${mysql_version}"
  case "${mysql_version}" in
    8.0)
      :
      ;;
    *)
      mysql_server_version+="-lts"
      ;;
  esac
  sudo \
    env DEBIAN_FRONTEND=noninteractive \
        MYSQL_SERVER_VERSION=${mysql_server_version} \
      apt install -V -y ./mysql-apt-config.deb
  sudo apt update
}

case ${package} in
  mariadb-*)
    old_package=${package}
    case "${distribution}-${code_name}" in
      # TODO: Remove debian-bookworm after we release a new
      # version. We can't do upgrade test because the previous Mroonga
      # requires old MariaDB that isn't provided by Debian.
      #
      # TODO: Remove ubuntu-noble after we release a package for
      # ubuntu-noble on pckages.groonga.org.
      debian-bookworm|ubuntu-noble)
        old_package=
        ;;
    esac
    mysql_package_prefix=mariadb
    client_dev_package=libmariadb-dev
    test_package=mariadb-test
    mysql_test_dir=/usr/share/mysql/mysql-test
    ;;
  mysql-community-*)
    old_package=${package}
    case "${distribution}-${code_name}" in
      # TODO: Remove debian-bookworm after we release a package for
      # debian-bookworm on pckages.groonga.org.
      #
      # TODO: Remove ubuntu-jammy after we release a package for
      # ubuntu-jammy on pckages.groonga.org.
      #
      # TODO: Remove ubuntu-noble after we release a package for
      # ubuntu-noble on pckages.groonga.org.
      debian-bookworm|ubuntu-jammy|ubuntu-noble)
        old_package=
        ;;
    esac
    wget https://repo.mysql.com/mysql-apt-config.deb
    mysql_community_install_mysql_apt_config
    mysql_package_prefix=mysql
    client_dev_package=libmysqlclient-dev
    test_package=mysql-testsuite
    mysql_test_dir=/usr/lib/mysql-test
    ;;
  mysql-*)
    old_package=${package}
    mysql_package_prefix=mysql
    client_dev_package=libmysqlclient-dev
    test_package=mysql-testsuite
    mysql_test_dir=/usr/lib/mysql-test
    ;;
esac

sudo apt install -V -y ${package}

sudo mysql -e "SHOW ENGINES" | grep Mroonga

echo "::endgroup::"


echo "::group::Prepare test"

sudo apt install -V -y \
  gdb \
  ${client_dev_package} \
  ${test_package} \
  patch

pushd ${mysql_test_dir}
sudo rm -rf plugin/mroonga
sudo mkdir -p plugin
sudo cp -a /host/mysql-test/mroonga/ plugin/

case ${package} in
  mysql-8.*|mysql-community-8.*)
    ( \
      echo "/usr/lib/mysql-test/var/ r,"; \
      echo "/usr/lib/mysql-test/var/** rwk,"; \
    ) | sudo tee --append /etc/apparmor.d/local/usr.sbin.mysqld
    if [ -f /etc/apparmor.d/usr.sbin.mysqld ]; then
      sudo apparmor_parser --replace --skip-cache /etc/apparmor.d/usr.sbin.mysqld
    fi
    ;;
esac

parallel=$(nproc)
case ${package} in
  mysql-8.*|mysql-community-8.*|percona-server-8.*)
    parallel=1
    ;;
esac

test_suite_names=""
set +x
for test_suite_name in $(find plugin/mroonga -type d '!' -name '[tr]'); do
  if [ -n "${test_suite_names}" ]; then
    test_suite_names="${test_suite_names},"
  fi
  test_suite_names="${test_suite_names}${test_suite_name}"
done
set -x

mtr_args=()
mtr_args+=(--force)
case ${package} in
  mysql-8.*|mysql-community-8.*|percona-server-8.*)
    mtr_args+=(--mysqld=--loose-plugin-load-add=ha_mroonga.so)
    mtr_args+=(--mysqld=--loose-plugin-mroonga=ON)
    ;;
esac
mtr_args+=(--no-check-testcases)
mtr_args+=(--parallel=${parallel})
mtr_args+=(--retry=3)
mtr_args+=(--suite="${test_suite_names}")
if [ -d "${mysql_test_dir}/bin" ]; then
  mtr_args+=(--client-bindir="${mysql_test_dir}/bin")
fi

echo "::endgroup::"


echo "::group::Run test"
sudo ./mtr "${mtr_args[@]}"
echo "::endgroup::"


echo "::group::Run test with prepared statement protocol"
case ${package} in
  mariadb-*)
    # Test with prepared statement protocol
    sudo ./mtr "${mtr_args[@]}" --ps-protocol
    ;;
esac
popd
echo "::endgroup::"


echo "::group::Upgrade test"
sudo apt purge -V -y \
  ${package} \
  "${mysql_package_prefix}-*"
sudo rm -rf /var/lib/mysql

sudo mv /etc/apt/sources.list.d/${package}.list /tmp/
case ${package} in
  mysql-community-8.*)
    mysql_community_install_mysql_apt_config
    ;;
esac

sudo apt update
if apt show ${old_package} > /dev/null 2>&1; then
  sudo apt install -V -y ${old_package}
  sudo mv /tmp/${package}.list /etc/apt/sources.list.d/
  sudo apt update
  sudo apt upgrade -V -y
  sudo mysql -e "SHOW ENGINES" | grep Mroonga
else
  echo "Skip because ${old_package} hasn't been released yet."
fi
echo "::endgroup::"
