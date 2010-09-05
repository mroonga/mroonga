SUBDIRS = unit sql

if WITH_CUTTER
TESTS = run-unit-test.sh
TESTS_ENVIRONMENT = CUTTER="$(CUTTER)"
endif

EXTRA_DIST =					\
	run-unit-test.sh			\
	run-sql-test.sh

echo-cutter:
	echo $(CUTTER)
