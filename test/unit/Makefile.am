noinst_LTLIBRARIES = test_mrnsys.la

AM_CPPFLAGS = $(GROONGA_INCLUDES) $(MECAB_INCLUDES) $(CUTTER_CFLAGS) $(GCUTTER_CFLAGS)
AM_INCLUDES = -l. -l..
ACLOCAL_AMFLAGS = $$ACLOCAL_ARGS

DEFS          = @DEFS@

LDFLAGS = -module -rpath $(libdir) -avoid-version -no-undefined

LIBS = $(CUTTER_LIBS) $(GROONGA_LIBS) $(MECAB_LIBS)

EXTERNAL_SRC = ../../mrnsys.c

test_mrnsys_la_SOURCES = $(EXTERNAL_SRC) test_mrnsys.c
