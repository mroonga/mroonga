ARG FROM=almalinux:8
FROM ${FROM}

ARG DEBUG

RUN \
  quiet=$([ "${DEBUG}" = "yes" ] || echo "--quiet") && \
  dnf update -y ${quiet} && \
  dnf install -y \
    https://repo.percona.com/yum/percona-release-latest.noarch.rpm \
    https://packages.groonga.org/almalinux/8/groonga-release-latest.noarch.rpm && \
  dnf module disable -y mysql && \
  percona-release setup ps80 && \
  dnf groupinstall -y ${quiet} "Development Tools" && \
  dnf install --enablerepo=powertools -y ${quiet} \
    'dnf-command(builddep)' \
    'dnf-command(download)' && \
  # Percona Server's SRPM is broken. devtoolset-8 is required by dnf builddep.
  (dnf builddep -y ${quiet} --enablerepo=ps-80-release-sources \
     percona-server || :) && \
  dnf download -y ${quiet} --enablerepo=ps-80-release-sources --source \
    percona-server && \
  dnf install --enablerepo=powertools -y ${quiet} \
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
    gcc-toolset-10 \
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
