ARG FROM=centos:7
FROM ${FROM}

ARG DEBUG

RUN \
  quiet=$([ "${DEBUG}" = "yes" ] || echo "--quiet") && \
  yum install -y https://packages.groonga.org/centos/groonga-release-latest.noarch.rpm && \
  yum update -y ${quiet} && \
  yum install -y ${quiet} epel-release && \
  yum groupinstall -y ${quiet} "Development Tools" && \
  yum http://repo.mysql.com/mysql-community-release-el6-7.noarch.rpm
  yum install -y ${quiet} \
    cmake \
    cyrus-sasl-devel \
    gcc \
    gcc-c++ \
    gperf \
    groonga-devel \
    groonga-normalizer-mysql-devel \
    intltool \
    libtool \
    libaio-devel \
    libcurl-devel \
    make \
    ncurses-devel \
    numactl-devel \
    openssl-devel \
    pam-devel \
    perl-Time-HiRes \
    perl-Env \
    perl-Test-Simple \
    perl-JSON \
    openldap-devel \
    readline-devel \
    pkgconfig \
    sudo \
    selinux-policy-devel \
    systemtap-sdt-devel \
    tar \
    time \
    wget \
    zlib-devel && \
  yum install -y mysql-community-devel && \
  yum clean ${quiet} all
