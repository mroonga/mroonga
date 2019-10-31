ARG FROM=debian:stretch
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
    autoconf-archive \
    autotools-dev \
    build-essential \
    debhelper \
    devscripts \
    gdb \
    lsb-release \
    libmariadb-client-lgpl-dev \
    libmariadbd-dev \
    libmecab-dev \
    libssl-dev \
    libxml2-dev \
    mecab-utils \
    pkg-config \
    unixodbc-dev \
    wget && \
  apt clean && \
  rm -rf /var/lib/apt/lists/* && \
  : "" && { \
    echo "deb [signed-by=/usr/share/keyrings/groonga-archive-keyring.gpg] http://packages.groonga.org/debian/ stretch main"; \
    echo "deb-src [signed-by=/usr/share/keyrings/groonga-archive-keyring.gpg] http://packages.groonga.org/debian/ stretch main"; \
  } | tee /etc/apt/sources.list.d/groonga.list && \
  wget -O /usr/share/keyrings/groonga-archive-keyring.gpg \
      https://packages.groonga.org/debian/groonga-archive-keyring.gpg && \
  apt update ${quiet} && \
  apt install -y -V ${quiet} \
    groonga-normalizer-mysql \
    libgroonga-dev && \
  apt build-dep -y mariadb-server-10.1
