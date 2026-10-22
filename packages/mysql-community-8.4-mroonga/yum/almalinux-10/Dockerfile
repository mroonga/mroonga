ARG FROM=almalinux:10
FROM ${FROM}

ARG DEBUG

RUN \
  quiet=$([ "${DEBUG}" = "yes" ] || echo "--quiet") && \
  os_version=$(cut -d: -f5 /etc/system-release-cpe | cut -d. -f1) && \
  dnf update -y ${quiet} && \
  dnf install --enablerepo=crb -y ${quiet} \
    'dnf-command(builddep)' \
    'dnf-command(download)' \
    https://packages.groonga.org/almalinux/${os_version}/groonga-release-latest.noarch.rpm \
    https://repo.mysql.com/yum/mysql-8.4-community/el/${os_version}/x86_64/mysql84-community-release-el${os_version}-2.noarch.rpm \
    https://packages.apache.org/artifactory/arrow/almalinux/${os_version}/apache-arrow-release-latest.rpm && \
  dnf builddep --enablerepo=crb -y ${quiet} mysql-community && \
  dnf download -y ${quiet} --source mysql-community && \
  dnf install --enablerepo=crb -y ${quiet} \
    ccache \
    cmake \
    groonga-devel \
    groonga-normalizer-mysql-devel \
    intltool \
    libcurl-devel \
    libquadmath-devel \
    libtool \
    make \
    pkgconfig \
    rpm-build \
    wget \
    which && \
  dnf install --disablerepo=appstream -y ${quiet} \
    mysql-community-devel && \
  dnf clean ${quiet} all
