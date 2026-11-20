ARG FROM=ubuntu:resolute
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
  wget https://packages.groonga.org/ubuntu/groonga-apt-source-latest-$(lsb_release --codename --short).deb && \
  apt install -y -V ./groonga-apt-source-latest-$(lsb_release --codename --short).deb && \
  apt update && \
  apt install -y -V ${quiet} \
    autoconf-archive \
    bison \
    build-essential \
    ccache \
    chrpath \
    cmake \
    debhelper \
    devscripts \
    dh-apparmor \
    gettext \
    groonga-normalizer-mysql \
    libaio-dev \
    libcurl4-openssl-dev \
    libedit-dev \
    libevent-dev \
    libfido2-dev \
    libgoogle-perftools-dev \
    libgroonga-dev \
    libicu-dev \
    liblz4-dev \
    libmecab-dev \
    libmysqlclient-dev \
    libncurses-dev \
    libnuma-dev \
    libpcre2-dev \
    libprotobuf-dev \
    libprotoc-dev \
    libreadline-dev \
    libssl-dev \
    libtirpc-dev \
    libxml2-dev \
    libzstd-dev \
    libre2-dev \
    libwrap0-dev \
    lsb-release \
    mecab-utils \
    ninja-build \
    pkg-config \
    protobuf-compiler \
    python3-myst-parser \
    python3-pydata-sphinx-theme \
    python3-sphinx \
    ruby \
    ruby-gettext \
    unixodbc-dev \
    wget \
    zlib1g-dev
