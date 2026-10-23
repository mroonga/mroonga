ARG FROM=almalinux:9
FROM ${FROM}

ARG DEBUG

ENV \
  SCL=gcc-toolset-12

RUN \
  quiet=$([ "${DEBUG}" = "yes" ] || echo "--quiet") && \
  os_version=$(cut -d: -f5 /etc/system-release-cpe | cut -d. -f1) && \
  dnf update -y ${quiet} && \
  dnf install --enablerepo=crb -y ${quiet} \
    'dnf-command(builddep)' \
    'dnf-command(download)' \
    https://packages.groonga.org/almalinux/${os_version}/groonga-release-latest.noarch.rpm \
    https://repo.mysql.com/mysql84-community-release-el${os_version}.rpm \
    https://apache.jfrog.io/artifactory/arrow/almalinux/${os_version}/apache-arrow-release-latest.rpm && \
  dnf builddep --enablerepo=crb -y ${quiet} mysql-community && \
  dnf download -y ${quiet} --source mysql-community && \
  dnf install --enablerepo=crb -y ${quiet} \
    ${SCL} \
    ${SCL}-annobin-plugin-gcc \
    ccache \
    cmake \
    groonga-devel \
    groonga-normalizer-mysql-devel \
    intltool \
    libcurl-devel \
    libtool \
    make \
    pkgconfig \
    rpm-build \
    wget \
    which && \
  dnf install -y ${quiet} mysql-community-devel && \
  dnf clean ${quiet} all
