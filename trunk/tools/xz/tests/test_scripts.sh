#!/bin/sh

###############################################################################
#
# Author: Jonathan Nieder
#
# This file has been put into the public domain.
# You can do whatever you want with this file.
#
###############################################################################

# If scripts weren't built, this test is skipped.
XZ=../src/xz/xz
XZDIFF=../src/scripts/xzdiff
test -x "$XZ" || XZ=
test -x "$XZDIFF" || XZDIFF=
if test -z "$XZ" || test -z "$XZDIFF"; then
	(exit 77)
	exit 77
fi

PATH=`pwd`/../src/xz:$PATH
export PATH

preimage=$srcdir/files/good-1-check-crc32.xz
samepostimage=$srcdir/files/good-1-check-crc64.xz
otherpostimage=$srcdir/files/good-1-lzma2-1.xz

"$XZDIFF" "$preimage" "$samepostimage" >/dev/null
status=$?
if test "$status" != 0 ; then
	echo "xzdiff with no changes exited with status $status != 0"
	(exit 1)
	exit 1
fi

"$XZDIFF" "$preimage" "$otherpostimage" >/dev/null
status=$?
if test "$status" != 1 ; then
	echo "xzdiff with changes exited with status $status != 1"
	(exit 1)
	exit 1
fi

"$XZDIFF" "$preimage" "$srcdir/files/missing.xz" >/dev/null 2>&1
status=$?
if test "$status" != 2 ; then
	echo "xzdiff with missing operand exited with status $status != 2"
	(exit 1)
	exit 1
fi

(exit 0)
exit 0
