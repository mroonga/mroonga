:orphan:

News
====

.. _release-12-05:

Release 12.05 - 2022-07-29
--------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/install/centos`] Added support for MySQL 5.7.39.

* [:doc:`/install/centos`][:doc:`/install/almalinux`] Added support for MySQL 8.0.39.

* [:doc:`/install/centos`][:doc:`/install/almalinux`] Added support for Percona Server 8.0.28-20.

* [:doc:`/install/centos`][:doc:`/install/almalinux`] Added support for MariaDB 10.8.

* [:doc:`/install/centos`][:doc:`/install/almalinux`] Dropped support for MariaDB 10.2.

* [:doc:`/install/debian`] Dropped support for Debian 10 (buster).

.. _release-12-04:

Release 12.04 - 2022-06-01
--------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/reference/server_variables`] Add a new status variable ``Mroonga_memory_map_size``.

  We can get the total memory map size in bytes of Mroonga as below.

  .. code-block::

     mysql> SHOW STATUS LIKE 'Mroonga_memory_map_size';
     +-------------------------+----------+
     | Variable_name           | Value    |
     +-------------------------+----------+
     | Mroonga_memory_map_size | 83406848 |
     +-------------------------+----------+
     1 row in set (0.00 sec)

  In Windows, If Mroonga uses up physical memory and swap area, Mroonga can't more mapping memory than that.
  Therefore, we can control properly memory map size by monitoring this value even if the environment does have not enough memory.

* [:doc:`/install/centos`][:doc:`/install/almalinux`] Added support for Percona Server 8.0.28-19.

* [:doc:`/install/centos`][:doc:`/install/almalinux`] Added support for Percona Server 5.7.38-41.

* [:doc:`/install/centos`][:doc:`/install/almalinux`] Added support for MariaDB 10.2.44, 10.3.35, 10.4.25, 10.5.16, 10.6.8, and 10.7.4.

Fixes
^^^^^

* Fixed a bug that Mroonga may update failed. [groonga-dev,04982, groonga-dev,04987][Reported by Mitsuo Yoshida and OHTSUKA Soushi]

  If this bug occurs, Mroonga is disabled after Mroonga update with such as "apt update".
  In that case, we install Mroonga manually with the following procedure.

  .. code-block::

     % mysql -u root < /usr/share/mroonga/install.sql

Thanks
^^^^^^

* Mitsuo Yoshida
* OHTSUKA Soushi

.. _release-12-03:

Release 12.03 - 2022-05-06
--------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/install/centos`] Added support for Percona Server 5.7.37-40.

* [:doc:`/install/centos`] Added support for MySQL 5.7.38.

* [:doc:`/install/centos`][:doc:`/install/almalinux`] Added support for MySQL 8.0.29.

Fixes
^^^^^

* Fixed a bug that Mroonga may fail create the index on MariaDB 10.5.14. [GitHub clear-code/redmine_full_text_search#103][Reported by wate]

* Fixed a memory leak on full text search. [Reported by OHTSUKA Soushi and Mitsuo Yoshida]

  This is occurred when `order limit optimization <https://mroonga.org/ja/docs/reference/optimizations.html#order-by-limit>`_ is used.
  However, if we use MariaDB, this occurs even if we don't use order limit optimization.

  This bug had occurred since Mroonga 11.03.

Thanks
^^^^^^

* wate
* OHTSUKA Soushi
* Mitsuo Yoshida

.. _release-12-02:

Release 12.02 - 2022-03-29
--------------------------

Improvements
^^^^^^^^^^^^

* Dropped support wrapper mode with MySQL 8.0 or later.

* Added support for disabling a back trace by the server variable.

  We can disable a back trace by "SET GLOBAL mroonga_enable_back_trace = false;".

* Added support for ``float32`` weight vector.

  We can store weight as ``float32``.
  We need to add ``WEIGHT_FLOAT32`` flag when we define a column to use this feature.

* [:doc:`/install/centos`][:doc:`/install/almalinux`] Added support for MariaDB 10.3.34, 10.4.24, 10.5.15, 10.6.7, and 10.7.3.

* [:doc:`/install/centos`] Added support for MariaDB 10.2.43.

* [:doc:`/install/centos`][:doc:`/install/almalinux`] Added support for Percona Server 8.0.27-18.

* Added support for ``MISSING_*`` and ``INVALID_*`` flags

  Please refer to https://groonga.org/docs/news.html#release-12-0-2 about details of these flags.

.. _release-12-00:

Release 12.00 - 2022-02-09
--------------------------

This is a major version up!
But It keeps backward compatibility. We can upgrade to 12.00 without rebuilding database.

First of all, we introduce the Summary of changes from Mroonga 11.00 to 11.13.
Then, we introduce the main changes in 12.00.

Summary of changes from Mroonga 11.0.0 to 11.1.3
------------------------------------------------

New Features and Improvements
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

* Renamed package names as below.

  * ``mariadb-server-10.x-mroonga`` -> ``mariadb-10.x-mroonga``
  * ``mysql-server-5.x-mroonga`` -> ``mysql-community-5.x-mroonga``
  * ``mysql-server-8.x-mroonga`` -> ``mysql-community-8.x-mroonga``
  * ``percona-server-5x-mroonga`` -> ``percona-server-5.x-mroonga``
  * ``percona-server-8x-mroonga`` -> ``percona-server-8.x-mroonga``

  .. warning::

     The package names are changed.
     Mroonga may be invalid after upgrade by the influence of this modification.
     If we upgrade to this version, please always be sure to confirm the below points.

     If Mroonga is invalid after the upgrade, we need to install manually Mroonga again.
     Please refer to the following URL about the manual installation of Mroonga and how to confirming whether Mroonga is valid or not.

       * https://mroonga.org/docs/tutorial/installation_check.html

     If we will upgrade mroonga to stride over a Mroonga 11.03.

     If Mroonga is valid after upgrade to this version but, Mroonga's version is old, we need to restart MySQL, MariaDB, or PerconaServer.
     We can confirm Mroonga's version as the below command.

       * ``SHOW VARIABLES LIKE 'mroonga_version';``

* [:doc:`/reference/udf/mroonga_snippet_html`] Added support for custom normalizer in ``mroonga_snippet_html()``

  * We can use custom normalizer in ``mroonga_snippet_html()`` by this feature as below.

    .. code-block::

       CREATE TABLE terms (
         term VARCHAR(64) NOT NULL PRIMARY KEY
       ) COMMENT='normalizer "NormalizerNFKC130(''unify_kana'', true)"'
         DEFAULT CHARSET=utf8mb4
         COLLATE=utf8mb4_unicode_ci;

       SELECT mroonga_snippet_html('これはMroonga（ムルンガ）です。',
                                   'terms' as lexicon_name,
                                   'むるんが') as snippet;

       snippet
       <div class="snippet">これはMroonga（<span class="keyword">ムルンガ</span>）です。</div>

* [:doc:`/reference/server_variables`] We disabled ``mroonga_enable_operations_recording`` by default.

  ``mroonga_enable_operations_recording`` to determine whether recording operations for auto recovering are enabled or not. 

  This recording of operations is for auto recovering Mroonga when it crashed.
  Normally, if lock remain in Mroonga, we can't execute INSERT/DELETE/UPDATE, but if ``mroonga_enable_operations_recording`` is enable, we may not execute SELECT at times in addition to INSERT/DELETE/UPDATE.
  Because auto recovery is sometimes blocked by residual lock when they crashed.

  Therefore, we set ``OFF`` to the default value in this version.
  By we disable operation recording, INSERT/DELETE/UPDATE is blocked as usual because of the residual lock, but "SELECT" may bework.

  An appropriate way to handle to residual lock is as follows.

    * https://www.clear-code.com/blog/2021/6/1/mroonga-recover-lock-failed-2021.html
      (Japanese only)

Fixes
^^^^^

* Fix a crash bug that may be caused after MySQL/MariaDB upgrade.

  * Mronnga may crash if we execute ``SELECT ... MATCH AGAINST`` after MySQL/MariaDB upgrade.

* Fixed a bug that if we use "WHERE primary_key IN ("")" in a where clause, Mroonga may return wrong record.

  See :ref:`release 11.07 <release-11-07>` for details.

* [:doc:`/reference/optimizations`] Fixed a bug that Mroonga apply the optimization of row count wrongly.

  See :ref:`release 11.10 <release-11-10>` for details.

* Fixed a bug that Mroonga crashed when we upgrade DB created by MySQL 5.7 to MySQL 8.0.

* Fixed a bug that latitude and longitude are stored conversely.

  .. warning::

     backward compatibility is broken by this fix.

     Users that are using ``GEOMETRY`` type need to store the current data before upgrading to Mroonga 11.03 and restore the stored data after upgrading to Mroonga 11.03.
     Users can use the following methods for dumping/restoring data.

       * ``mysqldump``
       * Execute ``ALTER TABLE ENGINE=InnoDB`` before upgrading and execute ``ALTER TABLE ENGINE=Mroonga`` after upgrading.

     If without this fix, ``INSERT/UPDATE/SELECT/SELECT`` works well but data stored in Groonga are wrong (Latitude and longitude are swapped in Groonga). Therefore, ``mroonga_command('select ...')`` doesn't work for spatial data.

* Fixed a bug that FOREIGN KEY constraint was not registered.

  This bug had only occurred on MySQL 8.0.

  See :ref:`release 11.01 <release-11-01>` for details.

* Fixed a bug that ``DROP DATABASE`` had failed if a target database had FOREIGN KEY constraint.

  See :ref:`release 11.01 <release-11-01>` for details.

* Fixed a bug that ``DROP COLUMN`` had failed if a target table was referred a other table.

  See :ref:`release 11.01 <release-11-01>` for details.

* Fixed a bug that a update of Mroonga fails on MariaDB.

Newly supported OSes
^^^^^^^^^^^^^^^^^^^^

* [:doc:`/install/debian`] Added support for Debian 11 (bullseye).

* [:doc:`/install/almalinux`] Added support for Mroonga on AlamLinux 8.

Dropped OSes
^^^^^^^^^^^^

* [:doc:`/install/centos`] Dropped support for CentOS 8.

* [:doc:`/install/ubuntu`] Dropped Ubuntu 21.04 (Hirsute Hippo) support.

* [:doc:`/install/ubuntu`] Dropped Ubuntu 20.10 (Groovy Gorilla) support.

* [:doc:`/install/ubuntu`] Dropped Ubuntu 16.04 LTS (Xenial Xerus) support.

* [:doc:`/install/ubuntu`] Dropped support for MariaDB 10.1 on Ubuntu 18.04 LTS.

Thanks
^^^^^^

* shibanao4870
* Marc Laporte
* santalex
* Josep Sanz
* Tomohiro KATO
* Katsuhito Watanabe
* kenichi arimoto
* Vincent Pelletier
* Kosuke Yamashita
* ひじー

The main changes in 12.00 are as follows.

Improvements
^^^^^^^^^^^^

* [:doc:`/install/ubuntu`] Added support for the latest version of mysql-server package for Ubuntu.

  We supported the following versions.

  * Ubuntu 18.04 LTS (bionic) mysql-server (5.7.37-0ubuntu0.18.04.1)
  * Ubuntu 20.04 LTS (focal) mysql-server (8.0.28-0ubuntu0.20.04.3)
  * Ubuntu 21.10 LTS (impish) mysql-server (8.0.28-0ubuntu0.21.10.3)

* [:doc:`/install/centos`] Added support for MariaDB 10.2.42, 10.3.33, 10.4.23, 10.5.14, and 10.6.6.

* [:doc:`/install/almalinux`] Added support for MariaDB 10.3.33, 10.4.23, 10.5.14, and 10.6.6.

.. _release-11-13:

Release 11.13 - 2022-01-29
--------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/install/centos`] Added support for MySQL 5.7.37.

* [:doc:`/install/centos`][:doc:`/install/almalinux`] Added support for MySQL 8.0.28.

  There are below restrictions in the MySQL 8.0 package.

    * [:doc:`/tutorial/wrapper`] Wrapper mode is not supported yet.
    * [:doc:`/tutorial/storage`] Storage mode does not support the following feature.

      * The feature of relevant to the optimization.

* [:doc:`/install/centos`][:doc:`/install/almalinux`] Added support for Percona Server 8.0.26-17.

* [:doc:`/install/ubuntu`] Dropped Ubuntu 21.04 (Hirsute Hippo) support.

  * Because Ubuntu 21.04 reached EOL at January 20, 2022.

.. _release-11-11:

Release 11.11 - 2021-12-29
--------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/install/centos`] Dropped support for CentOS 8.

  Because CentOS 8 will reach EOL at 2021-12-31.

Fixes
^^^^^

* [:doc:`/install/almalinux`] Fixed a bug that we have not provided the package of Mroonga for MariaDB 10.3, 10.4, 10.5, and 10.6. [Gitter][Reported by shibanao4870]

* [Documentation][:doc:`/install`] Fixed a typo. [GitHub #469,#470][Patched by Marc Laporte]

* [Documentation][:doc:`/install/centos`] Fixed the wrong URL of ``percona-release-latest.noarch.rpm``. [GitHub #471][Patched by santalex]

Thanks
^^^^^^

* shibanao4870

* Marc Laporte

* santalex

.. _release-11-10:

Release 11.10 - 2021-11-29
--------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/install/almalinux`] Added support for Mroonga on AlamLinux 8.

 * [:doc:`/install/almalinux`] Added support for MySQL 8.0.27.

    * There are below restrictions in the MySQL 8.0 package.

      * [:doc:`/tutorial/wrapper`] Wrapper mode is not supported yet.
      * [:doc:`/tutorial/storage`] Storage mode does not support the following feature.

        * The feature of relevant to the optimization.

  * [:doc:`/install/almalinux`] Added support for Percona Server 8.0.26.

  * [:doc:`/install/almalinux`] Added support for MariaDB 10.3.32, 10.4.22, 10.5.13, and 10.6.5.

* [:doc:`/install/centos`] Added support for MariaDB 10.2.41, 10.3.32, 10.4.22, 10.5.13, and 10.6.5.

* [:doc:`/install/ubuntu`] Added support for MySQL 8.0 on Ubuntu 21.04 (Hirsute Hippo) and 21.10 (Impish Indri).

  * There are below restrictions in the MySQL 8.0 package.

    * [:doc:`/tutorial/wrapper`] Wrapper mode is not supported yet.
    * [:doc:`/tutorial/storage`] Storage mode does not support the following feature.

      * The feature of relevant to the optimization.

Fixes
^^^^^

* [:doc:`/install/centos`] Fixed a bug that we have not provided the package of Mroonga for MariaDB 10.6.

* [:doc:`/reference/optimizations`] Fixed a bug that Mroonga apply the optimization of row count wrongly.
  [MDEV-16922][Reported by Josep Sanz]

  Normally, Mroonga apply the optimization of row count when the ``SELECT`` fetches only ``COUNT(*)`` and
  condition in ``WHERE`` can be processed only by index.

  However, Mroonga applied the optimization of row count even if Mroonga couldn't process condition
  in ``WHERE`` only by index as below case by this bug.

  Consequently, the result of ``SELECT COUNT(*) WHERE ...`` is wrong.

  .. code-block::

     CREATE TABLE roles (
       id INT
     );

     INSERT INTO roles VALUES (1), (2), (3), (4), (5);

     CREATE TABLE users (
       id INT,
       role_id INT,
       INDEX (role_id)
     );

     INSERT INTO users VALUES (10, 1);
     INSERT INTO users VALUES (11, 2);
     INSERT INTO users VALUES (13, 3);
     INSERT INTO users VALUES (14, 4);
     INSERT INTO users VALUES (15, 5);
     INSERT INTO users VALUES (20, 1);
     INSERT INTO users VALUES (21, 2);
     INSERT INTO users VALUES (23, 3);
     INSERT INTO users VALUES (24, 4);
     INSERT INTO users VALUES (25, 5);

     SELECT COUNT(*)
       FROM (
         SELECT roles.id
           FROM roles
                LEFT JOIN users ON users.id <= 100 AND
                                   users.role_id = roles.id
       ) roles_users;

* Fixed a bug that Mroonga doesn't set proper encoding on condition push down for 'STRING_FIELD ='. [groonga-dev,04913][Reported by Tomohiro 'Tomo-p' KATO]

  Mroonga executes condition push down for 'STRING_FIELD =' in the following case.
  However, Mroonga doesn't set proper encoding in search keywords at this time.
  Consequently, Mroonga fails to normalize search keywords.

  .. code-block::

    CREATE TABLE memos (
      id INT PRIMARY KEY,
      title TEXT,
      INDEX (title)
    ) ENGINE=Mroonga DEFAULT CHARSET=utf8mb4;

    INSERT INTO memos VALUES (1, 'Groonga');
    INSERT INTO memos VALUES (2, 'Mroonga');

    SELECT *
      FROM memos
      WHERE title = 'mroonga'
      ORDER BY id;

Thanks
^^^^^^

* Josep Sanz

* Tomohiro KATO

.. _release-11-09:

Release 11.09 - 2021-11-04
--------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/install/centos`] Added support for MySQL 5.7.36, 8.0.27.

* [:doc:`/install/centos`] Added support for Percona Server 8.0.26.

.. _release-11-08:

Release 11.08 - 2021-10-06
--------------------------

Fixes
^^^^^

* Fixed a bug that Mroonga crashed when we upgrade DB created by MySQL 5.7 to MySQL 8.0. 

.. _release-11-07:

Release 11.07 - 2021-09-29
--------------------------

  .. warning::

     Mroonga has had a bug that if we upgrade DB created by MySQL 5.7 to MySQL 8.0, Mroonga crashes.

     We fixed this bug on Mroonga 11.08.
     Therefore, if you were using Mroonga on MySQL 5.7 or previous version,
     we highly recommended that we use Mroonga 11.08 or later.

Improvements
^^^^^^^^^^^^

* [:doc:`/install/ubuntu`] Added support for MySQL 8.0 on Ubuntu 20.04 (Focal Fossa).

  * There are below restrictions in the MySQL 8.0 package.

    * [:doc:`/tutorial/wrapper`] Wrapper mode is not supported yet.
    * [:doc:`/tutorial/storage`] Storage mode does not support the following feature.

      * The feature of relevant to the optimization.

* [:doc:`/reference/udf/mroonga_snippet_html`] Added support for specifying lexicon name.

  * We can use custom normalizer in ``mroonga_snippet_html()`` by this feature as below.

    .. code-block::

       CREATE TABLE terms (
         term VARCHAR(64) NOT NULL PRIMARY KEY
       ) COMMENT='normalizer "NormalizerNFKC130(''unify_kana'', true)"'
         DEFAULT CHARSET=utf8mb4
         COLLATE=utf8mb4_unicode_ci;

       SELECT mroonga_snippet_html('これはMroonga（ムルンガ）です。',
                                   'terms' as lexicon_name,
                                   'むるんが') as snippet;

       snippet
       <div class="snippet">これはMroonga（<span class="keyword">ムルンガ</span>）です。</div>

* Added support for outputting vector column values as text not raw binary.

  * We can use ``mysqldump`` for dumping vector column values by this feature.

* Don't create .mrn files if nonexistent table is dropped. [groonga-dev: 04893][Reported by kenichi arimoto]

* Added support for W pragma against elements of vector column.

  * We can set the weight to elements of vector column by this feature.
    However, Mroonga only search the specified section in this case.
    In a normal multiple column index, Mroonga also searches not specified sections with the default weight.

Fixes
^^^^^

* Fixed a bug that if we use "WHERE primary_key IN ("")" in a where clause, Mroonga may return wrong record. [groonga-dev,04855][Reported by Katsuhito Watanabe]

  * For example, Mroonga may return wrong record as below.

    .. code-block::

       CREATE TABLE ids (
         id varchar(7) PRIMARY KEY,
         parent_id varchar(7)
       )ENGINE=Mroonga DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

       INSERT INTO ids VALUES("abcdefg", "");
       INSERT INTO ids VALUES("hijklmn", "");
       INSERT INTO ids VALUES("opqrstu", "hijklmn");

       SELECT * FROM ids WHERE id IN (SELECT parent_id FROM ids);
       +---------+-----------+
       | id      | parent_id |
       +---------+-----------+
       | abcdefg |           |
       | hijklmn |           |
       +---------+-----------+
       2 rows in set (0.00 sec)

Thanks
^^^^^^

* Katsuhito Watanabe

* kenichi arimoto

.. _release-11-06:

Release 11.06 - 2021-08-29
--------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/install/centos`] Added support for MariaDB 10.2.40, 10.3.31, 10.4.21, and 10.5.12

* [:doc:`/install/centos`] Added support for MariaDB 10.6.4 [GitHub#434][Patched by Tomohiro KATO]

* [:doc:`/install/centos`] Added support for Percona Server 5.7.35.

* [:doc:`/install/debian`] Added support for Debian 11 (bullseye).

Thanks
^^^^^^

* Tomohiro KATO

.. _release-11-05:

Release 11.05 - 2021-07-30
--------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/install/centos`] Added support for Percona Server 8.0.25

* [:doc:`/install/centos`] Added support for MySQL 5.7.35, 8.0.26.

  * There are below restrictions in the MySQL 8.0 package.

    * [:doc:`/tutorial/wrapper`] Wrapper mode is not supported yet.
    * [:doc:`/tutorial/storage`] Storage mode does not support the following feature.

      * The feature of relevant to the optimization.

* [:doc:`/install/ubuntu`] Dropped Ubuntu 20.10 (Groovy Gorilla) support.

  * Because Ubuntu 20.10 reached EOL July 22, 2021.

Fixes
^^^^^

* Fix a crash bug that may be caused after MySQL/MariaDB upgrade. [GitHub#423][Reported by Vincent Pelletier]

  * Mronnga may crash if we execute ``SELECT ... MATCH AGAINST`` after MySQL/MariaDB upgrade.

Thanks
^^^^^^

* Vincent Pelletier

.. _release-11-04:

Release 11.04 - 2021-06-29
--------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/reference/server_variables`] We disabled ``mroonga_enable_operations_recording`` by default.

  ``mroonga_enable_operations_recording`` to determine whether recording operations for auto recovering are enabled or not. 

  This recording of operations is for auto recovering Mroonga when it crashed.
  Normally, if lock remain in Mroonga, we can't execute INSERT/DELETE/UPDATE, but if ``mroonga_enable_operations_recording`` is enable, we may not execute SELECT at times in addition to INSERT/DELETE/UPDATE.
  Because auto recovery is sometimes blocked by residual lock when they crashed.

  Therefore, we set ``OFF`` to the default value in this version.
  By we disable operation recording, INSERT/DELETE/UPDATE is blocked as usual because of the residual lock, but "SELECT" may bework.

  An appropriate way to handle to residual lock is as follows.

    * https://www.clear-code.com/blog/2021/6/1/mroonga-recover-lock-failed-2021.html
      (Japanese only)

* [:doc:`/install/debian`] Added a install procedure for Mroonga for Debian GNU/Linux with Oracle MySQL package.

  We supoort Mroonga for Debian GNU/Linux with Oracle MySQL package in Mroonga 11.03.
  We wrote install procedure for these package to this documentation.
  When we will install these packages, please refer to this documentation.

* [:doc:`/install/centos`] Added support for MariaDB 10.2.39, 10.3.30, 10.4.20, and 10.5.11.

Fixes
^^^^^

* Fix a crash bag when we execute search

  **This issue doesn't occur under normal use.**

  The occurrence condition is as below.

    * We directly make tables and columns on Groonga by using ``mroonga_command``.
    * We execute search at the same time as delete the above tables and columns.

  When the above conditions are established, Mroonga might crash.

.. _release-11-03:

Release 11.03 - 2021-05-31
--------------------------

  .. warning::

     This release has had a critical bug about uninstall and upgrade.
     If we install this version, we fail upgrade Mroonga and also uninstall it.

     Therefore, please don't use Mroonga of this version.

     If we have already installed Mroonga of this version, we can upgrade Mroonga or uninstall it by using the following workaround. 

       1. % echo "#\!/bin/sh" > /usr/share/mroonga/deb/postrm.sh
       2. % chmod u+x /usr/share/mroonga/deb/postrm.sh
       3. Upgrade Mroonga or uninstall Mroonga.

  .. warning::

     The package names are changed from this release.
     Mroonga may be invalid after upgrade to this version by the influence of this modification.
     If we upgrade to this version, please always be sure to confirm the below points.

     If Mroonga is invalid after the upgrade, we need to install manually Mroonga again.
     Please refer to the following URL about the manual installation of Mroonga and how to confirming whether Mroonga is valid or not.

       * https://mroonga.org/docs/tutorial/installation_check.html

     Besides, please be careful the above phenomenon will continue from now if we will upgrade mroonga to stride over a Mroonga 11.03.

     If Mroonga is valid after upgrade to this version but, Mroonga's version is old, we need to restart MySQL, MariaDB, or PerconaServer.
     We can confirm Mroonga's version as the below command.

       * ``SHOW VARIABLES LIKE 'mroonga_version';``

  .. warning::

       There are broken backward compatibility on this version.

       Users that are using GEOMETRY type need to store the current data
       before upgrading to Mroonga 11.03 and restore the stored data
       after upgrading to Mroonga 11.03.

       Please be careful if we upgrade to this version without executing the above procedure,
       data is broken.

       Users can use the following methods for dumping/restoring data.

         * mysqldump
         * Execute ALTER TABLE ENGINE=InnoDB before upgrading and
           execute ALTER TABLE ENGINE=Mroonga after upgrading.

       If without this fix, INSERT/UPDATE/SELECT/SELECT works well
       but data stored in Groonga are wrong (Latitude and longitude are swapped in Groonga).
       Therefore, mroonga_command('select ...') doesn「t work for spatial data.

Improvements
^^^^^^^^^^^^

* Renamed package names as below.

  * ``mariadb-server-10.x-mroonga`` -> ``mariadb-10.x-mroonga``
  * ``mysql-server-5.x-mroonga`` -> ``mysql-community-5.x-mroonga``
  * ``mysql-server-8.x-mroonga`` -> ``mysql-community-8.x-mroonga``
  * ``percona-server-5x-mroonga`` -> ``percona-server-5.x-mroonga``
  * ``percona-server-8x-mroonga`` -> ``percona-server-8.x-mroonga``

* [:doc:`/install/debian`] Added support for the Oracle MySQL 5.7 package and the Oracle MySQL 8.0 package.

  * In this release, we newly provided the package of Mroonga for Debian GNU/Linux with the Oracle MySQL 5.7 and the Oracle MySQL 8.0.

    * By this, we make the Docker image of Mroonga with MySQL for Docker official images.

  * There are below restrictions in the MySQL 8.0 package.

    * [:doc:`/tutorial/wrapper`] Wrapper mode is not supported yet.
    * [:doc:`/tutorial/storage`] Storage mode does not support the following feature.

      * The feature of relevant to the optimization.

* Added support for the SRID of Spatial Indexes.

  * the index of Mroonga is used in search with ``MBRContains`` function on MySQL 8.x since this release.

* We added condition expressions that are applied condition push down.

  .. note::

     Condition push down doesn't evaluate condition expressions in MySQL, but it evaluate condition expressions in Mroonga.
     Therefore, more queries will become high-performance, but the result of search may different than usual by Mroonga evaluate them.
     If Mroonga returns unexpected search results, we request that you report from the below URL.
     And we would like you to invalid for condition push down.

       * https://github.com/mroonga/mroonga/issues/

     We can invalid condition push down as below.

       * ``SET GLOBAL mroonga_condition_push_down_type = "NONE"``

* [:doc:`/install/centos`] Added support for Percona Server 8.0.23

Fixes
^^^^^

* Fixed a bug that latitude and longitude are stored conversely.

  .. warning::

     backward compatibility is broken by this fix.

     Users that are using ``GEOMETRY`` type need to store the current data before upgrading to Mroonga 11.03 and restore the stored data after upgrading to Mroonga 11.03.
     Users can use the following methods for dumping/restoring data.

       * ``mysqldump``
       * Execute ``ALTER TABLE ENGINE=InnoDB`` before upgrading and execute ``ALTER TABLE ENGINE=Mroonga`` after upgrading.

     If without this fix, ``INSERT/UPDATE/SELECT/SELECT`` works well but data stored in Groonga are wrong (Latitude and longitude are swapped in Groonga). Therefore, ``mroonga_command('select ...')`` doesn't work for spatial data.

.. _release-11-02:

Release 11.02 - 2021-05-10
--------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/install/centos`] Added support for MySQL 5.7.34, 8.0.25.

  * There are below restrictions in the MySQL 8.0 package.

    * [:doc:`/tutorial/wrapper`] Wrapper mode is not supported yet.
    * [:doc:`/tutorial/storage`] Storage mode does not support the following feature.

      * The feature of relevant to the optimization.
      * The SRID of Spatial Indexes.

        * For example, the index of Mroonga is not used in search with ``MBRContains`` function.
          (It search by sequential search.)

* [:doc:`/install/centos`] Added support for MariaDB 10.2.38, 10.3.29, 10.4.19, and 10.5.10.

* [:doc:`/install/ubuntu`] Dropped Ubuntu 16.04 (Xenial Xerus) support.

  * Because it reached End of Standard Support at April, 2021.

.. _release-11-01:

Release 11.01 - 2021-04-02
--------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/install/centos`] Added support for MariaDB 10.2.37, 10.3.28, 10.4.18, and 10.5.9.

* [:doc:`/install/centos`] Added support for Percona Server 5.7.33.

* Added support for adding value with text in JSON format to reference vector columns as below.

  * It makes easier to add JSON data into reference columns by this feature.
    Because we can directly add values to columns for reference destination from the source table.

  .. code-block::

     CREATE TABLE attributes (
       _id int,
       name varchar(255),
       value varchar(255)
     ) DEFAULT CHARSET=utf8mb4;

     CREATE TABLE items (
       id int PRIMARY KEY AUTO_INCREMENT,
       attributes text DEFAULT NULL flags='COLUMN_VECTOR' groonga_type='attributes'
     );

     INSERT INTO items (attributes)
       VALUES ('[{"name": "color", "value": "white"},
                 {"name": "size",  "value": "big"}]');
     INSERT INTO items (attributes)
       VALUES ('[{"name": "color", "value": "black"}]');
     INSERT INTO items (attributes) VALUES ('');

     SELECT * FROM attributes;
       _id	name	value
       1	color	white
       2	size	big
       3	color	black

     SELECT * FROM items;
     id	attributes
     1	[1,2]
     2	[3]
     3	[]

Fixes
^^^^^

* Fixed a bug that FOREIGN KEY constraint was not registered. [GitHub#393][Reported by Kosuke Yamashita]

  * This bug had only occurred on MySQL 8.0.
  * For example, the FOREIGN KEY constraint information had not been outputted even if we define it as below.

    .. code-block::

       CREATE TABLE referred (
         id int PRIMARY KEY AUTO_INCREMENT
       );

       CREATE TABLE refer (
         id int PRIMARY KEY AUTO_INCREMENT,
         id_referred int NOT NULL,
         CONSTRAINT id_referred FOREIGN KEY (id_referred) REFERENCES referred (id)
       );

       SELECT CONSTRAINT_NAME, TABLE_NAME, REFERENCED_TABLE_NAME
              FROM information_schema.REFERENTIAL_CONSTRAINTS;
       Empty set (0.000 sec)

* Fixed a bug that ``DROP DATABASE`` had failed if a target database had FOREIGN KEY constraint as below. [GitHub#390][Reported by Kosuke Yamashita]

  .. code-block::

    CREATE DATABASE another;
    USE another;

    CREATE TABLE referred (
      id int PRIMARY KEY AUTO_INCREMENT
    ) ENGINE=mroonga DEFAULT CHARSET utf8mb4;

    CREATE TABLE refer (
      id int PRIMARY KEY AUTO_INCREMENT,
      id_referred int NOT NULL,
      CONSTRAINT id_referred FOREIGN KEY (id_referred) REFERENCES referred (id)
    ) ENGINE=mroonga DEFAULT CHARSET utf8mb4;

    DROP DATABASE another;
    ERROR 1016 (HY000): [table][remove] a column that references the table exists: <refer.id_referred> -> <referred>

* Fixed a bug that ``DROP COLUMN`` had failed if a target table was referred a other table as below. [GitHub#389][Reported by Kosuke Yamashita]

  .. code-block::

     CREATE TABLE referred (
       id int PRIMARY KEY AUTO_INCREMENT,
       name varchar(255),
       text text
     ) ENGINE=mroonga DEFAULT CHARSET utf8mb4;

     CREATE TABLE refer (
       id int PRIMARY KEY AUTO_INCREMENT,
       id_referred int NOT NULL,
       CONSTRAINT id_referred FOREIGN KEY (id_referred) REFERENCES referred (id)
     ) ENGINE=mroonga DEFAULT CHARSET utf8mb4;

     ALTER TABLE referred DROP COLUMN name;
     ERROR 1016 (HY000): [table][remove] a column that references the table exists: <refer.id_referred> -> <#sql2-3bc-25>

* Fixed a build error when we built Mroonga with MariaDB 10.3.28, 10.4.18, or 10.5.9. [GitHub#392][Patched by Tomohiro KATO]

* Fixed a bug that a update of Mroonga fails on MariaDB. [Reported by ひじー]

Thanks
^^^^^^

* Kosuke Yamashita

* Tomohiro KATO

* ひじー

.. _release-11-00:

Release 11.00 - 2021-02-09
--------------------------

This is a major version up! But It keeps backward compatibility.
You can upgrade to 11.00 without rebuilding database.

In this version, MySQL, MariaDB, or PerconaServer will be automatically restarted.
Because Mroonga 11.00 requires Groonga 11.0.0 or later but it will not reloaded
until MySQL, MariaDB, or PrerconaServer is restarted.

Improvements
^^^^^^^^^^^^

* [:doc:`/install/centos`] Dropped support for MySQL 5.6

  * Because it reached EOL at Feb 1, 2021.

* [:doc:`/install/centos`] Dropped support for Percona Server 5.6

* Dropped support for MariaDB 10.1 on Ubuntu 18.04 LTS.

  * Because MariaDB 10.1 already has reached EOL in upstream.

* Updated version of Groonga to 11.0.0 or later that Mroonga requires.

  * Because a high impact bug of index corruption is fixed in Groonga 11.0.0.

.. _release-10-11:

Release 10.11 - 2021-01-29
--------------------------

Improvements
^^^^^^^^^^^^

* Added support for ``lexicon_flags`` parameter.

  * We can add ``KEY_LARGE`` flag to a ``USING HASH`` (non full-text search index) index as below by this parameter.

    .. code-block::

      CREATE TABLE memos (
        id INT UNSIGNED PRIMARY KEY,
        title VARCHAR(64) NOT NULL,
        UNIQUE INDEX (title) USING HASH COMMENT 'lexicon_flags "KEY_LARGE"'
      ) DEFAULT CHARSET=utf8mb4;

* [:doc:`/install/centos`] Added support for MySQL 5.6.51, 5.7.33, and 8.0.23.

  * There are below restrictions in the MySQL 8.0 package.

    * [:doc:`/tutorial/wrapper`] Wrapper mode is not supported yet.
    * [:doc:`/tutorial/storage`] JSON data type is not supported yet.

Fixes
^^^^^

* [:doc:`/reference/optimizations`] Fixed a bug that the fast order limit optimization doen't work when "WHERE COLUMN IN (SUBQUERY)" exists.

* Fixed a bug that we can't use in place ``ALTER TABLE`` when we modify the data type of a column that has any indexes.

  * This bug occur when we execute ``ALTER TABLE`` in ``ALGORITHM=INPLACE``.
    Therefore, this bug doesn't occur when we use ``ALTER TABLE`` in ``ALGORITHM=COPY``

.. _release-10-10:

Release 10.10 - 2020-12-29
--------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/tutorial/storage`] Added support for customize table flags. [Gitter][Reported by Shinichi Takayanagi]

  * Until now, we can't customize table flags in storage mode.
  * We can customize table flags as below since this release.

    .. code-block::

      CREATE TABLE terms (
        term VARCHAR(64) NOT NULL PRIMARY KEY
      ) COMMENT='flags "TABLE_HASH_KEY|KEY_LARGE"'
        DEFAULT CHARSET=utf8mb4;

  * Please refer to the following URL about customizable items.

    * https://groonga.org/ja/docs/reference/commands/table_create.html#flags

* [:doc:`/install/ubuntu`] Added support for Ubuntu 20.10 (Groovy Gorilla).

* [:doc:`/install/centos`] Added support for Percona Server 8.0.22.

Thanks
^^^^^^

* Shinichi Takayanagi

* pinpikokun [Provided the patch at GitHub#373]

.. _release-10-09:

Release 10.09 - 2020-12-04
--------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/install/centos`] Added support for MariaDB 10.2.36, 10.3.27, 10.4.17, and 10.5.8

* [:doc:`/install/centos`] Added support for Percona Server 5.6.50, 5.7.32, and 8.0.21.

* [:doc:`/install/centos`] Added support for MySQL 5.6.50, 5.7.32, and 8.0.22.

  * There are below restrictions in the MySQL 8.0 package.

    * [:doc:`/tutorial/wrapper`] Wrapper mode is not supported yet.
    * [:doc:`/tutorial/storage`] JSON data type is not supported yet.

* Dropped support for MariaDB 10.1 except Ubuntu 18.04 LTS.

  * Because MariaDB 10.1 is EOL.
  * However, we have only supported MariaDB 10.1 for Ubintu 18.04 LTS

    * Because MariaDB 10.1 has supported yet on it.

* Dropped support for CentOS 6.

  * Because CentOS 6 is EOL.

* [:doc:`/reference/udf/mroonga_snippet_html`] Added support for customizing normalizer.

  * We can use custom normalizer instead of the default normalizer(NromalizerAuto) by using ``table_name`` and ``index_name``.

    * We specify target table name to ``table_name`` as below.
    * We specify index name that is specified on target table  to ``index_name`` as below.

    .. code-block::

      SET NAMES utf8mb4;

      CREATE TABLE memos (
      content text,
      fulltext index content_index (content)
      COMMENT 'normalizer "NormalizerNFKC121(\'unify_kana\', true)"'
      ) DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

      SELECT mroonga_snippet_html('This is Mroonga（ムルンガ）.',
      'memos' as table_name,
      'content_index' as index_name,
      'むるんが') as snippet;

      snippet
      <div class="snippet">This is Mroonga（<span class="keyword">ムルンガ</span>）.</div>

    * We can also use mixing search by query and search by keywords by this modification.
    * For example as below, we can highlight keywords that we both specify by ``AS query`` and ``AS snippet``.

      .. code-block::

	 SET NAMES utf8mb4;
	 SELECT mroonga_snippet_html('Mroonga has two running modes.

         One is “storage mode”, that is the default mode, and we use Groonga for both storing data and searching. With this mode, you can have full benefits of Groonga described above, like fast data update, lock-free full text search and geolocation search. But it does not support transactions.
         
         Another one is “wrapper mode”, that adds full text search function on other storage engines like MyISAM or InnoDB. With this mode, you can use Groonga’s fast full text search with having the benefits of the storage engine, ex. transaction in InnoDB. But you cannot have benefits from Groonga’s read-lock free characteristic. And you might have the performance bottle neck in the storage engine in updating data.',
         'lock storage' AS query,
         'update' AS snippet;

	 snippet
	 <div class="snippet"><span class="keyword">storage</span> mode”, that is the default mode, and we use Groonga for both storing data and searching. With this mode, you can have full benefits of Groonga described above, like fast data <span class="keyword">update</span>, <span class="keyword">lock</span>-fr</div><div class="snippet">text search function on other <span class="keyword">storage</span> engines like MyISAM or InnoDB. With this mode, you can use Groonga’s fast full text search with having the benefits of the <span class="keyword">storage</span> engine, ex. transaction in In</div><div class="snippet">noDB. But you cannot have benefits from Groonga’s read-<span class="keyword">lock</span> free characteristic. And you might have the performance bottle neck in the <span class="keyword">storage</span> engine in updating data.</div>

.. _release-10-07:

Release 10.07 - 2020-10-02
--------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/install/centos`] Added support for MariaDB 10.5.5

* Added new tests that use > 256 byte strings in the column compression tests. [GitHub#350][Patched by KartikSoneji]

Thanks
^^^^^^

* KartikSoneji

.. _release-10-06:

Release 10.06 - 2020-09-02
--------------------------

Improvements
^^^^^^^^^^^^

.. note::

   We removed the news that "Added support for MariaDB 10.5.5".

   At first, we announced that "Added support for MariaDB 10.5.5".
   However, it had been wrong.
   Mroonga 10.06 can build on MariaDB 10.5. However, there are points that are not working normally yet.

* [:doc:`/install/ubuntu`] Dropped Ubuntu 19.10 (Eoan Ermine) support.

* [:doc:`/install/centos`] Added support for Percona Server 8.0.20.

* [:doc:`/install/centos`] Added support for MariaDB 10.1.46, 10.2.33, 10.3.24, and 10.4.14.

* Modify how to install into Debian GNU/Linux.

  * We modify to use ``groonga-apt-source`` instead of ``groonga-archive-keyring``.
  * Because the ``lintian`` command recommends using apt-source if a package that it puts files under the ``/etc/apt/sources.lists.d/``.

    * The ``lintian`` command is the command which checks for many common packaging errors.
    * Please also refer to the following for the details about installation procedures.

      * [:doc:`/install/debian`]

* [:doc:`/install/centos`] Added support for Percona Server 5.7.31.

.. _release-10-05:

Release 10.05 - 2020-07-29
--------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/install/centos`] Added support for MySQL 5.6.49, 5.7.31, and 8.0.21.

  * There are below restrictions in the MySQL 8.0 package.

    * [:doc:`/tutorial/wrapper`] Wrapper mode is not supported yet.
    * [:doc:`/tutorial/storage`] JSON data type is not supported yet.

* [:doc:`/install/centos`] Added support for Percona Server 5.6.49.

.. _release-10-03:

Release 10.03 - 2020-05-29
--------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/install/ubuntu`] Added support for Ubuntu 20.04 (Focal Fossa) support.

* [:doc:`/install/ubuntu`] Dropped Ubuntu 19.04 (Disco Dingo) support.

* [:doc:`/install/centos`] Added support for MariaDB 10.1.45, 10.2.32, 10.3.23, and 10.4.13.

* [:doc:`/install/centos`] Added support for Percona Server 5.6.48 and 5.7.30.

* Dropped support for MariaDB 10.1 in Windows.

  * Because MariaDB 10.1 will be EOL soon.

Fixes
^^^^^

* [:doc:`/install/ubuntu`] Fixed a bug that Mroonga for Ubuntu 19.10 (Eoan Ermine) was not provided.

.. _release-10-02:

Release 10.02 - 2020-04-29
--------------------------

Improvements
^^^^^^^^^^^^

* Added support for ``WITH_WEIGHT`` column flag.

  * This flag require Groonga 10.0.2 or later.
  * We can insert vector with weight as below by this flag.

    .. code-block::

       CREATE TABLE tags (
         name VARCHAR(64) PRIMARY KEY
       ) DEFAULT CHARSET=utf8mb4;

       CREATE TABLE bugs (
         tags TEXT COMMENT 'flags "COLUMN_VECTOR|WITH_WEIGHT", type "tags"',
         FULLTEXT INDEX bugs_tags (tags) COMMENT 'table "tags", flags "WITH_WEIGHT"'
       ) DEFAULT CHARSET=utf8mb4;

       INSERT INTO bugs VALUES ('{"package": 100, "priority": 5}');


* Dropped support for MariaDB 5.5 in CentOS7.

* [:doc:`/install/centos`] Added support for MySQL 5.6.48, 5.7.30, 8.0.20

.. _release-10-01:

Release 10.01 - 2020-03-30
--------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/install/centos`] Added support for Percona Server 5.7.29.

Fixes
^^^^^

* Fixed a bug that Mroonga crashed when we sent to query as below conditions. [GitHub#303][Reported by pinpikokun]

  * When Mroonga's optimize of ``ORDER_BY_LIMIT`` is valid.

    * See https://mroonga.org/docs/reference/optimizations.html#order-by-limit about conditions of this optimize.

  * When sub query by using Mroonga does not use ``ORDER BY LIMIT``.

Thanks
^^^^^^

* pinpikokun

.. _release-9-12:

Release 9.12 - 2020-01-29
-------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/install/centos`] Added support for MySQL 5.6.47, 5.7.29, 8.0.19

* [:doc:`/install/centos`] Added support for MariaDB 10.1.44, 10.2.31, 10.3.22, 10.4.12.

* Fixed a build error when we built by using source of MySQL 8.0.19.

* [:doc:`/install/centos`] Added support for Percona Server 5.6.47.

.. _release-9-11:

Release 9.11 - 2020-01-08
-------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/install/ubuntu`] Dropped Ubuntu 14.04 LTS (Trusty Tahr) support.

* [:doc:`/install/ubuntu`] Dropped Ubuntu 18.10 (Cosmic Cuttlefish) support.

* [:doc:`/install/centos`] Added how to install for CentOS 8.

* [:doc:`/install/centos`] Added support for MariaDB 10.2.30, 10.3.21, 10.4.11.

* Applied a patch for visualizing the shutdown process of InnoDB into MariaDB 10.3 and 10.4.

  * This is a temporary change. We will remove this modify in a future version.
  * This modify Increasing outputted logs when InnoDB shutdown.

Fixes
^^^^^

* [:doc:`/install/ubuntu`] Added support for Ubuntu 16.04 (Xenial Xerus) that was missing.

.. _release-9-10:

Release 9.10 - 2019-11-29
-------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/install/centos`] Added support for MariaDB 10.1.43, 10.2.29, 10.3.20, 10.4.10

* [:doc:`/install/centos`] Added support for MariaDB 10.3.20, 10.4.10, and MySQL 8.0.18 on CentOS 8.

* [:doc:`/install/ubuntu`] Added support for Ubuntu 19.10 (Eoan Ermine)

Fixes
^^^^^

* Fixed a bug that ``dpkg --configure`` will fail when we install the package on Ubuntu 18.04 on WSL. [GitHub#282][Patched by ochaochaocha3]

Thanks
^^^^^^

* ochaochaocha3

.. _release-9-09:

Release 9.09 - 2019-10-30
-------------------------

.. note::

    Maybe performance decreases from this version.
    Therefore, If performance decreases than before, please report us with reproducible steps.

Improvements
^^^^^^^^^^^^

* [:doc:`/install/centos`] Added support for MySQL 5.6.46, 5.7.28, and 8.0.18(with restrictions).

Fixes
^^^^^

* Fixed a build error a package for MySQL 8.0.18.

.. _release-9-08:

Release 9.08 - 2019-09-27
-------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/install/centos`] Added support for MariaDB 5.5.64, 10.2.27, 10.3.18, and 10.4.8

* [:doc:`/install/centos`] Dropped 32-bit package support on CentOS 6.

Fixes
^^^^^

* [:doc:`/install/debian`] Fixed that can't install mariadb-server-10.3-mroonga in Debian 10(buster).

Thanks
^^^^^^

* kajiys

* bizlevel

.. _release-9-07:

Release 9.07 - 2019-08-29
-------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/install/debian`] Added a install document for Debian 10(buster).

* [:doc:`/install/centos`] Added support for MariaDB 10.1.41, 10.2.26, 10.3.17, and 10.4.7.

* [:doc:`/reference/server_variables`] Added a document for ``mroonga_query_log_file``.

* [:doc:`/install/others`] Added a document about how to uninstall Mroonga. [GitHub#135][Patched by ryfjwr]

* [:doc:`/tutorial/storage`] Added a document about how to use regular expression search.

* Dropped support for MariaDB 10.0

* [:doc:`/install/centos`] Added support for Percona Server 5.6.45 and
  5.7.27

* [:doc:`/install/centos`] Dropped support for MariaDB 10.x on CentOS 6

Thanks
^^^^^^

* ryfjwr

.. _release-9-05:

Release 9.05 - 2019-07-30
-------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/install/centos`] Added a description about current limitation with MySQL 8.0.

* [:doc:`/install/centos`] Added support for MySQL 5.6.45, 5.7.27 and 8.0.17.

Fixes
^^^^^

* [:doc:`/install/centos`] Fixed a wrong ``mysql80-comunity-release`` package name.
  [groonga-dev,04759][Reported by Kagami Hiroshi]

* [:doc:`/tutorial/storage`] Fixed an unique index update bug. This bug causes duplicated key error when the following conditions are met.

  * An unique index is created against multiple column index
  * partial unique key column is updated

  Note that if you already created an unique index, you must recreate target tables because unique index may have garbage entries. We recommend to recreate an target table with dump and restore, or execute ``ALTER TABLE (TABLE_NAME) FORCE``.

* [mysql8.0] Added a support for ``TIMESTAMP``
  [groonga-dev,04763][Reported by Kagami Hiroshi]

Thanks
^^^^^^

* Kagami Hiroshi

.. _release-9-04:

Release 9.04 - 2019-06-29
-------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/install/centos`] Added support for Percona Server 5.7.26.

* [:doc:`/install/centos`] Added support for MariaDB 10.2.25/10.3.16.

* [:doc:`/install/centos`][experimental] Added support for MySQL 8.0.16.

Fixes
^^^^^

* Fixed a infinite loop bug.
  This bug is occurred when invalid flag is specified such as ``FULLTEXT INDEX (...) COMMENT 'index_flags "INVALID|WITH_SECTION"'``.

* [windows] Fixed a inappropriate pdb path with MariaDB 10.2/10.3.

* [:doc:`/install/others`] Fixed a typo about missing appropriate account name in plugin install instruction.

* Fixed a crash bug with ``((MATCH OR MATCH) AND (MATCH))`` query.

.. _release-9-03:

Release 9.03 - 2019-05-29
-------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/tutorial/storage`] Updated example to use ``tokenizer "XXX"` in ``COMMENT`` because ``default_tokenizer "XXX"`` is deprecated since 9.01.

* [:doc:`/install/windows`] Added support to provide MariaDB 10.1, 10.2 zip package again.

* [:doc:`/install/centos`] Added support for MariaDB 10.3.14 and 10.3.15.

* [:doc:`/install/debian`] Updated install instruction for copy and paste friendly.

* Added support for ``INDEX_LARGE`` flag such as ``COMMENT 'flags "INDEX_LARGE"'`` syntax.

* [:doc:`/install/centos`] Added support for MariaDB 10.2.24.

* [:doc:`/install/centos`] Added support for MariaDB 10.1.40.

* [:doc:`/install/ubuntu`] Added support for Ubuntu 19.04 (Disco Dingo)

* [:doc:`/install/centos`] Added support for Percona Server 5.6.44.

* [:doc:`/install/centos`] Added support for MySQL 5.6.44 and 5.7.26.

.. _release-9-01:

Release 9.01 - 2019-03-29
-------------------------

Improvements
^^^^^^^^^^^^

* [storage] Added support for tokenizer options.

  * For example, you can specify tokenizer options in COMMENT section such as ``CREATE TABLE foo (...) COMMENT='tokenizer "TokenNgram(''n'', 4)"'``.

* [mariadb] Added support for "tokenizer" table parameter.

  * For example, you can specify "tokenizer" such as ``CREATE TABLE foo (...) TOKENIZER='TokenNgram("n", 4)'``.

* [storage] Added support for tokenizer parameter in comment.

  * Note that ``default_tokenizer`` is deprecated.

* [mariadb] Added support for "normalizer" table parameter.

  * For example, you can specify "normalizer" such as ``CREATE TABLE foo (...) NORMALIZER='NormalizerNFKC100("unify_kana", true)'``.

* [mariadb] Added support for "token_filters" table parameter.

  * For example, you can specify "token_filters" such as ``CREATE TABLE foo (...)  TOKEN_FILTERS='TokenFilterNFKC100("unify_katakana_v_sounds", true)'``.

* Added support for "LEXICON" index parameter.

  * For example, you can specify ``FULLTEXT INDEX foo (bar) LEXICON='terms'`` or ``FULLTEXT INDEX foo (bar) COMMENT 'lexicon "terms"'``.

* [appveyor] Improved support to building Mroonga enabled package [GitHub#230]

* [:doc:`/install/centos`] Added support for Percona Server 5.7.25-28.

* [:doc:`/install/centos`] Added support for MariaDB 10.3.13.

* [:doc:`/install/centos`] Added support for MariaDB 10.2.23.

.. _release-9-00:

Release 9.00 - 2019-02-09
-------------------------

This is a major version up! But It keeps backward compatibility.
You can upgrade to 9.00 without rebuilding database.

In Groonga 9.0.0, ``TokenPattern``, ``TokenTable`` tokenizer and
``remove_blank`` for ``NormalizerNFKC100`` is supported.
If you upgrade to Groonga 9.0.0, you can use them from Mroonga 9.00!

* ref: http://groonga.org/docs/news.html#release-9-0-0-2019-02-09

Improvements
^^^^^^^^^^^^

* [:doc:`/install/centos`] Supported Percona Server 5.7.24-27.

* [:doc:`/install/centos`] Supported Percona Server 5.6.43 rel84.3.

* [rpm][centos] Supported MariaDB 10.3.12.

* [rpm][centos] Supported MariaDB 10.2.21.

* [rpm][centos] Supported Percona Server 5.7.24-27.

* [rpm][centos] Supported Percona Server 5.6.43 rel84.3.

* [rpm][centos] Supported MySQL 5.7.25.

* [rpm][centos] Supported MySQL 5.6.43.

.. _release-8-09:

Release 8.09 - 2018-11-29
-------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/install/ubuntu`] Supported Ubuntu 18.10 (Cosmic Cuttlefish).

* [:doc:`/install/windows`] Supported MariaDB 10.3.10.

* [:doc:`/install/centos`] Supported MariaDB 10.2.19

* [:doc:`/install/centos`] Supported MariaDB 10.1.37

* [:doc:`/install/centos`] Supported Percona Server 5.7.23-25.

* [rpm][centos] Supported MariaDB 10.3.11.

* [rpm][centos] Supported MySQL 5.6.42.

* [rpm][centos] Supported MySQL 5.7.24.

Revision:

We deleted information as below.

"Supported MySQL 8"

Sorry, There was wrong release information in Mroonga 8.09.
The MySQL 8 is not supported. That is still being handled.

.. _release-8-07:

Release 8.07 - 2018-09-29
-------------------------

Improvements
^^^^^^^^^^^^

* Deprecated tokenizer ``off`` option. Use tokenizer ``none`` option instead.
* Dropped support for MariaDB 10.2.2 which is shipped at Sep 27, 2016 and older series.

Fixes
^^^^^

* [:doc:`/install/centos`] Supported MariaDB 10.1.36.

.. _release-8-06:

Release 8.06 - 2018-08-29
-------------------------

In this version, MySQL will be automatically restarted if you had
already installed Mroonga and not installed Groonga 8.0.4 or later.
Because Mroonga 8.06 requires Groonga 8.0.4 or later but it will not
reloaded until MySQL is restarted.

Improvements
^^^^^^^^^^^^

* Updated required Groonga version to 8.0.4 or later.

* Updated required groonga-normalizer-mysql version to 1.1.3 or later.

* Supported utf8mb4_0900 family collation.

  * ref: https://github.com/groonga/groonga-normalizer-mysql#description

* Supported tokenizer options.

  * e.g.: ``tokenizer "TokenNgram(\'loose_symbol\', true)"``

  * ref: http://groonga.org/docs/news.html#release-8-0-2-2018-04-29

* Use the Groonga's default logger.

* [:doc:`/install/windows`] Updated bundled MariaDB to 10.3.9 from 10.1.33.

  * NOTICE: Before upgrading to MariaDB 10.3, you need to dump existing MariaDB 10.1 databases. Then restore it after upgrading.

* [:doc:`/install/debian`] Dropped Debian 8 (jessie) support.

* [:doc:`/install/ubuntu`] Dropped Ubuntu 17.10 (Artful Aardvark) support.

* [WIP] Working on supporting MySQL 8.

  * The storage mode is almost done (JSON type doesn't work yet).

  * The wrapper mode is in progress.

Fixes
^^^^^

* [storage] Fixed a bug that wrong result may be returned on multi range read.
  [GitHub#211][Reported by colt27]

Thanks
^^^^^^

* colt27

.. _release-8-03:

Release 8.03 - 2018-05-29
-------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/install/docker`] Added quick start guide link to Docker Hub.

* [:doc:`/install/centos`] Supported MariaDB 10.3.7.

* [:doc:`/install/centos`] Supported MariaDB 10.2.15 (backported to 8.02).

* [:doc:`/install/centos`] Supported MariaDB 10.1.33 (backported to 8.02).

Fixes
^^^^^

* [:doc:`/install/ubuntu`] Fixed install failure on Ubntu 14.04 LTS (Trusty)
  (backported to 8.02). [GitHub#202,#205][Reported by Masato Hirai]

Thanks
^^^^^^

* Masato Hirai

.. _release-8-02:

Release 8.02 - 2018-04-29
-------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/install/ubuntu`] Supported Ubuntu 18.04 LTS (Bionic Beaver).

* [:doc:`/install/debian`] Supported i386 for Jessie.

* Suppress meaningless "duplicated unique id" error log. [GitHub#197]

* [developer][test] Supported `--record` option.

* [:doc:`/install/centos`] Use `groonga-release-latest` instead of `groonga-release-X.X.X`.

* [:doc:`/tutorial/installation_check`] Added version check howto.

* [:doc:`/install/centos`][percona] Supported upgrading from "< 5.6.34" and "< 5.7.21".
  [groonga-dev,04599][Reported by Takashi Kinoshita][Looked into by Satoshi Mitani]

* [:doc:`/install/centos`] Supported MySQL 5.6.40 and 5.7.22.

* [:doc:`/install/centos`] Supported Percona Server 5.7.21-21.

Fixes
^^^^^

* Fixed a crash bug when some complex condition in `ORDER BY` such as `ORDER BY 1 + 1, id, content`.

* Fixed a bug that `MATCH AGAINST` condition is ignored if SQL containing such as
  `AND (x = 1 OR x = 2)` when condition push down is enabled.
  [Gitter/ja:5ae014842b9dfdbc3ac7ce1f][Reported by colt27]

* Fixed a memory leak for column caches.

Thanks
^^^^^^

* Takashi Kinoshita

* Satoshi Mitani

* colt27

.. _release-8-01:

Release 8.01 - 2018-03-29
-------------------------

In this version, MySQL will be automatically restarted if you had
already installed Mroonga. This is because Mroonga requires newer
version of Groonga (8.0.1) to fix bugs, but it will not reloaded until
MySQL is restarted.

Improvements
^^^^^^^^^^^^

* [rpm][centos] Supported Percona Server 5.6.39. [Reported by @iiiiyyyy]

* [rpm][centos] Supported Percona Server 5.7.21.

* [rpm][centos] Supported MariaDB 10.2.13. [GitHub#198] [Reported by
  shota suzuki]

* [rpm][centos] Supported MariaDB 10.2.14.

Fixes
^^^^^

* Fixed a bug that wrong cache for other database is used. If you
  create multiple database and use `mroonga_command()` against one of
  them, wrong cache is returned unexpectedly. To fix this issue,
  Groonga 8.0.1 or later is required.

* Fixed a bug that "NOT IN" query returns empty result. This bug
  occurs when "NOT IN" is used with multiple arguments such as "NOT IN
  (xxx, xxx)"

* Fixed a bug that specified "NOT IN" can't exclude correctly when
  condition push down is enabled.

* Fixed a bug that "ORDER BY RAND()" query returns wrong result. This
  bug occurs when "ORDER BY RAND()" and "LIMIT" is specified at the
  same time.

* Fixed a bug that "fast order limit" optimization is applied
  unexpectedly to "ORDER BY function()".

Thanks
^^^^^^

* @iiiiyyyy

* shota suzuki

.. _release-8-00:

Release 8.00 - 2018-02-09
-------------------------

This is a major version up! But It keeps backward compatibility.
You can upgrade to 8.0.0 without rebuilding database.

Improvements
^^^^^^^^^^^^

* When create hash index, used tiny hash for reducing used resource.

* [percona57] Added gap lock detection support.
  [GitHub#188][Reported by Iwo]

Thanks
^^^^^^

* Iwo

.. _release-7-11:

Release 7.11 - 2018-01-29
-------------------------

Improvements
^^^^^^^^^^^^

* [test] Added a test case for sub query and order limit optimization.
  [GitHub#184] [Reported by Kazuki Hamasaki]

* [rpm][centos] Supported  10.3.

* [deb][ubuntu] Supported MariaDB 10.1 for Ubuntu 17.10 (Artful Aadvark)

Fixes
^^^^^

* [:ref:`status-variable-mroonga-n-pooling-contexts`] Fixed a bug that
  value is reset unexpectedly by ``FLUSH STATUS``.

* [maradb10.3] Fixed a build error which is caused by renamed constant
  variable (``HA_MUST_USE_TABLE_CONDITION_PUSHDOWN`` is renamed to
  ``HA_CAN_TABLE_CONDITION_PUSHDOWN``).

* [:doc:`/install/macos`] Removed obsolete install guide and updated
  link to latest documentation [Reported by Ryuji AMANO]

* [rpm][centos] Supported MariaDB 10.2.12. [GitHub#186] [Reported by
  Shota Suzuki]

* [rpm][centos] Supported Percona Server 5.7.20-19.

* [:doc:`/install/ubuntu`] Dropped Ubuntu 17.04 (Zesty Zapus) support.
  It has reached EOL at Jan 13, 2018.

Thanks
^^^^^^

* Kazuki Hamasaki
* Ryuji AMANO
* Shota Suzuki

.. _release-7-10:

Release 7.10 - 2017-12-29
-------------------------

Improvements
^^^^^^^^^^^^

* Updated required Groonga version to 7.1.0. You need to restart MySQL
  after you upgrade to Mroonga 7.10.

* [mariadb10.3] Supported MariaDB 10.3.

* [rpm][centos] Supported MariaDB 10.2.11.

* [rpm][centos] Supported MariaDB 10.1.30.

* [rpm][centos] Supported Percona Server 5.7.20.

* [rpm][centos] Supported Percona Server 5.6.38.

* [:doc:`/reference/optimizations`] Enable count_skip optimization for multi-column index.

* Supported condition push down and added related new variables.

  * [:ref:`status-variable-mroonga-condition-push-down`] Added ``Mroonga_condition_push_down``.

  * [:ref:`server-variable-mroonga-condition-push-down-type`] Added
    ``mroonga_condition_push_down_type``. The default value is
    ``ONE_FULL_TEXT_SEARCH``. It means that condition push down is
    enabled only when ``WHERE`` clause has one ``MATCH AGAINST``
    condition.  If the value ``ALL`` is set, condition push down is
    always used (``ALL`` is experimental for now. We are glad if you
    use it and tell us if it got faster or not).

* Supported column cache when to get fixed size column value to improve performance.

* Renamed a function ``last_insert_grn_id`` to ``mroonga_last_insert_grn_id`` to add missing ``mroonga_`` prefix.
  ``last_insert_grn_id`` is deprecated.
  [GitHub#177] [Reported by Ian Gilfillan]

* [:ref:`status-variable-mroonga-n-pooling-contexts`] Added
  a new status variable to show the number of pooling contexts for
  :doc:`/reference/udf/mroonga_command`.

* [ ``FLUSH TABLES`` ] Added clearing pooling contexts for
  :doc:`/reference/udf/mroonga_command` support.

Thanks
^^^^^^

* Ian Gilfillan

.. _release-7-09:

Release 7.09 - 2017-11-29
-------------------------

Improvements
^^^^^^^^^^^^

* [rpm][centos] Supported MariaDB 10.2.10.

* [rpm][centos] Supported MariaDB 10.1.29.

* Fixed not to require sed to run tests. [Patch by Sergei Golubchik]

* [cmake] Changed to skip Mroonga related configurations on without
  Mroonga build. [Patch by Vladislav Vaintroub]

Thanks
^^^^^^

* Sergei Golubchik

* Vladislav Vaintroub

.. _release-7-08:

Release 7.08 - 2017-10-29
-------------------------

Improvements
^^^^^^^^^^^^

* Supported table level ``flags`` option.
  You can specify ``TABLE_HASH_KEY``, ``TABLE_PAT_KEY``, ``TABLE_DAT_KEY``, and ``KEY_LARGE``
  table options. [groonga-dev, 04494] [Reported by Masanori Miyashita]

* [rpm][centos] Supported MySQL 5.6.38-2 and 5.7.20-1.

* [:doc:`/install/ubuntu`] Supported Ubuntu 17.10 (Artful Aardvark).

Thanks
^^^^^^

* Masanori Miyashita

.. _release-7-07:

Release 7.07 - 2017-10-12
-------------------------

Improvements
^^^^^^^^^^^^

* [mroonga_query_expand] Added ``mroonga_query_expand`` UDF. If you
  prepare synonyms table in advance, you can get expanded synonym in
  your query by ``mroonga_query_expanded``. Note that Groonga 7.0.6 or
  later version is required to use this function.

* [rpm][centos] Supported Percona Server 5.7.19-17.1. [Reported by
  tigersun2000]

* [rpm][centos] Supported MariaDB 5.5.56-2. [Reported by akiko_pusu]

* [rpm][centos] Supported MariaDB 10.1/10.2 provided by MariaDB.

Fixes
^^^^^

* Fixed a bug that wrong database may be used on "DROP DATABASE".
  This bug may cause a crash because internal "mroonga_operations"
  table is removed unexpectedly. It may happen when the following two conditions are true:

  1. There are multiple databases that uses Mroonga.
  2. "DROP DATABASE" against no longer Mroonga tables exist.

  As unexpected result, "DROP DATABASE x" may remove
  "mroonga_operations" table on existing "y" database.

* Fix a crash bug after CHECK TABLE is used. [GitHub#167] [Reported by
  GMO Media, Inc.]

* [deb][mariadb10] Added missing dependency to lsb-release package for
  preinst and postrm maintainer script. [GitHub#169] [Patch by Tatsuki
  Sugiura]

Thanks
^^^^^^

* @tigersun2000
* @akiko_pusu
* GMO Media, Inc.
* Tatsuki Sugiura

.. _release-7-06:

Release 7.06 - 2017-08-29
-------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/reference/udf/mroonga_highlight_html`] Added usage about
  ``mroonga_highlight_html``.

* Supported generated column. It is useful to full-text search for
  partially extracted content from JSON column. [GitHub#159,#160,#163]
  [Patch by Naoya Murakami]

* Added :ref:`server-variable-mroonga-enable-operations-recording`. variable.
  [GitHub#158] [Patch by Naoya Murakami]

* Supported virtual column for MariaDB 10.2 and MySQL 5.7. It supports
  ``VIRTUAL`` type.  [GitHub#161,#162] [Patch by Naoya Murakami]

* Supported MariaDB 10.1.26.

* [rpm][centos] Supported Percona Server 5.6.36 rel82.1 and 5.7.18-16.
  [Gitter/ja:59894500bc46472974622cbd] [Reported by
  @tigersun2000_twitter]

* [rpm][centos] Supported MySQL 5.6.37 and 5.7.19 on CentOS 7.
  [groonga-dev,04441] [Reported by Kagami Hiroshi]

Thanks
^^^^^^

* Naoya Murakami
* @tigersun2000_twitter
* Kagami Hiroshi

.. _release-7-05:

Release 7.05 - 2017-07-29
-------------------------

Improvements
^^^^^^^^^^^^

* Supported Groonga query log. Use ``mroonga_query_log_file`` variable
  to use this feature. [GitHub#148]

* Supported MariaDB 10.2.7. [groonga-dev,04397] [Reported by Tomohiro
  'Tomo-p' KATO]

* [:doc:`/reference/udf/mroonga_command`] Supported database name that
  has special name such as ``db-1`` for example. It contains special
  character ``-``.

* [:doc:`/reference/udf/mroonga_command`] Supported auto command
  syntax escape feature. It makes easy to use Groonga functionality
  from Mroonga.

* Supported MariaDB 5.5.57.

* [rpm][centos] Supported MySQL 5.6.37-2 and MySQL 5.7.19-1 on
  CentOS 6. [groonga-dev,04403] [Reported by Kagami Hiroshi]

* [:doc:`/install/ubuntu`] Dropped Ubuntu 16.10 (Yekkety Yak) support.
  It has reached EOL at July 20, 2017.

* [:doc:`/reference/udf/mroonga_highlight_html`] Supported a function
  to highlight target column or text.

Fixes
^^^^^

* Fixed a crash bug when there is no active index internally.
  [Gitter:groonga/ja:596714a5c101bc4e3a7db4e5] [Reported by K
  Torimoto]

Thanks
^^^^^^

* K Torimoto
* Tomohiro 'Tomo-p' KATO
* Kagami Hiroshi

.. _release-7-04:

Release 7.04 - 2017-06-29
-------------------------

Improvements
^^^^^^^^^^^^

* Supported to show error message when failed to create a table for
  matched records. This kind of error occurs when indexes are
  broken. This error message helps to identify problem.

* [:doc:`/install/debian`] Supported Debian 9 (stretch).

Fixes
^^^^^

* Fixed a crash bug that missing ``NULL`` check before calling
  ``grn_table_setoperation`` causes. Such a crash bug occurs when
  indexes are broken.

.. _release-7-03:

Release 7.03 - 2017-05-29
-------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/reference/limitations`] Updated limitations about NULL in
  column. [Gitter/ja] [Reported by @bignum_twitter]

* Supported ``INDEX_MEDIUM`` and ``INDEX_SMALL`` flags. [GitHub#141]
  [Patch by Naoya Murakami]

* [:doc:`/install/centos`] Supported recent Percona Server 5.6.36 and
  5.7.18. [Reported by @pinpikokun]

Thanks
^^^^^^

* @bignum_twitter
* @pinpikokun
* Naoya Murakami

.. _release-7-02:

Release 7.02 - 2017-04-29
-------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/install/centos`] Dropped Ubuntu 12.04 (Precise Pangolin)
  support because of EOL.

* [:doc:`/install/ubuntu`] Added Zesty Zapus (Ubuntu 17.04) support.

Fixes
^^^^^

* [:doc:`/install/centos`] Fixed build error with MySQL 5.6.36 and
  MySQL 5.7.18.

* [cmake] Fixed missing link to ``libgroonga`` when mroonga is bundled
  and ``libgroonga`` isn't bundled. [GitHub#137] [Patch by Naoya
  Murakami]

Thanks
^^^^^^

* Naoya Murakami

.. _release-7-01:

Release 7.01 - 2017-03-29
-------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/install/centos`] Dropped CentOS 5 support because of EOL.

* [:doc:`/tutorial/storage`] Supported ``ALTER TABLE ADD/DROP FOREIGN
  KEY``.

* [:doc:`/tutorial/storage`] Supported fast ``ORDER LIMIT`` with
  ``ENUM``.  [groonga-dev,04277] [Reported by murata satoshi]

* Supported ``COMPRESS_ZSTD`` column compression flag. [GitHub#133]
  [Patch by Naoya Murakami]

* [:doc:`/reference/server_variables`] Added documentation about
  :ref:`server-variable-mroonga-libgroonga-support-zstd`
  variable. [GitHub#133] [Patch by Naoya Murakaimi]

* [:doc:`/install`] Changed to recommend ``https://packages.groonga.org``
  for downloading resources.

Fixes
^^^^^

* [:doc:`/tutorial/storage`] Fixed update error for log-bin and
  ``UPDATE`` with collated ``PRIMARY KEY``. [GitHub#132] [Reported by
  kitora]

* Fixed a bug that ``FOREIGN KEY`` is dumped unexpectedly even though
  you didn't set it. [groonga-dev,04276] [Reported by murata satoshi]

Thanks
^^^^^^

* kitora
* murata satoshi
* Naoya Murakami

.. _release-7-00:

Release 7.00 - 2017-02-09
-------------------------

Improvements
^^^^^^^^^^^^

* [:doc:`/install/centos`] Changed to ensure enabling EPEL to install
  package.

* Supported ``FOREIGN KEY`` constrain on ``UPDATE`` and ``DELETE``
  parent row. In the previous versions, only ``FOREIGN KEY`` constrain
  on ``INSERT`` is supported.

* [:doc:`/tutorial/storage`] Supported updating row even though its
  table has primary key with ROW binlog format. In the previous
  version, it causes update statement error. [GitHub#130] [Reported by
  kitora]

Thanks
^^^^^^

* kitora

Past releases
-------------

.. toctree::
   :maxdepth: 2

   news/6.x
   news/5.x
   news/4.x
   news/3.x
   news/2.x
   news/1.x
   news/0.x
