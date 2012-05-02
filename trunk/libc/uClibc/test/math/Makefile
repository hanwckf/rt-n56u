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

CFLAGS+=-D_GNU_SOURCE -DNO_LONG_DOUBLE
EXTRA_LIBS=-lm
PERL=/usr/bin/perl

TARGETS:=
libm-tests=libm-test.c
libm-tests+= test-double test-idouble
libm-tests+= diff
#libm-tests+= test-float test-ifloat
#libm-tests+= test-ldouble test-ildouble
libm-tests.o = $(addsuffix .o,$(libm-tests))

libm-tests-generated = libm-test-ulps.h libm-test.c
generated += $(libm-tests-generated) libm-test.stmp
TARGETS += $(libm-tests) #$(libm-tests-generated)

all: $(TARGETS)
	
test-double: test-double.o
	$(CC) $(LDFLAGS) $@.o -o $@ $(EXTRA_LIBS)
	./$@
test-idouble: test-idouble.o
	$(CC) $(LDFLAGS) $@.o -o $@ $(EXTRA_LIBS)
	./$@
test-float: test-float.o
	$(CC) $(LDFLAGS) $@.o -o $@ $(EXTRA_LIBS)
	./$@
test-ifloat: test-ifloat.o
	$(CC) $(LDFLAGS) $@.o -o $@ $(EXTRA_LIBS)
	./$@
test-ldouble: test-ldouble.o
	$(CC) $(LDFLAGS) $@.o -o $@ $(EXTRA_LIBS)
	./$@
test-ildouble: test-ildoubl.o
	$(CC) $(LDFLAGS) $@.o -o $@ $(EXTRA_LIBS)
	./$@

rint: rint.c Makefile $(TESTDIR)/Rules.mak
	-@ echo "-------"
	-@ echo " "
	-@ echo "Compiling $@ vs uClibc: "
	-@ echo " "
	$(CC) $(CFLAGS) -c $< -o $@.o
	$(CC) $(LDFLAGS) $@.o -o $@ $(EXTRA_LIBS)
	$(STRIPTOOL) -x -R .note -R .comment $@
	./$@ > $@.out 2>&1
	-@ echo " "

rint_glibc: rint.c Makefile $(TESTDIR)/Rules.mak
	-@ echo "-------"
	-@ echo " "
	-@ echo "Compiling $@ vs glibc: "
	-@ echo " "
	$(HOSTCC) $(GLIBC_CFLAGS) -c $< -o $@.o
	$(HOSTCC) $(GLIBC_LDFLAGS) $@.o -o $@ $(EXTRA_LIBS)
	$(STRIPTOOL) -x -R .note -R .comment $@
	./$@ > $@.out 2>&1
	-@ echo " "

diff: rint_glibc rint
	-@ echo "-------"
	-@ echo " "
	-@ echo "Diffing output: "
	-@ echo " "
	-diff -u rint_glibc.out rint.out
	-@ echo " "

test-float.o: libm-test.c
test-ifloat.o: libm-test.c 
test-double.o: libm-test.c
test-idouble.o: libm-test.c
test-ldouble.o: libm-test.c
test-ildoubl.o: libm-test.c

ulps-file = $(firstword $(wildcard $(config-sysdirs:%=$(..)%/libm-test-ulps)))

libm-test.c: $(ulps-file) libm-test.inc gen-libm-test.pl
	$(PERL) ./gen-libm-test.pl -u $< ./libm-test.inc -o "." 2>&1 > /dev/null

clean:
	$(RM) *.[oa] *~ core $(TARGETS) $(generated) rint_glibc.out rint.out
