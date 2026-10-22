ARG FROM=almalinux:8
FROM ${FROM}

ARG DEBUG

ENV \
  SCL=gcc-toolset-12

RUN \
  quiet=$([ "${DEBUG}" = "yes" ] || echo "--quiet") && \
  os_version=$(cut -d: -f5 /etc/system-release-cpe | cut -d. -f1) && \
  dnf update -y ${quiet} && \
  dnf module -y disable mariadb && \
  dnf module -y disable mysql && \
  dnf install --enablerepo=powertools -y ${quiet} \
    'dnf-command(builddep)' \
    'dnf-command(download)' \
    https://packages.apache.org/artifactory/arrow/almalinux/${os_version}/apache-arrow-release-latest.rpm \
    https://packages.groonga.org/almalinux/${os_version}/groonga-release-latest.noarch.rpm \
    https://repo.mysql.com/yum/mysql-8.4-community/el/${os_version}/x86_64/mysql84-community-release-el${os_version}-2.noarch.rpm && \
  dnf builddep --enablerepo=powertools -y ${quiet} mysql-community && \
  dnf download -y ${quiet} --source mysql-community && \
  dnf install --enablerepo=powertools -y ${quiet} \
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

# Workaround: We can remove this once redhat-rpm-config uses "annobin"
# not "gcc-annobin".
RUN \
  sed \
    -i \
    -e 's/gcc-annobin/annobin/g' \
    /usr/lib/rpm/redhat/redhat-annobin-select-gcc-built-plugin
