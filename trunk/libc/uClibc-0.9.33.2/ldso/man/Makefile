# Makefile for uClibc
#
# Copyright (C) 2000,2001,2005 Erik Andersen <andersen@uclibc.org>
#
# Derived in part from the Linux-8086 C library, the GNU C Library, and several
# other sundry sources.  Files within this library are copyright by their
# respective copyright holders.
#
# Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
#

include ../Config.mk

ALL = #ld.so.info

all:	$(ALL)

ld.so.info: ld.so.texi
	makeinfo $<

clean:
	$(RM) $(ALL) *~
