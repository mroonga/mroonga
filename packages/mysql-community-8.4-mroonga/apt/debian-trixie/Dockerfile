ARG FROM=debian:trixie
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
  distribution=$(lsb_release --id --short | tr 'A-Z' 'a-z') && \
  code_name=$(lsb_release --codename --short) && \
  apt update ${quiet} && \
  apt install -y -V --no-install-recommends ${quiet} \
    ca-certificates \
    lsb-release \
    wget && \
  wget https://packages.apache.org/artifactory/arrow/${distribution}/apache-arrow-apt-source-latest-${code_name}.deb && \
  apt install -y -V ${quiet} ./apache-arrow-apt-source-latest-${code_name}.deb && \
  wget https://packages.groonga.org/${distribution}/groonga-apt-source-latest-${code_name}.deb && \
  # This is a workaround.
  # We can use mysql-apt-config.deb when https://repo.mysql.com/mysql-apt-config.deb is updated to the latest version.
  # The https://repo.mysql.com/mysql-apt-config.deb updating has already been reported to upstream.
  # See: https://bugs.mysql.com/bug.php?id=119212
  wget https://repo.mysql.com/apt/${distibution}/pool/mysql-apt-config/m/mysql-apt-config/mysql-apt-config_0.8.36-1_all.deb && \
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
    ninja-build
