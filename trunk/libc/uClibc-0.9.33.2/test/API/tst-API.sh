#!/bin/sh

std="$1"
shift
cod="$*"

result=0

for l in $top_builddir/lib/lib*-*.so; do \
	readelf -D -W -s $l | \
	$AWK '
/^[[:space:]]*[[:digit:]]/ { if ($8 != "UND") print $NF; }
'; \
	done | sort | uniq > $uclibc_out
for code in $cod; do cat $code.$std.syms; done | sort | uniq > $glibc_out
result=0
exit $result
