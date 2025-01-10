#!/bin/bash

set -exu

package="$1"

os_version=$(cut -d: -f5 /etc/system-release-cpe)
mariadb_version=$(echo "${package}" | cut -d'-' -f2)

cat <<REPO | sudo tee /etc/yum.repos.d/MariaDB.repo
[mariadb]
name = MariaDB
baseurl = "https://rpm.mariadb.org/${mariadb_version}/rhel/\$releasever/\$basearch"
gpgkey = https://rpm.mariadb.org/RPM-GPG-KEY-MariaDB
gpgcheck = 1
REPO

case "${os_version}" in
  8)
    DNF_INSTALL="dnf install -y --enablerepo=powertools"
    sudo dnf module -y disable mysql
    ;;
  *)
    DNF_INSTALL="dnf install -y"
    sudo ${DNF_INSTALL} "https://apache.jfrog.io/artifactory/arrow/almalinux/${os_version}/apache-arrow-release-latest.rpm"
    ;;
esac

sudo ${DNF_INSTALL} "https://packages.groonga.org/almalinux/${os_version}/groonga-release-latest.noarch.rpm"
sudo dnf module -y disable mariadb
sudo ${DNF_INSTALL} mariadb-server
sudo systemctl start mariadb
sudo ${DNF_INSTALL} "${package}"
sudo mysql -e "SHOW ENGINES" | grep Mroonga
