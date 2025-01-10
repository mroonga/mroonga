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
  9)
    DNF="dnf"
    sudo "${DNF}" install -y "https://apache.jfrog.io/artifactory/arrow/almalinux/${os_version}/apache-arrow-release-latest.rpm"
    ;;
  *)
    DNF="dnf --enablerepo=powertools"
    sudo "${DNF}" module -y disable mysql
    ;;
esac

sudo "${DNF}" install -y "https://packages.groonga.org/almalinux/${os_version}/groonga-release-latest.noarch.rpm"
sudo "${DNF}" module -y disable mariadb
sudo "${DNF}" install -y --enablerepo=powertools mariadb-server
sudo systemctl start mariadb
sudo "${DNF}" install -y --enablerepo=powertools "${package}"
sudo mysql -e "SHOW ENGINES" | grep Mroonga
