AM_CPPFLAGS = $(MYSQL_INC) $(GROONGA_INCLUDES) $(MECAB_INCLUDES)
AM_INCLUDES = -l. -l..
ACLOCAL_AMFLAGS = $$ACLOCAL_ARGS

MYSQLDATAdir  = $(localstatedir)
MYSQLSHAREdir = $(pkgdatadir)
MYSQLBASEdir  = $(prefix)
MYSQLLIBdir   = $(pkglibdir)
pkgplugindir  = $(pkglibdir)/plugin

DEFS          = @DEFS@

noinst_HEADERS = mrnsys.h ha_mroonga.h

lib_LTLIBRARIES      = libgroonga_storage_engine.la
#pkgplugin_LTLIBRARIES  = @plugin_mroonga_shared_target@
libgroonga_storage_engine_la_LDFLAGS  = -module -rpath $(pkgplugindir) $(GROONGA_LIBS) $(MECAB_LIBS)
libgroonga_storage_engine_la_CXXFLAGS = $(AM_CFLAGS) -DMYSQL_DYNAMIC_PLUGIN
libgroonga_storage_engine_la_CFLAGS   = $(AM_CFLAGS) -DMYSQL_DYNAMIC_PLUGIN
libgroonga_storage_engine_la_SOURCES  = ha_mroonga.cc ha_mroonga.h mrnsys.c mrnsys.h

lib_LIBRARIES          = libgroonga_storage_engine.a
libgroonga_storage_engine_a_LDFLANGS  = $(GROONGA_LIBS) $(MECAB_LIBS)
libgroonga_storage_engine_a_CXXFLAGS  = $(AM_CFLAGS)
libgroonga_storage_engine_a_CFLAGS    = $(AM_CFLAGS)
libgroonga_storage_engine_a_SOURCES   = ha_mroonga.cc mrnsys.c

SUBDIRS = test

EXTRA_DIST = plug.in

installcheck-local: install
	test/run-sql-test.sh
