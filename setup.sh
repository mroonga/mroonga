#!/bin/bash
#
# Copyright(C) 2023  Sutou Kouhei <kou@clear-code.com>
# Copyright(C) 2024  Horimoto Yasuhiro <horimoto@clear-code.com>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

set -eux

if type sudo > /dev/null 2>&1; then
  SUDO=sudo
else
  SUDO=
fi

if type lsb_release > /dev/null 2>&1; then
  distribution=$(lsb_release --id --short | tr 'A-Z' 'a-z')
  code_name=$(lsb_release --codename --short)
else
  distribution=unknown
  code_name=unknown
fi

case "${distribution}-${code_name}" in
  debian-*|ubuntu-*)
    ${SUDO} apt update
    ${SUDO} apt install -y -V lsb-release wget
    deb=apache-arrow-apt-source-latest-${code_name}.deb
    wget https://apache.jfrog.io/artifactory/arrow/${distribution}/${deb}
    ${SUDO} apt install -y -V ./${deb}
    rm ${deb}
    ${SUDO} apt update
    ;;
  *)
    echo "This OS setup is not supported."
    exit 1
    ;;
esac

case "${distribution}-${code_name}" in
  debian-*|ubuntu-*)
    ${SUDO} apt install -y -V \
      ccache \
      cmake \
      curl \
      doxygen \
      g++ \
      gcc \
      gettext \
      graphviz \
      jq \
      libarrow-dev \
      libedit-dev \
      liblz4-dev \
      libmecab-dev \
      libmsgpack-dev \
      libsimdjson-dev \
      libstemmer-dev \
      libxxhash-dev \
      libzstd-dev \
      ninja-build \
      pkg-config \
      python3-pip \
      rapidjson-dev \
      zlib1g-dev
    ;;
esac

case "${distribution}-${code_name}" in
  debian-*)
    deb=groonga-apt-source-latest-${code_name}.deb
    wget https://packages.groonga.org/${distribution}/${deb}
    ${SUDO} apt install -y -V ./${deb}
    rm ${deb}
    ${SUDO} apt update
    ${SUDO} apt install -y -V libgroonga-dev
    ;;
  ubuntu-*)
    ${SUDO} apt install -y -V software-properties-common
    ${SUDO} add-apt-repository -y ppa:groonga/ppa
    ${SUDO} apt install -y -V libgroonga-dev
    ;;
esac

case "${distribution}-${code_name}" in
  debian-*|ubuntu-*)
    ${SUDO} apt install -y -V \
      libgroonga-dev \
      groonga-normalizer-mysql
    ;;
esac
