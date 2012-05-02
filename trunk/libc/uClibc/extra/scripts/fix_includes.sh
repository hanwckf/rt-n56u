#!/bin/sh
# Copyright (C) 2003 Erik Andersen <andersen@uclibc.org>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later
# version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Library General Public License for more details.
#
# You should have received a copy of the GNU Library General
# Public License along with this program; if not, write to the
# Free Software Foundation, Inc., 59 Temple Place, Suite 330,
# Boston, MA 02111-1307 USA

usage () {
    echo ""
    echo "usage: "`basename $0`" -k KERNEL_SOURCE_DIRECTORY -t TARGET_ARCH"
    echo ""
    echo "This utility scans the KERNEL_SOURCE_DIRECTORY directory and"
    echo "checks that it contains well formed kernel headers suitable"
    echo "for inclusion as the include/linux/ directory provided by"
    echo "uClibc."
    echo ""
    echo "If the specified kernel headers are present and already"
    echo "configured for the architecture specified by TARGET_ARCH,"
    echo "they will be used as-is."
    echo ""
    echo "If the specified kernel headers are missing entirely, this"
    echo "script will return an error."
    echo ""
    echo "If the specified kernel headers are present, but are either"
    echo "not yet configured or are configured for an architecture"
    echo "different than that specified by TARGET_ARCH, this script"
    echo "will attempt to 'fix' the kernel headers and make them"
    echo "suitable for use by uClibc.  This fixing process may fail."
    echo "It is therefore best to always provide kernel headers that"
    echo "are already configured for the selected architecture."
    echo ""
    echo "Most Linux distributions provide 'kernel-headers' packages"
    echo "that are suitable for use by uClibc."
    echo ""
    echo ""
    exit 1;
}

HAS_MMU="y";
while [ -n "$1" ]; do
    case $1 in
	-k ) shift; if [ -n "$1" ]; then KERNEL_SOURCE=$1; shift; else usage; fi; ;;
	-t ) shift; if [ -n "$1" ]; then TARGET_ARCH=$1; shift; else usage; fi; ;;
	-n ) shift; HAS_MMU="n"; ;;
	-* ) usage; ;;
	* ) usage; ;;
    esac;
done;

if [ ! -f "$KERNEL_SOURCE/Makefile" -a ! -f "$KERNEL_SOURCE/include/linux/version.h" ]; then
    echo "";
    echo "";
    echo "The file $KERNEL_SOURCE/Makefile or $KERNEL_SOURCE/include/linux/version.h is missing!";
    echo "Perhaps your kernel source is broken?"
    echo "";
    echo "";
    exit 1;
fi;

if [ ! -d "$KERNEL_SOURCE" ]; then
    echo "";
    echo "";
    echo "$KERNEL_SOURCE is not a directory";
    echo "";
    echo "";
    exit 1;
fi;

# Create a symlink to include/asm

rm -f include/asm*
if [ ! -d "$KERNEL_SOURCE/include/asm" ]; then
    echo "";
    echo "";
    echo "The symlink $KERNEL_SOURCE/include/asm is missing\!";
    echo "Perhaps you forgot to configure your kernel source?";
    echo "You really should configure your kernel source tree so I";
    echo "do not have to try and guess about this sort of thing.";
    echo ""
    echo "Attempting to guess a usable value....";
    echo ""
    echo "";
    sleep 1;

    if [ "$TARGET_ARCH" = "powerpc" ];then
	set -x;
	ln -fs $KERNEL_SOURCE/include/asm-ppc include/asm;
	set +x;
    elif [ "$TARGET_ARCH" = "mips" ];then
	set -x;
	ln -fs $KERNEL_SOURCE/include/asm-mips include/asm;
	set +x;
    elif [ "$TARGET_ARCH" = "arm" ];then
	set -x;
	ln -fs $KERNEL_SOURCE/include/asm-arm include/asm;
	set +x;
	if [ ! -L $KERNEL_SOURCE/include/asm-arm/proc ] ; then
	    if [ ! -L proc ] ; then
		(cd include/asm;
		ln -fs proc-armv proc;
		ln -fs arch-ebsa285 arch);
	    fi
	fi;
    elif [ "$TARGET_ARCH" = "cris" ]; then
	set -x;
	ln -fs $KERNEL_SOURCE/include/asm-cris include/asm;
	set +x;
    elif [ "$HAS_MMU" != "y" ]; then
	    if [ -d $KERNEL_SOURCE/include/asm-${TARGET_ARCH}nommu ] ; then
		set -x;
		ln -fs $KERNEL_SOURCE/include/asm-${TARGET_ARCH}nommu include/asm;
		set +x;
	    else
		set -x;
		ln -fs $KERNEL_SOURCE/include/asm-$TARGET_ARCH include/asm;
		set +x;
	    fi;
    else
	set -x;
	ln -fs $KERNEL_SOURCE/include/asm-$TARGET_ARCH include/asm;
	set +x;
    fi;
else
# No guessing required.....
ln -fs $KERNEL_SOURCE/include/asm include/asm
if [ -e $KERNEL_SOURCE/include/asm-$TARGET_ARCH ] ; then
ln -fs $KERNEL_SOURCE/include/asm-$TARGET_ARCH include/asm-$TARGET_ARCH
fi
fi;


# Annoyingly, 2.6.x kernel headers also need an include/asm-generic/ directory
if [ -e $KERNEL_SOURCE/include/asm-generic ]; then
    ln -fs $KERNEL_SOURCE/include/asm-generic include/asm-generic
fi;


# Create the include/linux symlink.
rm -f include/linux
ln -fs $KERNEL_SOURCE/include/linux include/linux

