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
  apt install -y -V ${quiet} \
    autoconf-archive \
    build-essential \
    ccache \
    chrpath \
    cmake \
    debhelper \
    devscripts \
    dh-apparmor \
    groonga-normalizer-mysql \
    libaio-dev \
    libcurl4-openssl-dev \
    libedit-dev \
    libevent-dev \
    libgroonga-dev \
    libicu-dev \
    liblz4-dev \
    libmecab-dev \
    libmysqlclient-dev \
    libncurses5-dev \
    libpcre3-dev \
    libprotobuf-dev \
    libprotoc-dev \
    libreadline-dev \
    libssl-dev \
    libtirpc-dev \
    libxml2-dev \
    libre2-dev \
    libwrap0-dev \
    lsb-release \
    mecab-utils \
    pkg-config \
    protobuf-compiler \
    unixodbc-dev \
    wget
