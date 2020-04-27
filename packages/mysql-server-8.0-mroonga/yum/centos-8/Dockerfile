ARG FROM=centos:8
FROM ${FROM}

ARG DEBUG

RUN \
  quiet=$([ "${DEBUG}" = "yes" ] || echo "--quiet") && \
  dnf update -y ${quiet} && \
  dnf install -y ${quiet} \
    http://repo.mysql.com/mysql80-community-release-el8.rpm && \
  dnf install -y \
    https://packages.groonga.org/centos/groonga-release-latest.noarch.rpm \
    epel-release && \
  dnf groupinstall -y ${quiet} "Development Tools" && \
  dnf install --enablerepo=PowerTools -y ${quiet} \
    cmake \
    cyrus-sasl-devel \
    gcc-toolset-9 \
    gcc-toolset-9-gcc-c++ \
    gperf \
    groonga-devel \
    groonga-normalizer-mysql-devel \
    intltool \
    libaio-devel \
    libcurl-devel \
    libtirpc-devel \
    libtool \
    make \
    ncurses-devel \
    numactl-devel \
    openldap-devel \
    openssl-devel \
    pam-devel \
    perl \
    perl-Env \
    perl-JSON \
    perl-Test-Simple \
    perl-Time-HiRes \
    pkgconfig \
    readline-devel \
    rpcgen \
    selinux-policy-devel \
    sudo \
    systemtap-sdt-devel \
    tar \
    time \
    wget \
    which \
    zlib-devel && \
  dnf install --disablerepo=AppStream -y ${quiet} \
    mysql-community-devel && \
  dnf clean ${quiet} all
