ARG FROM=centos:8
FROM ${FROM}

ARG DEBUG

RUN \
  quiet=$([ "${DEBUG}" = "yes" ] || echo "--quiet") && \
  dnf update -y ${quiet} && \
  { \
    echo "[mariadb]"; \
    echo "name = MariaDB"; \
    echo "baseurl = http://yum.mariadb.org/10.3/centos8-amd64"; \
    echo "gpgkey = https://yum.mariadb.org/RPM-GPG-KEY-MariaDB"; \
    echo "gpgcheck = 1"; \
  } | tee /etc/yum.repos.d/MariaDB.repo && \
  dnf install -y \
    https://packages.groonga.org/centos/groonga-release-latest.noarch.rpm \
    epel-release && \
  dnf groupinstall -y ${quiet} "Development Tools" && \
  dnf install --enablerepo=powertools -y ${quiet} \
    'dnf-command(builddep)' \
    'dnf-command(download)' && \
  # Install Judy-Devel directly.
  # Because dnf --enablerepo=powertools install Judy-devel is fail.
  # Probably, the metadata of the PowerTolls repository of CentOS8 has been broken.
  # Therefore, we install Judy-Devel from RPM.
  # We will remove this command when the metadata will be fixed.
  yum install -y ${quiet} \
    http://mirror.centos.org/centos/8/PowerTools/x86_64/os/Packages/Judy-devel-1.0.5-18.module_el8.3.0+757+d382997d.x86_64.rpm && \
  dnf builddep --enablerepo=powertools -y ${quiet} MariaDB && \
  dnf download -y ${quiet} --source MariaDB && \
  dnf install --enablerepo=powertools -y ${quiet} \
    ccache \
    gcc-c++ \
    groonga-devel \
    groonga-normalizer-mysql-devel \
    intltool \
    make \
    pkgconfig \
    sudo \
    wget \
    which && \
  dnf install --disablerepo=appstream -y ${quiet} MariaDB-devel && \
  dnf clean ${quiet} all
