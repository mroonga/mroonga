#!/bin/bash

set -exu

package=$1

sudo apt update
sudo apt install -V -y lsb-release wget

distribution=$(lsb_release --id --short | tr 'A-Z' 'a-z')
code_name=$(lsb_release --codename --short)

wget https://packages.apache.org/artifactory/arrow/${distribution}/apache-arrow-apt-source-latest-${code_name}.deb
sudo apt install -y -V ./apache-arrow-apt-source-latest-${code_name}.deb
wget https://packages.groonga.org/${distribution}/groonga-apt-source-latest-${code_name}.deb
sudo apt install -y -V ./groonga-apt-source-latest-${code_name}.deb

sudo apt update
sudo apt install -V -y ${package}
sudo apt install -y -V groonga-tokenizer-mecab

sudo mysql -e "SHOW ENGINES" | grep Mroonga
