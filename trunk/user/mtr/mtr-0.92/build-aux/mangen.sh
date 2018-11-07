#!/bin/sh

#
#  Generate the man pages.
#
#  We are just here to substitute the @VERSION@ string with our real version.
#

if [ $# -lt 3 ]; then
    echo Usage: mangen.sh VERSION IN OUT
    exit 1
fi

sed -e "s|@VERSION[@]|$1|g" $2 >$3
