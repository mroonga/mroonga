AM_CPPFLAGS = $(MYSQL_INC) $(GROONGA_INCLUDES) $(MECAB_INCLUDES) $(CUTTER_CFLAGS)
AM_INCLUDES = -l. -l..
ACLOCAL_AMFLAGS = $$ACLOCAL_ARGS

MYSQLDATAdir  = $(localstatedir)
MYSQLSHAREdir = $(pkgdatadir)
MYSQLBASEdir  = $(prefix)
MYSQLLIBdir   = $(pkglibdir)
pkgplugindir  = $(pkglibdir)/plugin

WRAPLIBS      =

LDADD         =

DEFS          = @DEFS@

noinst_HEADERS = driver.h ha_groonga.h

lib_LTLIBRARIES      = libmroonga.la
#pkgplugin_LTLIBRARIES  = @plugin_mroonga_shared_target@
libmroonga_la_LDFLAGS  = -module -rpath $(pkgplugindir) $(GROONGA_LIBS) $(MECAB_LIBS)
libmroonga_la_CXXFLAGS = $(AM_CFLAGS) -DMYSQL_DYNAMIC_PLUGIN
libmroonga_la_CFLAGS   = $(AM_CFLAGS) -DMYSQL_DYNAMIC_PLUGIN
libmroonga_la_SOURCES  = ha_groonga.cc ha_groonga.h driver.c driver.h

lib_LIBRARIES          = libmroonga.a
#EXTRA_LIBRARIES        = libmroonga.a
#noinst_LIBRARIES       = @plugin_mroonga_static_target@
libmroonga_a_LDFLANGS  = $(GROONGA_LIBS) $(MECAB_LIBS)
libmroonga_a_CXXFLAGS  = $(AM_CFLAGS)
libmroonga_a_CFLAGS    = $(AM_CFLAGS)
libmroonga_a_SOURCES   = ha_groonga.cc driver.c

noinst_LTLIBRARIES = test_driver.la
LDFLAGS = -module -rpath $(libdir) -avoid-version -no-undefined
test_driver_la_SOURCES = test-driver.c
LIBS = $(CUTTER_LIBS)


EXTRA_DIST = plug.in
# Don't update the files from bitkeeper
%::SCCS/s.%