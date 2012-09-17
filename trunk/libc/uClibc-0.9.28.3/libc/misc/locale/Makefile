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

TOPDIR=../../../
include $(TOPDIR)Rules.mak

MSRC= locale.c
MOBJ= setlocale.o localeconv.o _locale_init.o nl_langinfo.o
MOBJx=

ifeq ($(UCLIBC_HAS_LOCALE),y)
	MOBJ += newlocale.o __locale_mbrtowc_l.o
endif

ifeq ($(UCLIBC_HAS_XLOCALE),y)
	MOBJx += nl_langinfo_l.o duplocale.o freelocale.o uselocale.o __curlocale.o
endif

DATA=
ifeq ($(UCLIBC_HAS_LOCALE),y)
	DATA = locale_data.o
endif

OBJS= $(MOBJ) $(MOBJx) $(DATA)

OBJ_LIST=../../obj.misc.locale

all: $(OBJ_LIST)

$(OBJ_LIST): $(OBJS)
	echo $(patsubst %, misc/locale/%, $(OBJS)) > $(OBJ_LIST)

$(MOBJ): $(MSRC)
	$(CC) $(CFLAGS) -DL_$* $< -c -o $*.o
	$(STRIPTOOL) -x -R .note -R .comment $*.o

$(MOBJx): $(MSRC)
	$(CC) $(CFLAGS) -DL_$* -D__UCLIBC_DO_XLOCALE $< -c -o $*.o
	$(STRIPTOOL) -x -R .note -R .comment $*.o

locale_data.o:
	$(LN) -sf ../../../extra/locale/locale_data.c
	$(CC) $(CFLAGS) -c -D__WCHAR_ENABLED -I../../../extra/locale locale_data.c -o $@
	$(STRIPTOOL) -x -R .note -R .comment $*.o

$(OBJS): Makefile

clean:
	$(RM) *.[oa] *~ core locale_data.c

.PHONY: data
