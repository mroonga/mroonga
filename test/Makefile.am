SUBDIRS = unit sql

TESTS = run-sql-test.sh
TESTS_ENVIRONMENT = MYSQL_BUILD="$(MYSQL_BUILD)"

if WITH_CUTTER
TESTS += run-unit-test.sh
TESTS_ENVIRONMENT += CUTTER="$(CUTTER)"
endif

EXTRA_DIST =					\
	run-unit-test.sh			\
	run-sql-test.sh

echo-mysql-build:
	echo $(MYSQL_BUILD)

echo-cutter:
	echo $(CUTTER)
