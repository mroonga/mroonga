Amazon Linux
============

This section describes how to install Mroonga related RPM packages on
Amazon Linux. You can install them by ``yum``.

.. _amazon-linux-2-mariadb-10-5:

Amazon Linux 2 (with MariaDB 10.5 package)
------------------------------------------

Install::

  % sudo amazon-linux-extras install -y epel mariadb10.5
  % sudo yum install -y https://packages.groonga.org/amazon-linux/2/groonga-release-latest.noarch.rpm
  % sudo systemctl start mariadb
  % sudo yum install -y --enablerepo=epel mariadb-10.5-mroonga
  (% sudo mysqladmin -u root password 'new-password')

If you want to use `MeCab <https://taku910.github.io/mecab/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo yum install -y --enablerepo=epel groonga-tokenizer-mecab
