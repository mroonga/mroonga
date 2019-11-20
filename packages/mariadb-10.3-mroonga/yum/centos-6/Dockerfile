ARG FROM=centos:6
FROM ${FROM}

ARG DEBUG

RUN \
  quiet=$([ "${DEBUG}" = "yes" ] || echo "--quiet") && \
  yum update -y ${quiet} && \
  { \
    echo "[mariadb]"; \
    echo "name = MariaDB"; \
    echo "baseurl = http://yum.mariadb.org/10.3/centos6-amd64"; \
    echo "gpgkey = https://yum.mariadb.org/RPM-GPG-KEY-MariaDB"; \
    echo "gpgcheck = 1"; \
  } | tee /etc/yum.repos.d/MariaDB.repo && \
  yum install -y \
    https://packages.groonga.org/centos/groonga-release-latest.noarch.rpm \
    centos-release-scl \
    epel-release && \
  yum groupinstall -y ${quiet} "Development Tools" && \
  yum install -y ${quiet} \
    MariaDB-devel \
    cmake \
    cyrus-sasl-devel \
    devtoolset-6 \
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
