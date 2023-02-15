ARG FROM=oraclelinux:8
FROM ${FROM}

ARG DEBUG

ENV \
  SCL=gcc-toolset-11

RUN \
  quiet=$([ "${DEBUG}" = "yes" ] || echo "--quiet") && \
  dnf -y module disable mysql && \
  dnf update -y ${quiet} && \
  dnf install -y ${quiet} \
    'dnf-command(builddep)' \
    'dnf-command(download)' \
    https://apache.jfrog.io/artifactory/arrow/almalinux/8/apache-arrow-release-latest.rpm \
    https://packages.groonga.org/oracle-linux/8/groonga-release-latest.noarch.rpm \
    https://repo.mysql.com/mysql-community-minimal-release-el8-1.noarch.rpm && \
  dnf builddep --enablerepo=mysql80-community-minimal-source,ol8_codeready_builder -y ${quiet} mysql-community-minimal && \
  dnf download --enablerepo=mysql80-community-minimal-source -y ${quiet} --source mysql-community-minimal && \
  dnf install --enablerepo=ol8_codeready_builder -y ${quiet} \
    ${SCL} \
    ${SCL}-annobin-plugin-gcc \
    ccache \
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
  dnf install -y ${quiet} https://repo.mysql.com/mysql80-community-release-el8-3.noarch.rpm && \
  dnf install --disablerepo=appstream -y ${quiet} \
    mysql-community-devel && \
  dnf clean ${quiet} all

# Workaround: We can remove this once redhat-rpm-config uses "annobin"
# not "gcc-annobin".
RUN \
  sed \
    -i \
    -e 's/gcc-annobin/annobin/g' \
    /usr/lib/rpm/redhat/redhat-annobin-select-gcc-built-plugin
