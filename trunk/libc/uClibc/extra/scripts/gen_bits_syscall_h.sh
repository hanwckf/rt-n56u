#!/bin/sh
#
# June 27, 2001         Manuel Novoa III
#
# This script expects TOPDIR and CC (as used in the Makefiles) to be set in
# the environment, and outputs the appropriate $TOPDIR/include/bits/sysnum.h
# corresponding to $TOPDIR/include/asm/unistd.h to stdout.
#
# Warning!!! This does _no_ error checking!!!

UNISTD_H_PATH=$TOPDIR/include/asm/unistd.h
INCLUDE_OPTS="-I$TOPDIR/include"

( echo "#include \"$UNISTD_H_PATH\"" ;
  $CC -E -dN $INCLUDE_OPTS $UNISTD_H_PATH | # needed to strip out any kernel-internal defines
  sed -ne 's/^[ ]*#define[ ]*__NR_\([A-Za-z0-9_]*\).*/UCLIBC_\1 __NR_\1/gp'
) |
$CC -E $INCLUDE_OPTS - |
( echo "/* WARNING!!! AUTO-GENERATED FILE!!! DO NOT EDIT!!! */" ; echo ;
  echo "#ifndef _BITS_SYSNUM_H" ;
  echo "#define _BITS_SYSNUM_H" ;
  echo ;
  echo "#ifndef _SYSCALL_H" ;
  echo "# error \"Never use <bits/sysnum.h> directly; include <sys/syscall.h> instead.\"" ;
  echo "#endif" ; echo ;
  sed -ne 's/^UCLIBC_\([A-Za-z0-9_]*\) *\(.*\)/#undef __NR_\1\
#define __NR_\1 \2\
#define SYS_\1 __NR_\1/gp' \
     -e 's/^UNDEFUCLIBC_\([A-Za-z0-9_]*\).*/#undef __NR_\1/gp'
  echo ;
  echo "#endif" ;
)
