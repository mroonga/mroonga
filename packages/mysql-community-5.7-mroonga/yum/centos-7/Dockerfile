ARG FROM=centos:7
FROM ${FROM}

ARG DEBUG

RUN \
  quiet=$([ "${DEBUG}" = "yes" ] || echo "--quiet") && \
  yum update -y ${quiet} && \
  yum install -y ${quiet} \
    https://repo.mysql.com/mysql80-community-release-el7-5.noarch.rpm && \
  yum-config-manager --disable mysql80-community && \
  yum-config-manager --enable mysql57-community && \
  yum install -y \
    https://packages.groonga.org/centos/groonga-release-latest.noarch.rpm && \
  yum groupinstall -y ${quiet} "Development Tools" && \
  yum-builddep -y ${quiet} mysql-community && \
  yumdownloader -y ${quiet} --source mysql-community && \
  yum install -y ${quiet} \
    ccache \
    gcc-c++ \
    groonga-devel \
    groonga-normalizer-mysql-devel \
    intltool \
    libtool \
    make \
    mysql-community-devel \
    pkgconfig \
    sudo \
    wget \
    which && \
  yum clean ${quiet} all
