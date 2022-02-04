ARG FROM=centos:7
FROM ${FROM}

ARG DEBUG

ENV \
  SCL=devtoolset-10

RUN \
  quiet=$([ "${DEBUG}" = "yes" ] || echo "--quiet") && \
  yum update -y ${quiet} && \
  yum install -y ${quiet} \
    centos-release-scl-rh \
    https://packages.groonga.org/centos/groonga-release-latest.noarch.rpm \
    https://repo.mysql.com/mysql80-community-release-el7-5.noarch.rpm && \
  yum-config-manager --enable centos-sclo-rh-testing && \
  yum-builddep -y ${quiet} mysql-community && \
  yumdownloader -y ${quiet} --source mysql-community && \
  yum install -y ${quiet} \
    ccache \
    groonga-devel \
    groonga-normalizer-mysql-devel \
    intltool \
    libtool \
    make \
    mysql-community-devel \
    pkgconfig \
    rpm-build \
    wget \
    which && \
  yum clean ${quiet} all
