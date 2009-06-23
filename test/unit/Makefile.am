noinst_LTLIBRARIES = test_driver.la

AM_CPPFLAGS = $(GROONGA_INCLUDES) $(MECAB_INCLUDES) $(CUTTER_CFLAGS)
AM_INCLUDES = -l. -l..
ACLOCAL_AMFLAGS = $$ACLOCAL_ARGS

DEFS          = @DEFS@

LDFLAGS = -module -rpath $(libdir) -avoid-version -no-undefined

LIBS = $(CUTTER_LIBS) $(GROONGA_LIBS) $(MECAB_LIBS)

test_driver_la_SOURCES = ../../src/driver.c test-driver.c
