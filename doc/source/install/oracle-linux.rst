Oracle Linux
============

This section describes how to install Mroonga related RPM packages on
AlmaLinux. You can install them by ``dnf``.

.. _oracle-linux-8-oracle-8-0:

Oracle Linux 8 (with the Oracle MySQL 8.0 package)
--------------------------------------------------

You can use Oracle's MySQL packages version 8.0 on Oracle Linux 8 since
Mroonga 12.08 release.

.. note::

   There are already known issues about MySQL 8.0.

   * :doc:`/tutorial/wrapper` Wrapper mode is not supported yet
   * :doc:`/tutorial/storage`  Storage mode does not support the following feature.

     * The feature of relevant to the optimization.

Install::

  % sudo dnf -y module disable mysql
  % sudo dnf install -y https://packages.groonga.org/almalinux/8/groonga-release-latest.noarch.rpm
  % sudo dnf install -y https://repo.mysql.com/mysql80-community-release-el8.rpm
  % sudo dnf install --disablerepo=AppStream -y --enablerepo=ol8_codeready_builder mysql-community-8.0-mroonga
  (% sudo systemctl start mysqld)
  (% tmp_password=$(sudo grep 'A temporary password' /var/log/mysqld.log | sed -e 's/^.*: //'))
  (% sudo mysqladmin -u root --password="${tmp_password}" password)

If you want to use `MeCab <https://taku910.github.io/mecab/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo dnf install -y --enablerepo=ol8_codeready_builder groonga-tokenizer-mecab
