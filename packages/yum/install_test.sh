#!/bin/bash

set -exu

package="$1"

os_version=$(cut -d: -f5 /etc/system-release-cpe)
mariadb_version=$(echo "${package}" | cut -d'-' -f2)

cat > /etc/yum.repos.d/MariaDB.repo <<EOF
[mariadb]
name = MariaDB
baseurl = "http://yum.mariadb.org/${mariadb_version}/centos${os_version}-amd64"
gpgkey=https://yum.mariadb.org/RPM-GPG-KEY-MariaDB
gpgcheck=1
EOF

sudo dnf install -y "https://packages.groonga.org/almalinux/${os_version}/groonga-release-latest.noarch.rpm"
sudo dnf module -y disable mariadb
sudo dnf module -y disable mysql
sudo dnf install -y --enablerepo=powertools mariadb-server
sudo systemctl start mariadb
sudo dnf install -y --enablerepo=powertools "${package}"
sudo mysql -e "SHOW ENGINES" | grep Mroonga
