ARG FROM=centos:8
FROM ${FROM}

ARG DEBUG

RUN \
  quiet=$([ "${DEBUG}" = "yes" ] || echo "--quiet") && \
  dnf update -y ${quiet} && \
  dnf install -y \
    https://repo.percona.com/yum/percona-release-latest.noarch.rpm \
    https://packages.groonga.org/centos/groonga-release-latest.noarch.rpm \
    epel-release && \
  percona-release setup ps80 && \
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
    mysql-libs \
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
  dnf search percona-server && \
  dnf search --disablerepo=AppStream percona-server && \
  dnf install --disablerepo=AppStream -y ${quiet} \
    percona-server-devel && \
  dnf clean ${quiet} all
