#!/bin/sh

cd build/html

if [ -d _static ]; then
    rm -rf static
    mv _static static
fi
if [ -d _sources ]; then
    rm -rf sources
    mv _sources sources
fi

file_names="`ls *.html`"
for file_name in $file_names; do
    sed -e 's/_static/static/g' $file_name |
    sed -e 's/_sources/sources/g' > buf.txt
    cp buf.txt $file_name
    rm -f buf.txt
done
