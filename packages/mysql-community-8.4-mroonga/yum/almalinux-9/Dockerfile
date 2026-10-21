ARG FROM=almalinux:9
FROM ${FROM}

ARG DEBUG

ENV \
  SCL=gcc-toolset-12

RUN \
  quiet=$([ "${DEBUG}" = "yes" ] || echo "--quiet") && \
  dnf update -y ${quiet} && \
  dnf install --enablerepo=crb -y ${quiet} \
    'dnf-command(builddep)' \
    'dnf-command(download)' \
    https://packages.groonga.org/almalinux/9/groonga-release-latest.noarch.rpm \
    https://repo.mysql.com/mysql84-community-release-el9.rpm \
    https://apache.jfrog.io/artifactory/arrow/almalinux/$(cut -d: -f5 /etc/system-release-cpe | cut -d. -f1)/apache-arrow-release-latest.rpm && \
  dnf config-manager \
    --set-enabled mysql-8.4-lts-community,mysql-tools-8.4-lts-community && \
  dnf builddep \
    --disablerepo='mysql-9.7-lts-community*','mysql-tools-9.7-lts-community*' \
    --enablerepo=crb,mysql-8.4-lts-community-source -y ${quiet} \
    mysql-community && \
  dnf download \
    --disablerepo='mysql-9.7-lts-community*','mysql-tools-9.7-lts-community*' -y ${quiet} \
    --source mysql-community && \
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
  dnf install --disablerepo=appstream,'mysql-9.7-lts-community*' -y ${quiet} \
    mysql-community-devel && \
  dnf clean ${quiet} all
