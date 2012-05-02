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

LIBINTL=libintl.a
LIBINTL_SHARED=libintl.so
LIBINTL_SHARED_FULLNAME=libintl-$(MAJOR_VERSION).$(MINOR_VERSION).$(SUBLEVEL).so

MSRC= intl.c
MOBJ= gettext.o ngettext.o  dgettext.o dcgettext.o dngettext.o dcngettext.o \
	textdomain.o bindtextdomain.o bind_textdomain_codeset.o \
	_nl_expand_alias.o _nl_msg_cat_cntr.o # glibc-isms

OBJS=$(MOBJ)

all: $(LIBINTL)

$(LIBINTL) ar-target: $(OBJS)
	$(AR) $(ARFLAGS) $(LIBINTL) $(OBJS)
	$(INSTALL) -d $(TOPDIR)lib
	$(RM) $(TOPDIR)lib/$(LIBINTL)
	$(INSTALL) -m 644 $(LIBINTL) $(TOPDIR)lib/

$(MOBJ): $(MSRC)
	$(CC) $(CFLAGS) -DL_$* $< -c -o $*.o
	$(STRIPTOOL) -x -R .note -R .comment $*.o

shared: all
	$(LD) $(LDFLAGS) -soname=$(LIBINTL_SHARED).$(MAJOR_VERSION) \
		-o $(LIBINTL_SHARED_FULLNAME) --whole-archive $(LIBINTL) \
		--no-whole-archive $(TOPDIR)libc/misc/internals/interp.o \
		-L$(TOPDIR)lib -lc $(LDADD_LIBFLOAT) $(LIBGCC);
	$(INSTALL) -d $(TOPDIR)lib
	$(RM) $(TOPDIR)lib/$(LIBINTL_SHARED_FULLNAME) $(TOPDIR)lib/$(LIBINTL_SHARED).$(MAJOR_VERSION)
	$(INSTALL) -m 644 $(LIBINTL_SHARED_FULLNAME) $(TOPDIR)lib
	$(LN) -sf $(LIBINTL_SHARED_FULLNAME) $(TOPDIR)lib/$(LIBINTL_SHARED)
	$(LN) -sf $(LIBINTL_SHARED_FULLNAME) $(TOPDIR)lib/$(LIBINTL_SHARED).$(MAJOR_VERSION)

clean:
	$(RM) *.[oa] *~ core $(LIBINTL_SHARED)* $(LIBINTL_SHARED_FULLNAME)*
