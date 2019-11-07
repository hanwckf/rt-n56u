#!/bin/bash

set -ex

# Go to the top-level
topdir=`git rev-parse --show-toplevel`
if [ -z "${topdir}" ]; then
    do_abort "Not in the Git clone"
fi
cd "${topdir}"
git clean -fxdq
./bootstrap
./configure --enable-local
make dist-create-release
