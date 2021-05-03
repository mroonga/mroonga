ARG FROM=quay.io/centos/centos:stream8
FROM ${FROM}

ARG DEBUG

RUN \
  quiet=$([ "${DEBUG}" = "yes" ] || echo "--quiet") && \
  dnf update -y ${quiet} && \
  dnf install --enablerepo=powertools -y ${quiet} \
    'dnf-command(builddep)' \
    'dnf-command(download)' \
    https://packages.groonga.org/centos/groonga-release-latest.noarch.rpm \
    https://repo.mysql.com/mysql80-community-release-el8.rpm && \
  dnf builddep --enablerepo=powertools -y ${quiet} mysql-community && \
  dnf download -y ${quiet} --source mysql-community && \
  dnf install --enablerepo=powertools -y ${quiet} \
    ccache \
    groonga-devel \
    groonga-normalizer-mysql-devel \
    intltool \
    libtool \
    make \
    pkgconfig \
    rpm-build \
    wget \
    which && \
  dnf install --disablerepo=appstream -y ${quiet} \
    mysql-community-devel && \
  dnf clean ${quiet} all
