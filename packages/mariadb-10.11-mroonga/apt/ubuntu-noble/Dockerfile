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
    bison \
    build-essential \
    ccache \
    chrpath \
    cmake \
    debhelper \
    devscripts \
    dh-apparmor \
    gdb \
    groonga-normalizer-mysql \
    libaio-dev \
    libboost-dev \
    libgroonga-dev \
    libjemalloc-dev \
    libjudy-dev \
    libmariadb-client-lgpl-dev-compat \
    libmariadbd-dev \
    libmecab-dev \
    libncurses5-dev \
    libpam0g-dev \
    libpcre2-dev \
    libpmem-dev \
    libreadline-dev \
    libsnappy-dev \
    libssl-dev \
    liburing-dev \
    libxml2-dev \
    libre2-dev \
    lsb-release \
    perl \
    mecab-utils \
    pkg-config \
    psmisc \
    unixodbc-dev \
    zlib1g-dev
