SUBDIRS = unit

if WITH_CUTTER
TESTS = run-unit-test.sh
TESTS_ENVIRONMENT = CUTTER="$(CUTTER)"
endif

echo-cutter:
	echo $(CUTTER)
