# Makefile for uClibc
#
# Copyright (C) 2002-2003 Erik Andersen <andersen@uclibc.org>
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

TOPDIR=../
include $(TOPDIR)Rules.mak

CFLAGS+=$(SSP_ALL_CFLAGS)

LIBNSL=libnsl.a
LIBNSL_SHARED=libnsl.so
LIBNSL_SHARED_FULLNAME=libnsl-$(MAJOR_VERSION).$(MINOR_VERSION).$(SUBLEVEL).so

CSRC = nsl.c
OBJS=$(patsubst %.c,%.o, $(CSRC))

all: $(LIBNSL)

$(LIBNSL) ar-target: $(OBJS)
	$(AR) $(ARFLAGS) $(LIBNSL) $(OBJS)
	$(INSTALL) -d $(TOPDIR)lib
	$(RM) $(TOPDIR)lib/$(LIBNSL)
	$(INSTALL) -m 644 $(LIBNSL) $(TOPDIR)lib

$(OBJS): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	$(STRIPTOOL) -x -R .note -R .comment $*.o

$(OBJ): Makefile

shared: all
	$(LD) $(LDFLAGS) -soname=$(LIBNSL_SHARED).$(MAJOR_VERSION) \
		-o $(LIBNSL_SHARED_FULLNAME) --whole-archive $(LIBNSL) \
		--no-whole-archive $(TOPDIR)libc/misc/internals/interp.o \
		-L$(TOPDIR)lib -lc $(LDADD_LIBFLOAT) $(LIBGCC);
	$(INSTALL) -d $(TOPDIR)lib
	$(RM) $(TOPDIR)lib/$(LIBNSL_SHARED_FULLNAME) $(TOPDIR)lib/$(LIBNSL_SHARED).$(MAJOR_VERSION)
	$(INSTALL) -m 644 $(LIBNSL_SHARED_FULLNAME) $(TOPDIR)lib
	$(LN) -sf $(LIBNSL_SHARED_FULLNAME) $(TOPDIR)lib/$(LIBNSL_SHARED)
	$(LN) -sf $(LIBNSL_SHARED_FULLNAME) $(TOPDIR)lib/$(LIBNSL_SHARED).$(MAJOR_VERSION)

clean: 
	$(RM) *.[oa] *~ core $(LIBNSL_SHARED)* $(LIBNSL_SHARED_FULLNAME)*
