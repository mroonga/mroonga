#!/bin/sh

rename()
{
    old_name=$1
    new_name=$2

    if [ -d $old_name ]; then
        rm -rf $new_name
        mv $old_name $new_name
    fi
}

cd html

rename _static static
rename _sources sources
rename _images images

file_names="`ls *.html`"
for file_name in $file_names; do
    sed -e 's/_static/static/g' $file_name |
    sed -e 's/_sources/sources/g' |
    sed -e 's/_images/images/g' > buf.txt
    cp buf.txt $file_name
    rm -f buf.txt
done
