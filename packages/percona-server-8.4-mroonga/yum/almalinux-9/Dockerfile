ARG FROM=almalinux:9
FROM ${FROM}

ARG DEBUG

ENV \
  SCL=gcc-toolset-12

RUN \
  quiet=$([ "${DEBUG}" = "yes" ] || echo "--quiet") && \
  dnf update -y ${quiet} && \
  dnf install -y \
    https://repo.percona.com/yum/percona-release-latest.noarch.rpm \
    https://packages.groonga.org/almalinux/9/groonga-release-latest.noarch.rpm \
    https://apache.jfrog.io/artifactory/arrow/almalinux/$(cut -d: -f5 /etc/system-release-cpe | cut -d. -f1)/apache-arrow-release-latest.rpm && \
  percona-release enable-only ps-84-lts release && \
  percona-release enable tools release && \
  dnf groupinstall -y ${quiet} "Development Tools" && \
  dnf install --enablerepo=crb -y ${quiet} \
    'dnf-command(builddep)' \
    'dnf-command(download)' && \
  # Percona Server's SRPM is broken. devtoolset-8 is required by dnf builddep.
  (dnf builddep -y ${quiet} --enablerepo=ps-84-lts-release-sources \
     percona-server || :) && \
  dnf download -y ${quiet} --enablerepo=ps-84-lts-release-sources --source \
    percona-server && \
  dnf install --enablerepo=crb -y ${quiet} \
    # Percona Server dependencies.
    cmake \
    cyrus-sasl-devel \
    cyrus-sasl-scram \
    krb5-devel \
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
