ARG FROM=centos:6
FROM ${FROM}

ARG DEBUG

ENV \
  DEVTOOLSET_VERSION=8

RUN \
  quiet=$([ "${DEBUG}" = "yes" ] || echo "--quiet") && \
  yum update -y ${quiet} && \
  yum install -y ${quiet} centos-release-scl-rh && \
  yum install -y ${quiet} \
    http://repo.mysql.com/mysql80-community-release-el6.rpm && \
  yum install -y \
    https://packages.groonga.org/centos/groonga-release-latest.noarch.rpm \
    epel-release && \
  yum groupinstall -y ${quiet} "Development Tools" && \
  yum install -y ${quiet} \
    mysql-community-devel \
    cmake3 \
    cyrus-sasl-devel \
    devtoolset-${DEVTOOLSET_VERSION} \
    gperf \
    groonga-devel \
    groonga-normalizer-mysql-devel \
    intltool \
    libaio-devel \
    libcurl-devel \
    libtool \
    make \
    ncurses-devel \
    numactl-devel \
    openldap-devel \
    openssl-devel \
    pam-devel \
    perl-Env \
    perl-JSON \
    perl-Test-Simple \
    perl-Time-HiRes \
    pkgconfig \
    readline-devel \
    selinux-policy-devel \
    sudo \
    systemtap-sdt-devel \
    tar \
    time \
    wget \
    which \
    zlib-devel && \
  yum clean ${quiet} all
