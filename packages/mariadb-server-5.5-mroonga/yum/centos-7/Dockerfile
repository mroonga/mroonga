ARG FROM=centos:7
FROM ${FROM}

ARG DEBUG

RUN \
  quiet=$([ "${DEBUG}" = "yes" ] || echo "--quiet") && \
  yum install -y \
    https://packages.groonga.org/centos/groonga-release-latest.noarch.rpm \
    epel-release && \
  yum groupinstall -y ${quiet} "Development Tools" && \
  yum install -y ${quiet} \
    cmake \
    cyrus-sasl-devel \
    gcc \
    gcc-c++ \
    gperf \
    groonga-devel \
    groonga-normalizer-mysql-devel \
    intltool \
    libaio-devel \
    libcurl-devel \
    libtool \
    make \
    mariadb-devel \
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
