#!/bin/sh

aclocal $ACLOCAL_OPTS
autoheader
automake --add-missing --copy --foreign
autoconf --force
