ARG FROM=ubuntu:noble
FROM ${FROM}

RUN \
  echo "debconf debconf/frontend select Noninteractive" | \
    debconf-set-selections

ARG DEBUG

RUN \
  quiet=$([ "${DEBUG}" = "yes" ] || echo "-qq") && \
  sed \
    -e 's/^Types: deb/Types: deb-src/' \
    /etc/apt/sources.list.d/ubuntu.sources > \
    /etc/apt/sources.list.d/ubuntu-sources.sources && \
  apt update ${quiet} && \
  apt install -y -V --no-install-recommends ${quiet} \
    ca-certificates \
    lsb-release \
    software-properties-common \
    wget && \
  add-apt-repository -y ppa:groonga/ppa && \
  wget https://packages.groonga.org/$(lsb_release --id --short | tr 'A-Z' 'a-z')/groonga-apt-source-latest-$(lsb_release --codename --short).deb && \
  apt build-dep -y mariadb-server-10.11 && \
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
    pkg-config \
    unixodbc-dev
