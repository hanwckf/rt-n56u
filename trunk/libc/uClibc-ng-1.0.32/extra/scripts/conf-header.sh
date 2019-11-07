#!/bin/sh -e

# Turn .config into a header file

if [ -z "$1" ] ; then
	echo "Usage: conf-header.sh <.config>"
	exit 1
fi

cat <<EOF
#if !defined _FEATURES_H && !defined __need_uClibc_config_h
# error Never include <bits/uClibc_config.h> directly; use <features.h> instead
#endif

#define __UCLIBC_MAJOR__ ${MAJOR_VERSION}
#define __UCLIBC_MINOR__ ${MINOR_VERSION}
#define __UCLIBC_SUBLEVEL__ ${SUBLEVEL}
EOF

exec \
sed \
	-e '/^#$/d' \
	-e '/^[^#]/s:^\([^=]*\)=\(.*\):#define __\1__ \2:' \
	-e '/^#define /s: y$: 1:' \
	-e '/^# .* is not set$/s:^# \(.*\) is not set$:#undef __\1__:' \
	-e 's:^# \(.*\)$:/* \1 */:' \
	$1
