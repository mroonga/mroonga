ARG FROM=debian:trixie
FROM ${FROM}

RUN \
  echo "debconf debconf/frontend select Noninteractive" | \
    debconf-set-selections

ENV \
  DEBIAN_FRONTEND=noninteractive \
  MYSQL_SERVER_VERSION=mysql-9.7-lts

ARG DEBUG

RUN \
  quiet=$([ "${DEBUG}" = "yes" ] || echo "-qq") && \
  apt update ${quiet} && \
  apt install -y -V --no-install-recommends ${quiet} lsb-release && \
  distribution=$(lsb_release --id --short | tr 'A-Z' 'a-z') && \
  code_name=$(lsb_release --codename --short) && \
  apt install -y -V --no-install-recommends ${quiet} \
    ca-certificates \
    wget && \
  wget https://packages.apache.org/artifactory/arrow/${distribution}/apache-arrow-apt-source-latest-${code_name}.deb && \
  apt install -y -V ${quiet} ./apache-arrow-apt-source-latest-${code_name}.deb && \
  wget https://packages.groonga.org/${distribution}/groonga-apt-source-latest-${code_name}.deb && \
  # Normally, we should use https://repo.mysql.com/mysql-apt-config.deb.
  # However, it currently does not support MySQL 9.7.
  # MySQL 9.7 support is currently available only in mysql-apt-config_0.8.39-1_all.deb.
  # Therefore, we use mysql-apt-config_0.8.39-1_all.deb.
  wget https://repo.mysql.com/mysql-apt-config_0.8.39-1_all.deb && \
  apt install -y -V --no-install-recommends ${quiet} \
    ./*.deb && \
  rm *.deb && \
  apt update ${quiet} && \
  apt build-dep -y --no-install-recommends ${quiet} \
    mysql-community && \
  apt source ${quiet} \
    mysql-community && \
  apt install -y -V --no-install-recommends ${quiet} \
    build-essential \
    ccache \
    cmake \
    devscripts \
    dh-apparmor \
    groonga-normalizer-mysql \
    libgroonga-dev \
    libmysqlclient-dev \
    libtirpc-dev \
    mysql-router-community \
    ninja-build
