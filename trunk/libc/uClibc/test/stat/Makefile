# Makefile for uClibc
#
# Copyright (C) 2000,2001 Erik Andersen <andersen@uclibc.org>
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU Library General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option) any
# later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU Library General Public License for more
# details.
#
# You should have received a copy of the GNU Library General Public License
# along with this program; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

include ../Rules.mak

CFLAGS64=-D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64


TARGETS=stat_diff stat64_diff

all: $(TARGETS)

stat_source:
	-@ $(RM) $(TARGETS)
	-@ echo "-------"
	-@ echo "stat.c source: "
	-@ echo " "
	-@ cat stat.c
	-@ echo " "

stat: stat.c Makefile $(TESTDIR)/Rules.mak
	-@ echo "-------"
	-@ echo " "
	-@ echo "Compiling $@ vs uClibc: "
	-@ echo " "
	$(CC) $(CFLAGS) -c $< -o $@.o
	$(CC) $(LDFLAGS) $@.o -o $@ $(EXTRA_LIBS)
	$(STRIPTOOL) -x -R .note -R .comment $@
	./$@ stat.c > $@.out
	-@ echo " "

stat_glibc: stat.c Makefile
	-@ echo "-------"
	-@ echo " "
	-@ echo "Compiling $@ vs glibc: "
	-@ echo " "
	$(HOSTCC) $(GLIBC_CFLAGS) -c $< -o $@.o
	$(HOSTCC) $(GLIBC_LDFLAGS) $@.o -o $@
	$(STRIPTOOL) -x -R .note -R .comment $@
	./$@ stat.c > $@.out
	-@ echo " "

stat_diff: stat stat_glibc
	-@ echo "-------"
	-@ echo " "
	-@ echo "Diffing stat output: "
	-@ echo " "
	-diff -u stat_glibc.out stat.out
	-@ echo " "

stat64: stat.c Makefile $(TESTDIR)/Rules.mak
	-@ echo "-------"
	-@ echo " "
	-@ echo "Compiling $@ vs uClibc: "
	-@ echo " "
	$(CC) $(CFLAGS) $(CFLAGS64) -c $< -o $@.o
	$(CC) $(LDFLAGS) $@.o -o $@ $(EXTRA_LIBS)
	$(STRIPTOOL) -x -R .note -R .comment $@
	./$@ stat.c > $@.out
	-@ echo " "

stat64_glibc: stat.c Makefile
	-@ echo "-------"
	-@ echo " "
	-@ echo "Compiling $@ vs glibc: "
	-@ echo " "
	$(HOSTCC) $(GLIBC_CFLAGS) $(CFLAGS64) -c $< -o $@.o
	$(HOSTCC) $(GLIBC_LDFLAGS) $@.o -o $@
	$(STRIPTOOL) -x -R .note -R .comment $@
	./$@ stat.c > $@.out
	-@ echo " "

stat64_diff: stat64 stat64_glibc
	-@ echo "-------"
	-@ echo " "
	-@ echo "Diffing stat64 output: "
	-@ echo " "
	-diff -u stat64_glibc.out stat64.out
	-@ echo " "

clean:
	$(RM) *.[oa] *~ core stat stat_glibc stat_glibc.out stat.out \
		stat64 stat64_glibc stat64_glibc.out stat64.out
