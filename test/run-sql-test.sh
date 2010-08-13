#!/bin/sh

if test -n "$MYSQL_HOME"; then
    SQL_TESTS="$MYSQL_HOME/mysql-test/suite/groonga"
    if test -d "$SQL_TESTS"; then
	cd $MYSQL_HOME/mysql-test
	./mysql-test-run.pl --suite=groonga --force
    else
	echo "$SQL_TESTS does not exists. Aborting SQL test."
    fi
else
    echo '$MYSQL_HOME is not defined. Aborting SQL test.'
fi
