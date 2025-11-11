ARG FROM=ubuntu:jammy
FROM ${FROM}

RUN \
  echo "debconf debconf/frontend select Noninteractive" | \
    debconf-set-selections

ENV \
  DEBIAN_FRONTEND=noninteractive \
  MYSQL_SERVER_VERSION=mysql-8.4-lts

ARG DEBUG

RUN \
  quiet=$([ "${DEBUG}" = "yes" ] || echo "-qq") && \
  apt update ${quiet} && \
  apt install -y -V --no-install-recommends ${quiet} \
    ca-certificates \
    gpg-agent \
    lsb-release \
    software-properties-common \
    wget && \
  add-apt-repository -y ppa:groonga/ppa && \
  wget https://packages.groonga.org/$(lsb_release --id --short | tr 'A-Z' 'a-z')/groonga-apt-source-latest-$(lsb_release --codename --short).deb && \
  # This is This is workaround.
  # We can use mysql-apt-config.deb when https://repo.mysql.com/mysql-apt-config.deb is updated to the latest version.
  # About https://repo.mysql.com/mysql-apt-config.deb updating has already reported to upstream.
  # See: https://bugs.mysql.com/bug.php?id=119212
  wget https://repo.mysql.com/apt/$(lsb_release --id --short | tr 'A-Z' 'a-z')/pool/mysql-apt-config/m/mysql-apt-config/mysql-apt-config_0.8.36-1_all.deb && \
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
    ninja-build
