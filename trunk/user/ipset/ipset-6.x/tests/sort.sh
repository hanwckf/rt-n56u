#!/bin/sh

head -n 6 $1 > .foo
tail -n +7 $1 | grep  '[[:alnum:]]' | sort >> .foo
rm $1
