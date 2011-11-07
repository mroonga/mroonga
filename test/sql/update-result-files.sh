#!/bin/sh

remove_generated_path()
{
    while read path; do
	if [ ! -f "$path.in" ]; then
	    echo "$path"
	fi
    done
}

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

(find . -type f -name '*.result' | remove_generated_path; \
 find . -type f -name '*.result.in') | \
    sed -e 's,\./,,' | \
    list_paths "result_files"
