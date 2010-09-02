AM_CPPFLAGS = $(MYSQL_INC) $(GROONGA_INCLUDES) $(MECAB_INCLUDES)
AM_INCLUDES = -l. -l..
ACLOCAL_AMFLAGS = $$ACLOCAL_ARGS

noinst_HEADERS = mrnsys.h ha_mroonga.h

pkgplugin_LTLIBRARIES  = ha_groonga.la
ha_groonga_la_LDFLAGS  = -module $(GROONGA_LIBS) $(MECAB_LIBS)
ha_groonga_la_CXXFLAGS = $(AM_CFLAGS) -DMYSQL_DYNAMIC_PLUGIN
ha_groonga_la_CFLAGS   = $(AM_CFLAGS) -DMYSQL_DYNAMIC_PLUGIN
ha_groonga_la_SOURCES  = ha_mroonga.cc ha_mroonga.h mrnsys.c mrnsys.h

pkgplugin_LIBRARIES    = ha_groonga.a
ha_groonga_a_LDFLANGS  = $(GROONGA_LIBS) $(MECAB_LIBS)
ha_groonga_a_CXXFLAGS  = $(AM_CFLAGS)
ha_groonga_a_CFLAGS    = $(AM_CFLAGS)
ha_groonga_a_SOURCES   = ha_mroonga.cc mrnsys.c

SUBDIRS = test apt

EXTRA_DIST = plug.in

installcheck-local: install
	test/run-sql-test.sh
