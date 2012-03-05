#!/bin/sh

list_paths()
{
    variable_name=$1
    echo "$variable_name = \\"
    LC_ALL=C sort | \
    sed \
      -e 's,^,\t,' \
      -e 's,$, \\,'
    echo "\t\$(NULL)"
    echo
}

find . -type f -name '*.test' | \
    sed -e 's,\./,,' | \
    list_paths "test_files"

find . -type f -name '*.test.in' | \
    sed -e 's,\./,,' -e 's,\.in$,,' | \
    list_paths "generated_files"
