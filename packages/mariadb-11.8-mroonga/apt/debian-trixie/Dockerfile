ARG FROM=debian:trixie
FROM ${FROM}

RUN \
  echo "debconf debconf/frontend select Noninteractive" | \
    debconf-set-selections

ARG DEBUG

RUN \
  sed -e 's/^Types: deb/Types: deb-src/' \
    /etc/apt/sources.list.d/debian.sources > \
    /etc/apt/sources.list.d/debian-source.sources && \
  apt update ${quiet} && \
  apt install -y -V ${quiet} \
    lsb-release \
    wget && \
  wget https://packages.apache.org/artifactory/arrow/$(lsb_release --id --short | tr 'A-Z' 'a-z')/apache-arrow-apt-source-latest-$(lsb_release --codename --short).deb && \
  apt install -y -V ${quiet} ./apache-arrow-apt-source-latest-$(lsb_release --codename --short).deb && \
  rm apache-arrow-apt-source-latest-$(lsb_release --codename --short).deb && \
  wget https://packages.groonga.org/debian/groonga-apt-source-latest-trixie.deb && \
  apt install -y -V ${quiet} ./groonga-apt-source-latest-trixie.deb && \
  apt update ${quiet} && \
  apt build-dep -y mariadb-server && \
  apt install -y -V ${quiet} \
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
    libpcre2-dev \
    libreadline-dev \
    libssl-dev \
    libxml2-dev \
    libre2-dev \
    lsb-release \
    mecab-utils \
    ninja-build \
    pkg-config \
    unixodbc-dev
