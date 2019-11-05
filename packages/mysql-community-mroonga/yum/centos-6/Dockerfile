ARG FROM=centos:6
FROM ${FROM}

ARG DEBUG

RUN \
  quiet=$([ "${DEBUG}" = "yes" ] || echo "--quiet") && \
  yum install -y https://packages.groonga.org/centos/groonga-release-latest.noarch.rpm && \
  yum update -y ${quiet} && \
  yum install -y ${quiet} epel-release centos-release-scl && \
  yum groupinstall -y ${quiet} "Development Tools" && \
  yum install -y ${quiet} \
    cmake28 \
    cyrus-sasl-devel \
    devtoolset-6-gcc \
    devtoolset-6-g++ \
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
    scl-utils \
    selinux-policy-devel \
    systemtap-sdt-devel \
    tar \
    time \
    wget \
    zlib-devel && \
  yum clean ${quiet} all
