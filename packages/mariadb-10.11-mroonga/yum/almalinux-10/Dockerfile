ARG FROM=almalinux:10
FROM ${FROM}

ARG DEBUG

RUN \
  quiet=$([ "${DEBUG}" = "yes" ] || echo "--quiet") && \
  os_version=$(cut -d: -f5 /etc/system-release-cpe | cut -d. -f1) && \
  dnf update -y ${quiet} && \
  { \
    echo "[mariadb]"; \
    echo "name = MariaDB"; \
    echo "baseurl = https://yum.mariadb.org/10.11/rhel10-amd64"; \
    echo "gpgkey = https://yum.mariadb.org/RPM-GPG-KEY-MariaDB"; \
    echo "gpgcheck = 1"; \
  } | tee /etc/yum.repos.d/MariaDB.repo && \
  dnf install -y \
    https://packages.groonga.org/almalinux/${os_version}/groonga-release-latest.noarch.rpm && \
  dnf install -y epel-release && \
  dnf install -y https://packages.apache.org/artifactory/arrow/almalinux/${os_version}/apache-arrow-release-latest.rpm && \
  dnf config-manager --set-enabled epel || : && \
  dnf config-manager --set-enabled crb || : && \
  dnf groupinstall -y ${quiet} "Development Tools" && \
  dnf install --enablerepo=crb -y ${quiet} \
    'dnf-command(builddep)' \
    'dnf-command(download)' && \
  dnf builddep --enablerepo=crb -y ${quiet} MariaDB && \
  dnf download -y ${quiet} --source MariaDB && \
  dnf install --enablerepo=crb -y ${quiet} \
    arrow-devel \
    MariaDB-devel \
    ccache \
    cmake \
    gcc-c++ \
    groonga-devel \
    groonga-normalizer-mysql-devel \
    intltool \
    make \
    pkgconfig \
    sudo \
    wget \
    which && \
  dnf clean ${quiet} all
