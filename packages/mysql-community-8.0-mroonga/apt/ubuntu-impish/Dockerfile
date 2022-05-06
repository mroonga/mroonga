ARG FROM=ubuntu:impish
FROM ${FROM}

RUN \
  echo "debconf debconf/frontend select Noninteractive" | \
    debconf-set-selections

ENV \
  DEBIAN_FRONTEND=noninteractive \
  MYSQL_SERVER_VERSION=mysql-8.0

ARG DEBUG

RUN \
  quiet=$([ "${DEBUG}" = "yes" ] || echo "-qq") && \
  apt update ${quiet} && \
  apt install -y -V --no-install-recommends ${quiet} \
    ca-certificates \
    software-properties-common \
    wget && \
  add-apt-repository -y ppa:groonga/ppa && \
  wget https://repo.mysql.com/mysql-apt-config_0.8.22-1_all.deb && \
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
    devscripts \
    dh-apparmor \
    groonga-normalizer-mysql \
    libgroonga-dev \
    libmysqlclient-dev && \
  mkdir -p /usr/global/share && \
  wget --no-verbose \
    https://packages.groonga.org/tmp/boost/boost_1_77_0.tar.bz2 && \
  mv boost_1_77_0.tar.bz2 /usr/global/share/
