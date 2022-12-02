ARG FROM=almalinux:9
FROM ${FROM}

ARG DEBUG

ENV \
  SCL=gcc-toolset-11

RUN \
  quiet=$([ "${DEBUG}" = "yes" ] || echo "--quiet") && \
  dnf update -y ${quiet} && \
  dnf install -y \
    https://repo.percona.com/yum/percona-release-latest.noarch.rpm \
    https://packages.groonga.org/almalinux/9/groonga-release-latest.noarch.rpm && \
  dnf module disable -y mysql && \
  percona-release setup ps80 && \
  dnf groupinstall -y ${quiet} "Development Tools" && \
  dnf install --enablerepo=crb -y ${quiet} \
    'dnf-command(builddep)' \
    'dnf-command(download)' && \
  # Percona Server's SRPM is broken. devtoolset-8 is required by dnf builddep.
  (dnf builddep -y ${quiet} --enablerepo=ps-80-release-sources \
     percona-server || :) && \
  dnf download -y ${quiet} --enablerepo=ps-80-release-sources --source \
    percona-server && \
  dnf install --enablerepo=crb -y ${quiet} \
    # Percona Server dependencies.
    cmake\
    cyrus-sasl-devel \
    cyrus-sasl-scram \
    libaio-devel \
    libcurl-devel \
    libtirpc-devel \
    ncurses-devel \
    numactl-devel \
    openldap-devel \
    pam-devel \
    perl \
    'perl(JSON)' \
    'perl(Memoize)' \
    'perl(Time::HiRes)' \
    readline-devel \
    rpcgen \
    time \
    # Mroonga dependencies.
    ccache \
    ${SCL} \
    ${SCL}-annobin-plugin-gcc \
    groonga-devel \
    groonga-normalizer-mysql-devel \
    intltool \
    libtool \
    make \
    percona-server-devel \
    pkgconfig \
    wget \
    which && \
  dnf clean ${quiet} all
