ARG FROM=centos:7
FROM ${FROM}

ARG DEBUG

ENV \
  DEVTOOLSET_VERSION=9

RUN \
  quiet=$([ "${DEBUG}" = "yes" ] || echo "--quiet") && \
  yum update -y ${quiet} && \
  yum install -y ${quiet} centos-release-scl-rh && \
  yum install -y ${quiet} \
    https://repo.mysql.com/mysql80-community-release-el7.rpm && \
  yum install -y \
    https://packages.groonga.org/centos/groonga-release-latest.noarch.rpm \
    epel-release && \
  yum groupinstall -y ${quiet} "Development Tools" && \
  yum-builddep -y ${quiet} mysql-community && \
  yumdownloader -y ${quiet} --source mysql-community && \
  yum install -y ${quiet} \
    ccache \
    devtoolset-${DEVTOOLSET_VERSION} \
    groonga-devel \
    groonga-normalizer-mysql-devel \
    intltool \
    libtool \
    make \
    mysql-community-devel \
    pkgconfig \
    wget \
    which && \
  yum clean ${quiet} all
