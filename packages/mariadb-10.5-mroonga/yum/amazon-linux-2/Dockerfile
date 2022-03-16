ARG FROM=amazonlinux:2
FROM ${FROM}

ARG DEBUG

RUN \
  quiet=$([ "${DEBUG}" = "yes" ] || echo "--quiet") && \
  amazon-linux-extras install -y ${quiet} epel mariadb10.5 && \
  yum install -y ${quiet} ca-certificates && \
  yum install -y ${quiet} \
    https://packages.groonga.org/amazon-linux/2/groonga-release-latest.noarch.rpm && \
  yum update -y ${quiet} && \
  yum groupinstall -y ${quiet} "Development Tools" && \
  yum-builddep -y ${quiet} mariadb && \
  yumdownloader -y ${quiet} --source mariadb && \
  yum install -y ${quiet} \
    ccache \
    gcc-c++ \
    groonga-devel \
    groonga-normalizer-mysql-devel \
    intltool \
    libtool \
    make \
    mariadb-devel \
    pkgconfig \
    sudo \
    wget \
    which && \
  yum clean ${quiet} all
