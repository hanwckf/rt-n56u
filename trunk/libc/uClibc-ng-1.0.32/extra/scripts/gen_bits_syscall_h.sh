#!/bin/sh
#
# Copyright (C) 2001 Manuel Novoa III <mjn3@uclibc.org>
# Copyright (C) 2000-2005 Erik Andersen <andersen@uclibc.org>
#
# Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
#

# June 27, 2001         Manuel Novoa III
#
# This script expects  CC (as used in the Makefiles) to be set
# in the environment, and outputs the appropriate bits/sysnum.h #
# corresponding to asm/unistd.h to stdout.
#
# Warning!!! This does _no_ error checking!!!

if [ "${KERNEL_HEADERS:-/}" != "/" ] ; then
  INCLUDE_OPTS="-nostdinc -I${KERNEL_HEADERS}"
else
  # Let the toolchain use its configure paths.
  INCLUDE_OPTS=
fi

case $CC in
*icc*) CC_SYSNUM_ARGS="-dM" ;;
*clang*) CC_SYSNUM_ARGS="-dM" ;;
*)     CC_SYSNUM_ARGS="-dN" ;;
esac

( echo "#include <asm/unistd.h>";
  echo "#include <asm/unistd.h>" |
  $CC -E $CC_SYSNUM_ARGS $INCLUDE_OPTS - |
  sed -n -r \
      -e 's/^[ ]*#define[ ]*(__ARM_NR_|__NR_)([A-Za-z0-9_]*).*/UCLIBC\1\2 \1\2/gp' \
      -e 's/^[ ]*#undef[ ]*(__ARM_NR_|__NR_)([A-Za-z0-9_]*).*/UNDEFUCLIBC\1\2 \1\2/gp' # needed to strip out any kernel-internal defines
) |
$CC -E $INCLUDE_OPTS - |
(
  cat <<-EOF
/* WARNING!!! AUTO-GENERATED FILE!!! DO NOT EDIT!!! */
/* See $0 for more information. */

#ifndef _BITS_SYSNUM_H
#define _BITS_SYSNUM_H

#ifndef _SYSCALL_H
# error "Never use <bits/sysnum.h> directly; include <sys/syscall.h> instead."
#endif

EOF
  sed -n -r -e 's/^UCLIBC(__ARM_NR_|__NR_)([A-Za-z0-9_]*) *(.*)/#undef \1\2\
#define \1\2 \3\
#define SYS_\2 \1\2/gp' \
     -e 's/^UNDEFUCLIBC(__ARM_NR_|__NR_)([A-Za-z0-9_]*).*/#undef \1\2\
#undef SYS_\2/gp'
  cat <<-EOF

#endif
EOF
)
