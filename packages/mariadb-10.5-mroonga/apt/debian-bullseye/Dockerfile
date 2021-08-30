ARG FROM=debian:bullseye
FROM ${FROM}

RUN \
  echo "debconf debconf/frontend select Noninteractive" | \
    debconf-set-selections

ARG DEBUG

RUN \
  quiet=$([ "${DEBUG}" = "yes" ] || echo "-qq") && \
  grep '^deb ' /etc/apt/sources.list | \
    sed -e 's/^deb /deb-src /' > /etc/apt/sources.list.d/base-source.list && \
  apt update ${quiet} && \
  apt install -y -V ${quiet} \
    wget && \
  wget https://packages.groonga.org/debian/groonga-apt-source-latest-bullseye.deb && \
  apt install -y -V ${quiet} ./groonga-apt-source-latest-bullseye.deb && \
  apt update ${quiet} && \
  apt build-dep -y mariadb-server-10.5 && \
  apt install -y -V ${quiet} \
    autoconf-archive \
    build-essential \
    ccache \
    chrpath \
    debhelper \
    devscripts \
    dh-apparmor \
    groonga-normalizer-mysql \
    libgroonga-dev \
    libmariadb-client-lgpl-dev-compat \
    libmariadbd-dev \
    libmecab-dev \
    libpcre3-dev \
    libreadline-dev \
    libssl-dev \
    libxml2-dev \
    libre2-dev \
    lsb-release \
    mecab-utils \
    pkg-config \
    unixodbc-dev
