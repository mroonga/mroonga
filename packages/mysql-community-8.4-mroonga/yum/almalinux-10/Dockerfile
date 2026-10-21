ARG FROM=almalinux:10
FROM ${FROM}

ARG DEBUG

RUN \
  quiet=$([ "${DEBUG}" = "yes" ] || echo "--quiet") && \
  dnf update -y ${quiet} && \
  dnf install --enablerepo=crb -y ${quiet} \
    'dnf-command(builddep)' \
    'dnf-command(download)' \
    https://packages.groonga.org/almalinux/$(cut -d: -f5 /etc/system-release-cpe | cut -d. -f1)/groonga-release-latest.noarch.rpm \
    https://repo.mysql.com/mysql84-community-release-el$(cut -d: -f5 /etc/system-release-cpe | cut -d. -f1).rpm \
    https://packages.apache.org/artifactory/arrow/almalinux/$(cut -d: -f5 /etc/system-release-cpe | cut -d. -f1)/apache-arrow-release-latest.rpm && \
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
    ccache \
    cmake \
    groonga-devel \
    groonga-normalizer-mysql-devel \
    intltool \
    libcurl-devel \
    libquadmath-devel \
    libtool \
    make \
    pkgconfig \
    rpm-build \
    wget \
    which && \
  dnf install --disablerepo=appstream,'mysql-9.7-lts-community*' -y ${quiet} \
    mysql-community-devel && \
  dnf clean ${quiet} all
