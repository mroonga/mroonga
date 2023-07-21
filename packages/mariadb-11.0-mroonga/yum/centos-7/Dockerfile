ARG FROM=centos:7
FROM ${FROM}

ARG DEBUG

ENV \
  SCL=devtoolset-11

RUN \
  quiet=$([ "${DEBUG}" = "yes" ] || echo "--quiet") && \
  yum update -y ${quiet} && \
  { \
    echo "[mariadb]"; \
    echo "name = MariaDB"; \
    echo "baseurl = https://yum.mariadb.org/11.0/rhel/7/x86_64/"; \
    echo "gpgkey = https://yum.mariadb.org/RPM-GPG-KEY-MariaDB"; \
    echo "gpgcheck = 1"; \
  } | tee /etc/yum.repos.d/MariaDB.repo && \
  yum install -y \
    centos-release-scl-rh \
    https://packages.groonga.org/centos/7/groonga-release-latest.noarch.rpm && \
  yum groupinstall -y ${quiet} "Development Tools" && \
  yum-builddep -y ${quiet} MariaDB && \
  yumdownloader -y ${quiet} --source MariaDB && \
  yum install -y ${quiet} \
    ${SCL} \
    MariaDB-devel \
    ccache \
    groonga-devel \
    groonga-normalizer-mysql-devel \
    intltool \
    libtool \
    make \
    pkgconfig \
    sudo \
    wget \
    which && \
  yum clean ${quiet} all
