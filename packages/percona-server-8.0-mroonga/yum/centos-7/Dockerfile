ARG FROM=centos:7
FROM ${FROM}

ARG DEBUG

ENV \
  SCL=devtoolset-10

RUN \
  quiet=$([ "${DEBUG}" = "yes" ] || echo "--quiet") && \
  yum update -y ${quiet} && \
  yum install -y ${quiet} centos-release-scl-rh && \
  yum install -y ${quiet} \
    https://repo.percona.com/yum/percona-release-latest.noarch.rpm \
    https://packages.groonga.org/centos/groonga-release-latest.noarch.rpm && \
  percona-release setup ps80 && \
  yum groupinstall -y ${quiet} "Development Tools" && \
  yum-builddep -y ${quiet} --enablerepo=ps-80-release-sources \
    percona-server && \
  yumdownloader -y ${quiet} --enablerepo=ps-80-release-sources --source \
    percona-server && \
  yum install -y ${quiet} \
    # Percona Server's SRPM is broken. Some packages can't be installed by
    # yum-builddep.
    'perl(Env)' \
    'perl(Time::HiRes)' \
    # Percona Server dependencies.
    cyrus-sasl-devel \
    cyrus-sasl-scram \
    # Mroonga dependencies.
    percona-server-devel \
    ccache \
    ${SCL} \
    groonga-devel \
    groonga-normalizer-mysql-devel \
    intltool \
    make \
    pkgconfig \
    wget \
    which && \
  yum clean ${quiet} all
