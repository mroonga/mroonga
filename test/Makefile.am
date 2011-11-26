SUBDIRS = unit sql

TESTS = run-sql-test.sh
TESTS_ENVIRONMENT =				\
	NO_MAKE="yes"				\
	MYSQL_SOURCE="$(MYSQL_SOURCE)"		\
	MYSQL_BUILD="$(MYSQL_BUILD)"		\
	MYSQL_VERSION="$(MYSQL_VERSION)"

if WITH_CUTTER
TESTS += run-unit-test.sh
TESTS_ENVIRONMENT += CUTTER="$(CUTTER)"
endif

EXTRA_DIST =					\
	run-unit-test.sh			\
	run-sql-test.sh
