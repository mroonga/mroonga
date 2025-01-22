ARG FROM=almalinux:9
FROM ${FROM}

ARG DEBUG

ENV \
  SCL=gcc-toolset-12

RUN \
  quiet=$([ "${DEBUG}" = "yes" ] || echo "--quiet") && \
  dnf update -y ${quiet} && \
  dnf install --enablerepo=crb -y ${quiet} \
    'dnf-command(builddep)' \
    'dnf-command(download)' \
    https://packages.groonga.org/almalinux/9/groonga-release-latest.noarch.rpm \
    https://repo.mysql.com/mysql84-community-release-el9.rpm \
    https://apache.jfrog.io/artifactory/arrow/almalinux/$(cut -d: -f5 /etc/system-release-cpe | cut -d. -f1)/apache-arrow-release-latest.rpm && \
  dnf install -y ${quiet} \
    mysql-community-devel && \
  dnf install --enablerepo=crb -y ${quiet} \
    https://repo.mysql.com/mysql-community-minimal-release-el9.rpm && \
  dnf builddep --enablerepo=mysql-8.4-lts-community-minimal-source,crb -y ${quiet} mysql-community-minimal && \
  dnf download --enablerepo=mysql-8.4-lts-community-minimal-source -y ${quiet} --source mysql-community-minimal && \
  dnf install --enablerepo=crb -y ${quiet} \
    ${SCL} \
    ${SCL}-annobin-plugin-gcc \
    ccache \
    groonga-devel \
    groonga-normalizer-mysql-devel \
    ninja-build \
    pkgconfig \
    rpm-build \
    wget \
    which && \
  dnf clean ${quiet} all
