#!/bin/sh

list_paths()
{
    variable_name=$1
    echo "$variable_name = \\"
    sort | \
    sed \
      -e 's,^,\t,' \
      -e 's,$, \\,'
    echo "\t\$(NULL)"
    echo
}

find . -type f -name '*.result' | \
    sed -e 's,\./,,' | \
    list_paths "result_files"
