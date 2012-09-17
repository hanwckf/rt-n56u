#!/bin/sh

ret=0

# Make sure nothing uses the ARCH_HAS_MMU option anymore
result=$(
find ../.. \
	| grep -v \
		-e include/bits/uClibc_config.h \
		-e /test/ \
		-e /.svn/ \
	| xargs grep -sHI \
		__ARCH_HAS_MMU__
)
if [ -n "$result" ] ; then
	echo "The build system is incorrectly using ARCH_HAS_MMU:"
	echo "$result"
	ret=1
fi

exit $ret
