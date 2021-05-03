ARG FROM=centos:7
FROM ${FROM}

ARG DEBUG

RUN \
  ORACLE_LINUX_REPOSITORY_BASE_URL="https://yum.oracle.com/repo/OracleLinux/OL7/SoftwareCollections/x86_64/getPackage" && \
  quiet=$([ "${DEBUG}" = "yes" ] || echo "--quiet") && \
  yum update -y ${quiet} && \
  yum install -y ${quiet} \
    https://repo.mysql.com/mysql80-community-release-el7.rpm && \
  yum install -y \
    https://packages.groonga.org/centos/groonga-release-latest.noarch.rpm \
    epel-release && \
  yum groupinstall -y ${quiet} "Development Tools" && \
  yum-builddep -y ${quiet} mysql-community && \
  yumdownloader -y ${quiet} --source mysql-community && \
  yum install -y ${quiet} \
    ${ORACLE_LINUX_REPOSITORY_BASE_URL}/devtoolset-10-binutils-2.35-3.el7.x86_64.rpm \
    ${ORACLE_LINUX_REPOSITORY_BASE_URL}/devtoolset-10-gcc-10.2.1-2.1.0.1.el7.x86_64.rpm \
    ${ORACLE_LINUX_REPOSITORY_BASE_URL}/devtoolset-10-gcc-c++-10.2.1-2.1.0.1.el7.x86_64.rpm \
    ${ORACLE_LINUX_REPOSITORY_BASE_URL}/devtoolset-10-libstdc++-devel-10.2.1-2.1.0.1.el7.x86_64.rpm \
    ${ORACLE_LINUX_REPOSITORY_BASE_URL}/devtoolset-10-runtime-10.0-0.el7.x86_64.rpm \
    ccache \
    groonga-devel \
    groonga-normalizer-mysql-devel \
    intltool \
    libtool \
    make \
    mysql-community-devel \
    pkgconfig \
    wget \
    which && \
  yum clean ${quiet} all
