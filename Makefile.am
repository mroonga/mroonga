AUTOMAKE_OPTIONS = 1.9.7

LOCALES = ja

AM_CPPFLAGS = $(MYSQL_INC) $(GROONGA_CFLAGS) $(MYSQL_VERSION_CFLAGS)
ACLOCAL_AMFLAGS = $$ACLOCAL_ARGS

noinst_HEADERS =				\
	mrn_sys.h				\
	ha_mroonga.h				\
	mrn_table.h				\
	mrn_err.h				\
	mrn_mysql.h				\
	mrn_mysql_compat.h

plugin_LTLIBRARIES     = ha_groonga.la
ha_groonga_la_LDFLAGS  = -module $(GROONGA_LIBS)
ha_groonga_la_CXXFLAGS = $(AM_CXXFLAGS) $(MYSQL_CFLAGS) -DMYSQL_DYNAMIC_PLUGIN
ha_groonga_la_CFLAGS   = $(AM_CFLAGS) $(MYSQL_CFLAGS) -DMYSQL_DYNAMIC_PLUGIN
ha_groonga_la_SOURCES  = ha_mroonga.cc mrn_sys.c mrn_table.cc

plugin_LIBRARIES          = libha_groonga.a
libha_groonga_a_CXXFLAGS  = $(AM_CXXFLAGS) $(MYSQL_CFLAGS)
libha_groonga_a_CFLAGS    = $(AM_CFLAGS) $(MYSQL_CFLAGS)
libha_groonga_a_SOURCES   = ha_mroonga.cc mrn_sys.c mrn_table.cc

SUBDIRS =					\
	test					\
	doc					\
	tools					\
	packages

EXTRA_DIST = plug.in

installcheck-local: install
	test/run-sql-test.sh

tag:
	cd $(top_srcdir) && \
	  git tag v$(VERSION) -a -m 'mroonga $(VERSION)!!!'

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
	cd $(top_srcdir) &&				\
	  misc/update-latest-release.rb			\
	  $(PACKAGE) $(OLD_RELEASE) $(OLD_RELEASE_DATE)	\
	  $(VERSION) $(NEW_RELEASE_DATE)		\
	  packages/rpm/fedora/mysql-groonga.spec.in	\
	  packages/rpm/centos/mysql-groonga.spec.in	\
	  packages/debian/changelog			\
	  $(MROONGA_GITHUB_COM_PATH)/index.html		\
	  $(MROONGA_GITHUB_COM_PATH)/ja/index.html

update-po:
	@for lang in $(LOCALES); do						\
	  (cd $(top_srcdir)/doc/locale/$$lang/LC_MESSAGES && make update)	\
	done

update-document:
	@if test -z "$(MROONGA_GITHUB_COM_PATH)"; then			\
	  echo "\$$(MROONGA_GITHUB_COM_PATH) is missing";		\
	  echo "add --with-mroonga-github-com-path in configure";	\
	  exit 1;							\
	fi
	rm -rf tmp-doc
	mkdir tmp-doc
	(cd doc && $(MAKE) clean-html)
	(cd doc && $(MAKE) install docdir=$(abs_srcdir)/tmp-doc/install)
	ruby $(srcdir)/tools/prepare-sphinx-html.rb tmp-doc/install tmp-doc/dist
	rm -rf $(MROONGA_GITHUB_COM_PATH)/docs
	mv tmp-doc/dist/en $(MROONGA_GITHUB_COM_PATH)/docs
	for locale in `cd tmp-doc/dist; echo *`; do		\
	  dest_base_dir=$(MROONGA_GITHUB_COM_PATH)/$${locale};	\
	  mkdir -p $${dest_base_dir};				\
	  dest_dir=$${dest_base_dir}/docs;			\
	  rm -rf $${dest_dir};					\
	  mv tmp-doc/dist/$${locale} $${dest_dir};		\
	done

update-files:
	cd $(srcdir)/doc && $(MAKE) update-files
	cd $(srcdir)/test/sql && $(MAKE) update-files

echo-mysql-source:
	echo $(MYSQL_SOURCE)

echo-mysql-build:
	echo $(MYSQL_BUILD)

echo-mysql-version:
	echo $(MYSQL_VERSION)

echo-cutter:
	echo $(CUTTER)
