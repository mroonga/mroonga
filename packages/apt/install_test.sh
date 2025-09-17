#!/bin/bash

set -exu

package=$1

sudo apt update
sudo apt install -V -y lsb-release

distribution=$(lsb_release --id --short | tr 'A-Z' 'a-z')
code_name=$(lsb_release --codename --short)

case ${distribution} in
  debian)
    sudo apt install -y -V wget
    wget https://apache.jfrog.io/artifactory/arrow/debian/apache-arrow-apt-source-latest-${code_name}.deb
    sudo apt install -y -V ./apache-arrow-apt-source-latest-${code_name}.deb
    wget https://packages.groonga.org/debian/groonga-apt-source-latest-${code_name}.deb
    sudo apt install -y -V ./groonga-apt-source-latest-${code_name}.deb
  ;;
  ubuntu)
    sudo apt install -y -V software-properties-common lsb-release
    sudo add-apt-repository -y universe
    sudo add-apt-repository \
         "deb http://security.ubuntu.com/ubuntu $(lsb_release --short --codename)-security main restricted"
    sudo add-apt-repository -y ppa:groonga/ppa
  ;;
esac

sudo apt update

sudo apt install -V -y ${package}
sudo apt install -y -V groonga-tokenizer-mecab

sudo mysql -e "SHOW ENGINES" | grep Mroonga
