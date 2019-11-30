ARG FROM=centos:6
FROM ${FROM}

ARG DEBUG

RUN \
  quiet=$([ "${DEBUG}" = "yes" ] || echo "--quiet") && \
  yum update -y ${quiet} && \
  yum install -y ${quiet} \
    https://repo.percona.com/yum/percona-release-latest.noarch.rpm \
    https://packages.groonga.org/centos/groonga-release-latest.noarch.rpm && \
  yum groupinstall -y ${quiet} "Development Tools" && \
  yum install -y ${quiet} \
    Percona-Server-devel-57 \
    cmake28 \
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
