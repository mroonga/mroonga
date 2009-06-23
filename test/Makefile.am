AM_CPPFLAGS = $(GROONGA_INCLUDES) $(MECAB_INCLUDES) $(CUTTER_CFLAGS)
AM_INCLUDES = -l. -l..
ACLOCAL_AMFLAGS = $$ACLOCAL_ARGS

WRAPLIBS      =
LDADD         =
DEFS          = @DEFS@

noinst_LTLIBRARIES = test_driver.la
LDFLAGS = -module -rpath $(libdir) -avoid-version -no-undefined
test_driver_la_SOURCES = ../src/driver.c test-driver.c
LIBS = $(CUTTER_LIBS) $(GROONGA_LIBS) $(MECAB_LIBS)

TESTS = run-test.sh
TESTS_ENVIRONMENT = CUTTER="$(CUTTER)"

echo-cutter:
	echo $(CUTTER)
