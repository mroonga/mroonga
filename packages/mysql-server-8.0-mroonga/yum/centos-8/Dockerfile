ARG FROM=centos:8
FROM ${FROM}

ARG DEBUG

RUN \
  ORACLE_LINUX_REPOSITORY_BASE_URL="https://yum.oracle.com/repo/OracleLinux/OL8/appstream/x86_64/getPackage"
  quiet=$([ "${DEBUG}" = "yes" ] || echo "--quiet") && \
  dnf update -y ${quiet} && \
  dnf install -y ${quiet} \
    https://repo.mysql.com/mysql80-community-release-el8.rpm && \
  dnf install -y \
    https://packages.groonga.org/centos/groonga-release-latest.noarch.rpm \
    epel-release && \
  dnf groupinstall -y ${quiet} "Development Tools" && \
  dnf install --enablerepo=powertools -y ${quiet} \
    'dnf-command(builddep)' \
    'dnf-command(download)' && \
  dnf builddep --enablerepo=powertools -y ${quiet} mysql-community && \
  dnf download -y ${quiet} --source mysql-community && \
  dnf install --enablerepo=powertools -y ${quiet} \
    ${ORACLE_LINUX_REPOSITORY_BASE_URL}/gcc-toolset-10-annobin-9.29-1.el8.x86_64.rpm \
    ${ORACLE_LINUX_REPOSITORY_BASE_URL}/gcc-toolset-10-gcc-10.2.1-2.0.2.el8.x86_64.rpm \
    ${ORACLE_LINUX_REPOSITORY_BASE_URL}/gcc-toolset-10-gcc-c++-10.2.1-2.0.2.el8.x86_64.rpm \
    ${ORACLE_LINUX_REPOSITORY_BASE_URL}/gcc-toolset-10-libstdc++-devel-10.2.1-2.0.2.el8.x86_64.rpm \
    ccache \
    groonga-devel \
    groonga-normalizer-mysql-devel \
    intltool \
    libtool \
    make \
    pkgconfig \
    wget \
    which && \
  dnf install --disablerepo=appstream -y ${quiet} \
    mysql-community-devel && \
  dnf clean ${quiet} all
