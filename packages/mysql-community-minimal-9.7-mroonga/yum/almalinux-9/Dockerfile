ARG FROM=almalinux:9
FROM ${FROM}

ARG DEBUG

ENV \
  SCL=gcc-toolset-12

RUN \
  quiet=$([ "${DEBUG}" = "yes" ] || echo "--quiet") && \
  dnf update -y ${quiet} && \
  { \
    echo "[mysql-9.7-lts-community-minimal-source]"; \
    echo "name=MySQL 9.7 LTS Server Minimal - Source"; \
    echo "baseurl=https://repo.mysql.com/yum/mysql-9.7-community/docker/el/9/SRPMS/"; \
    echo "gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY-mysql-2023"; \
    echo "enabled=1"; \
    echo "gpgcheck = 1"; \
  } | tee /etc/yum.repos.d/MySQL-9.7-minimal-source.repo && \
  dnf install --enablerepo=crb -y ${quiet} \
    'dnf-command(builddep)' \
    'dnf-command(download)' \
    https://packages.groonga.org/almalinux/9/groonga-release-latest.noarch.rpm \
    https://repo.mysql.com/mysql97-community-release-el9.rpm \
    https://packages.apache.org/artifactory/arrow/almalinux/9/apache-arrow-release-latest.rpm && \
  dnf install -y ${quiet} \
    mysql-community-devel && \
  dnf install --enablerepo=crb -y ${quiet} \
    https://repo.mysql.com/mysql-community-minimal-release-el9.rpm && \
  dnf builddep --enablerepo=mysql-9.7-lts-community-minimal-source,crb -y ${quiet} mysql-community-minimal && \
  dnf download --enablerepo=mysql-9.7-lts-community-minimal-source -y ${quiet} --source mysql-community-minimal && \
  dnf install --enablerepo=crb -y ${quiet} \
    ${SCL} \
    ${SCL}-annobin-plugin-gcc \
    ccache \
    groonga-devel \
    groonga-normalizer-mysql-devel \
    ninja-build \
    pkgconfig \
    rpm-build \
    wget \
    which && \
  dnf clean ${quiet} all
