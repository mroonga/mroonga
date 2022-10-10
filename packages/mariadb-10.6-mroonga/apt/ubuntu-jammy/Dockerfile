ARG FROM=ubuntu:jammy
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
    ca-certificates \
    lsb-release \
    software-properties-common \
    wget && \
  wget https://packages.groonga.org/$(lsb_release --id --short | tr 'A-Z' 'a-z')/groonga-apt-source-latest-$(lsb_release --codename --short).deb && \
  apt install -y -V ${quiet} \
    ./*.deb && \
  rm *.deb && \
  add-apt-repository -y universe && \
  add-apt-repository -y ppa:groonga/ppa && \
  apt update ${quiet} && \
  apt build-dep -y mariadb-server-10.6 && \
  apt install -y -V ${quiet} \
    autoconf-archive \
    build-essential \
    ccache \
    chrpath \
    debhelper \
    devscripts \
    dh-apparmor \
    groonga-normalizer-mysql \
    libaio-dev \
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
    pkg-config && \
  mkdir -p /usr/global/share && \
  wget --no-verbose \
    https://packages.groonga.org/tmp/boost/boost_1_77_0.tar.bz2 && \
  mv boost_1_77_0.tar.bz2 /usr/global/share/
