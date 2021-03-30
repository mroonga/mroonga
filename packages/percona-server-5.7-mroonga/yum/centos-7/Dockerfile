ARG FROM=centos:7
FROM ${FROM}

ARG DEBUG

RUN \
  quiet=$([ "${DEBUG}" = "yes" ] || echo "--quiet") && \
  yum update -y ${quiet} && \
  yum install -y ${quiet} \
    https://repo.percona.com/yum/percona-release-latest.noarch.rpm \
    https://packages.groonga.org/centos/groonga-release-latest.noarch.rpm  && \
  percona-release setup ps57 && \
  yum groupinstall -y ${quiet} "Development Tools" && \
  yum-builddep -y ${quiet} --enablerepo=ps-57-release-sources \
    Percona-Server-57 && \
  yumdownloader -y ${quiet} --enablerepo=ps-57-release-sources --source \
    Percona-Server-57 && \
  yum install -y ${quiet} \
    'perl(Env)' \
    'perl(Time::HiRes)' \
    Percona-Server-devel-57 \
    gcc-c++ \
    groonga-devel \
    groonga-normalizer-mysql-devel \
    intltool \
    libtool \
    make \
    pkgconfig \
    selinux-policy-devel \
    wget \
    which && \
  yum clean ${quiet} all
