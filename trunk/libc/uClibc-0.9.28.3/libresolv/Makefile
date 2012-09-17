# Makefile for uClibc
#
# Copyright (C) 2000 by Lineo, inc.
# Copyright (C) 2000-2002 Erik Andersen <andersen@uclibc.org>
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
#
# Derived in part from the Linux-8086 C library, the GNU C Library, and several
# other sundry sources.  Files within this library are copyright by their
# respective copyright holders.
# Makefile for uClibc

TOPDIR=../
include $(TOPDIR)Rules.mak

CFLAGS+=$(SSP_ALL_CFLAGS)

LIBRESOLV=libresolv.a
LIBRESOLV_SHARED=libresolv.so
LIBRESOLV_SHARED_FULLNAME=libresolv-$(MAJOR_VERSION).$(MINOR_VERSION).$(SUBLEVEL).so

CSRC = resolv.c ns_name.c
OBJS=$(patsubst %.c,%.o, $(CSRC))

all: $(LIBRESOLV)

$(LIBRESOLV) ar-target: $(OBJS)
	$(AR) $(ARFLAGS) $(LIBRESOLV) $(OBJS)
	$(INSTALL) -d $(TOPDIR)lib
	$(RM) $(TOPDIR)lib/$(LIBRESOLV)
	$(INSTALL) -m 644 $(LIBRESOLV) $(TOPDIR)lib

$(OBJS): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	$(STRIPTOOL) -x -R .note -R .comment $*.o

$(OBJ): Makefile

shared: all
	$(LD) $(LDFLAGS) -soname=$(LIBRESOLV_SHARED).$(MAJOR_VERSION) \
		-o $(LIBRESOLV_SHARED_FULLNAME) --whole-archive $(LIBRESOLV) \
		--no-whole-archive $(TOPDIR)libc/misc/internals/interp.o \
		-L$(TOPDIR)lib -lc $(LDADD_LIBFLOAT) $(LIBGCC);
	$(INSTALL) -d $(TOPDIR)lib
	$(RM) $(TOPDIR)lib/$(LIBRESOLV_SHARED_FULLNAME) $(TOPDIR)lib/$(LIBRESOLV_SHARED).$(MAJOR_VERSION)
	$(INSTALL) -m 644 $(LIBRESOLV_SHARED_FULLNAME) $(TOPDIR)lib
	$(LN) -sf $(LIBRESOLV_SHARED_FULLNAME) $(TOPDIR)lib/$(LIBRESOLV_SHARED)
	$(LN) -sf $(LIBRESOLV_SHARED_FULLNAME) $(TOPDIR)lib/$(LIBRESOLV_SHARED).$(MAJOR_VERSION)

clean: 
	$(RM) *.[oa] *~ core $(LIBRESOLV_SHARED)* $(LIBRESOLV_SHARED_FULLNAME)*
