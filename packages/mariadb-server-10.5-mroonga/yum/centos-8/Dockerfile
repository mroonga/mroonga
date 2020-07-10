ARG FROM=centos:8
FROM ${FROM}

ARG DEBUG

RUN \
  quiet=$([ "${DEBUG}" = "yes" ] || echo "--quiet") && \
  dnf update -y ${quiet} && \
  { \
    echo "[mariadb]"; \
    echo "name = MariaDB"; \
    echo "baseurl = http://yum.mariadb.org/10.5/centos8-amd64"; \
    echo "gpgkey = https://yum.mariadb.org/RPM-GPG-KEY-MariaDB"; \
    echo "gpgcheck = 1"; \
  } | tee /etc/yum.repos.d/MariaDB.repo && \
  dnf install -y \
    https://packages.groonga.org/centos/groonga-release-latest.noarch.rpm \
    epel-release && \
  dnf groupinstall -y ${quiet} "Development Tools" && \
  dnf install --enablerepo=PowerTools -y ${quiet} \
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
  dnf install --disablerepo=AppStream -y ${quiet} \
    MariaDB-devel && \
  dnf clean ${quiet} all
