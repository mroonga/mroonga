AUTOMAKE_OPTIONS = 1.9.7

AM_CPPFLAGS = $(MYSQL_INC) $(GROONGA_CFLAGS) $(MECAB_INCLUDES) $(MYSQL_VERSION_CFLAGS)
ACLOCAL_AMFLAGS = $$ACLOCAL_ARGS

noinst_HEADERS = mrn_sys.h ha_mroonga.h mrn_table.h mrn_err.h mrn_mysql.h

plugin_LTLIBRARIES     = ha_groonga.la
ha_groonga_la_LDFLAGS  = -module $(GROONGA_LIBS) $(MECAB_LIBS)
ha_groonga_la_CXXFLAGS = $(AM_CFLAGS) $(MYSQL_CFLAGS) -DMYSQL_DYNAMIC_PLUGIN
ha_groonga_la_CFLAGS   = $(AM_CFLAGS) $(MYSQL_CFLAGS) -DMYSQL_DYNAMIC_PLUGIN
ha_groonga_la_SOURCES  = ha_mroonga.cc mrn_sys.c mrn_table.cc

plugin_LIBRARIES                      = libgroonga_storage_engine.a
libgroonga_storage_engine_a_LDFLANGS  = $(GROONGA_LIBS) $(MECAB_LIBS)
libgroonga_storage_engine_a_CXXFLAGS  = $(AM_CFLAGS) $(MYSQL_CFLAGS)
libgroonga_storage_engine_a_CFLAGS    = $(AM_CFLAGS) $(MYSQL_CFLAGS)
libgroonga_storage_engine_a_SOURCES   = ha_mroonga.cc mrn_sys.c mrn_table.cc

SUBDIRS = test packages doc

EXTRA_DIST = plug.in

installcheck-local: install
	test/run-sql-test.sh

tag:
	cd $(top_srcdir) && \
	  git tag v$(VERSION) -a -m 'groonga storage engine $(VERSION)!!!'

update-latest-release:
	@if test -z "$(OLD_RELEASE)"; then	\
	  echo "\$$(OLD_RELEASE) is missing";	\
	  exit 1;				\
	fi
	@if test -z "$(OLD_RELEASE_DATE)"; then		\
	  echo "\$$(OLD_RELEASE_DATE) is missing";	\
	  exit 1;					\
	fi
	@if test -z "$(NEW_RELEASE_DATE)"; then		\
	  echo "\$$(NEW_RELEASE_DATE) is missing";	\
	  exit 1;					\
	fi
	cd $(top_srcdir) && test -d "misc" ||					\
	  svn export --force							\
	  https://cutter.svn.sourceforge.net/svnroot/cutter/cutter/trunk/misc
	cd $(top_srcdir) &&					\
	  misc/update-latest-release.rb				\
	  $(PACKAGE) $(OLD_RELEASE) $(OLD_RELEASE_DATE)		\
	  $(VERSION) $(NEW_RELEASE_DATE)			\
	  packages/rpm/fedora/mysql-groonga.spec.in		\
	  packages/rpm/centos/mysql-groonga.spec.in		\
	  packages/debian/changelog

echo-mysql-source:
	echo $(MYSQL_SOURCE)

echo-mysql-build:
	echo $(MYSQL_BUILD)

echo-mysql-version:
	echo $(MYSQL_VERSION)

echo-cutter:
	echo $(CUTTER)
