ARG FROM=oraclelinux:9
FROM ${FROM}

ARG DEBUG

ENV \
  SCL=gcc-toolset-12

RUN \
  quiet=$([ "${DEBUG}" = "yes" ] || echo "--quiet") && \
  dnf update -y ${quiet} && \
  dnf install -y ${quiet} \
    'dnf-command(builddep)' \
    'dnf-command(download)' \
    https://apache.jfrog.io/artifactory/arrow/almalinux/9/apache-arrow-release-latest.rpm \
    https://packages.groonga.org/oracle-linux/9/groonga-release-latest.noarch.rpm \
    https://repo.mysql.com/mysql80-community-release-el9.rpm && \
  dnf builddep --enablerepo=mysql80-community-source,ol9_codeready_builder -y ${quiet} mysql-community && \
  dnf download --enablerepo=mysql80-community-source -y ${quiet} --source mysql-community && \
  dnf install --enablerepo=ol9_codeready_builder -y ${quiet} \
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
