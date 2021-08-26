ARG FROM=centos:7
FROM ${FROM}

ARG DEBUG

RUN \
  quiet=$([ "${DEBUG}" = "yes" ] || echo "--quiet") && \
  yum update -y ${quiet} && \
  { \
    echo "[mariadb]"; \
    echo "name = MariaDB"; \
    echo "baseurl = http://yum.mariadb.org/10.6/centos7-amd64"; \
    echo "gpgkey = https://yum.mariadb.org/RPM-GPG-KEY-MariaDB"; \
    echo "gpgcheck = 1"; \
  } | tee /etc/yum.repos.d/MariaDB.repo && \
  yum install -y \
    https://packages.groonga.org/centos/groonga-release-latest.noarch.rpm && \
  yum groupinstall -y ${quiet} "Development Tools" && \
  yum-builddep -y ${quiet} MariaDB && \
  yumdownloader -y ${quiet} --source MariaDB && \
  yum install -y ${quiet} \
    MariaDB-devel \
    ccache \
    gcc-c++ \
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
