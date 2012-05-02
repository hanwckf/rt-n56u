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

TOPDIR=../
include $(TOPDIR)Rules.mak

CFLAGS+=$(SSP_ALL_CFLAGS)

LIBCRYPT=libcrypt.a
LIBCRYPT_SHARED=libcrypt.so
LIBCRYPT_SHARED_FULLNAME=libcrypt-$(MAJOR_VERSION).$(MINOR_VERSION).$(SUBLEVEL).so

CSRC = crypt.c des.c md5.c
OBJS=$(patsubst %.c,%.o, $(CSRC))

all: $(LIBCRYPT)

$(LIBCRYPT) ar-target: $(OBJS)
	$(AR) $(ARFLAGS) $(LIBCRYPT) $(OBJS)
	$(INSTALL) -d $(TOPDIR)lib
	$(RM) $(TOPDIR)lib/$(LIBCRYPT)
	$(INSTALL) -m 644 $(LIBCRYPT) $(TOPDIR)lib

$(OBJS): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	$(STRIPTOOL) -x -R .note -R .comment $*.o

$(OBJ): Makefile

shared: all
	$(LD) $(LDFLAGS) -soname=$(LIBCRYPT_SHARED).$(MAJOR_VERSION) \
		-o $(LIBCRYPT_SHARED_FULLNAME) --whole-archive $(LIBCRYPT) \
		--no-whole-archive $(TOPDIR)libc/misc/internals/interp.o \
		-L$(TOPDIR)lib -lc $(LDADD_LIBFLOAT) $(LIBGCC);
	$(INSTALL) -d $(TOPDIR)lib
	$(RM) $(TOPDIR)lib/$(LIBCRYPT_SHARED_FULLNAME) $(TOPDIR)lib/$(LIBCRYPT_SHARED).$(MAJOR_VERSION)
	$(INSTALL) -m 644 $(LIBCRYPT_SHARED_FULLNAME) $(TOPDIR)lib;
	$(LN) -sf $(LIBCRYPT_SHARED_FULLNAME) $(TOPDIR)lib/$(LIBCRYPT_SHARED)
	$(LN) -sf $(LIBCRYPT_SHARED_FULLNAME) $(TOPDIR)lib/$(LIBCRYPT_SHARED).$(MAJOR_VERSION)

clean: 
	$(RM) *.[oa] *~ core $(LIBCRYPT_SHARED)* $(LIBCRYPT_SHARED_FULLNAME)*
