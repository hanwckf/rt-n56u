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

TARGETS+=teststrtol teststrtol_glibc teststrtol_diff
TARGETS+=qsort qsort_glibc qsort_diff
TARGETS+=teston_exit teston_exit_glibc teston_exit_diff
TARGETS+=testatexit testatexit_glibc testatexit_diff
TARGETS+=ptytest

all: $(TARGETS)

teststrtol_source:
	-@ echo "-------"
	-@ echo "teststrtol.c source: "
	-@ echo " "
	-@ cat teststrtol.c
	-@ echo " "

teststrtol: teststrtol.c Makefile $(TESTDIR)/Rules.mak
	-@ echo "-------"
	-@ echo " "
	-@ echo "Compiling $@ vs uClibc: "
	-@ echo " "
	$(CC) $(CFLAGS) -c $< -o $@.o
	$(CC) $(LDFLAGS) $@.o -o $@ $(EXTRA_LIBS)
	$(STRIPTOOL) -x -R .note -R .comment $@
	-$(LDD) $@
	./$@ > $@.out
	-@ echo " "

teststrtol_glibc: teststrtol.c Makefile
	-@ echo "-------"
	-@ echo " "
	-@ echo "Compiling $@ vs glibc: "
	-@ echo " "
	$(HOSTCC) $(GLIBC_CFLAGS) -c $< -o $@.o
	$(HOSTCC) $(GLIBC_LDFLAGS) $@.o -o $@
	$(STRIPTOOL) -x -R .note -R .comment $@
	-$(LDD) $@
	./$@ > $@.out
	-@ echo " "

teststrtol_diff: teststrtol_glibc teststrtol
	-@ echo "-------"
	-@ echo " "
	-@ echo "Diffing output: "
	-@ echo " "
	diff -u teststrtol_glibc.out teststrtol.out
	-@ echo " "

qsort: qsort.c Makefile $(TESTDIR)/Rules.mak
	-@ echo "-------"
	-@ echo " "
	-@ echo "Compiling $@ vs uClibc: "
	-@ echo " "
	$(CC) $(CFLAGS) -c $< -o $@.o
	$(CC) $(LDFLAGS) $@.o -o $@ $(EXTRA_LIBS)
	$(STRIPTOOL) -x -R .note -R .comment $@
	-$(LDD) $@
	./$@ > $@.out
	-@ echo " "

qsort_glibc: qsort.c Makefile
	-@ echo "-------"
	-@ echo " "
	-@ echo "Compiling $@ vs glibc: "
	-@ echo " "
	$(HOSTCC) $(GLIBC_CFLAGS) -c $< -o $@.o
	$(HOSTCC) $(GLIBC_LDFLAGS) $@.o -o $@
	$(STRIPTOOL) -x -R .note -R .comment $@
	-$(LDD) $@
	./$@ > $@.out
	-@ echo " "

qsort_diff: qsort_glibc qsort
	-@ echo "-------"
	-@ echo " "
	-@ echo "Diffing output: "
	-@ echo " "
	diff -u qsort_glibc.out qsort.out
	-@ echo " "

teston_exit: teston_exit.c Makefile $(TESTDIR)/Rules.mak
	-@ echo "-------"
	-@ echo " "
	-@ echo "Compiling $@ vs uClibc: "
	-@ echo " "
	$(CC) $(CFLAGS) -c $< -o $@.o
	$(CC) $(LDFLAGS) $@.o -o $@ $(EXTRA_LIBS)
	$(STRIPTOOL) -x -R .note -R .comment $@
	-$(LDD) $@
	./$@ > $@.out
	-@ echo " "

teston_exit_glibc: teston_exit.c Makefile
	-@ echo "-------"
	-@ echo " "
	-@ echo "Compiling $@ vs glibc: "
	-@ echo " "
	$(HOSTCC) $(GLIBC_CFLAGS) -c $< -o $@.o
	$(HOSTCC) $(GLIBC_LDFLAGS) $@.o -o $@
	$(STRIPTOOL) -x -R .note -R .comment $@
	-$(LDD) $@
	./$@ > $@.out
	-@ echo " "

teston_exit_diff: teston_exit_glibc teston_exit
	-@ echo "-------"
	-@ echo " "
	-@ echo "Diffing output: "
	-@ echo " "
	diff -u teston_exit_glibc.out teston_exit.out
	-@ echo " "

testatexit: testatexit.c Makefile $(TESTDIR)/Rules.mak
	-@ echo "-------"
	-@ echo " "
	-@ echo "Compiling $@ vs uClibc: "
	-@ echo " "
	$(CC) $(CFLAGS) -c $< -o $@.o
	$(CC) $(LDFLAGS) $@.o -o $@ $(EXTRA_LIBS)
	$(STRIPTOOL) -x -R .note -R .comment $@
	-$(LDD) $@
	./$@ > $@.out
	-@ echo " "

testatexit_glibc: testatexit.c Makefile
	-@ echo "-------"
	-@ echo " "
	-@ echo "Compiling $@ vs glibc: "
	-@ echo " "
	$(HOSTCC) $(GLIBC_CFLAGS) -c $< -o $@.o
	$(HOSTCC) $(GLIBC_LDFLAGS) $@.o -o $@
	$(STRIPTOOL) -x -R .note -R .comment $@
	-$(LDD) $@
	./$@ > $@.out
	-@ echo " "

testatexit_diff: testatexit_glibc testatexit
	-@ echo "-------"
	-@ echo " "
	-@ echo "Diffing output: "
	-@ echo " "
	diff -u testatexit_glibc.out testatexit.out
	-@ echo " "

ptytest: ptytest.c Makefile $(TESTDIR)/Rules.mak
	-@ echo "-------"
	-@ echo " "
	-@ echo "Compiling $@ vs uClibc: "
	-@ echo " "
	$(CC) $(CFLAGS) -c $< -o $@.o
	$(CC) $(LDFLAGS) $@.o -o $@ $(EXTRA_LIBS)
	$(STRIPTOOL) -x -R .note -R .comment $@
	-$(LDD) $@
	./$@
	-@ echo " "


clean:
	$(RM) *.[oa] *~ core $(TARGETS) *.out
