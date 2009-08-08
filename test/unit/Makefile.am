noinst_LTLIBRARIES = test_start_stop.la test_driver.la api_sample.la

AM_CPPFLAGS = $(GROONGA_INCLUDES) $(MECAB_INCLUDES) $(CUTTER_CFLAGS) $(GCUTTER_CFLAGS)
AM_INCLUDES = -l. -l..
ACLOCAL_AMFLAGS = $$ACLOCAL_ARGS

DEFS          = @DEFS@

LDFLAGS = -module -rpath $(libdir) -avoid-version -no-undefined

LIBS = $(CUTTER_LIBS) $(GROONGA_LIBS) $(MECAB_LIBS)

DRIVER_SRC = ../../driver.c

test_start_stop_la_SOURCES = $(DRIVER_SRC) test-start-stop.c
test_driver_la_SOURCES = $(DRIVER_SRC) test-driver.c
api_sample_la_SOURCES = api-sample.c
