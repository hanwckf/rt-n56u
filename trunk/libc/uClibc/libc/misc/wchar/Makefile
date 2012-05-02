# Makefile for uClibc
#
# Copyright (C) 2000 by Lineo, inc.
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
#
# Derived in part from the Linux-8086 C library, the GNU C Library, and several
# other sundry sources.  Files within this library are copyright by their
# respective copyright holders.

TOPDIR=../../../
include $(TOPDIR)Rules.mak

MSRC1=  wchar.c
MOBJ1=  btowc.o wctob.o mbsinit.o mbrlen.o mbrtowc.o wcrtomb.o mbsrtowcs.o \
	wcsrtombs.o _wchar_utf8sntowcs.o _wchar_wcsntoutf8s.o \
	__mbsnrtowcs.o __wcsnrtombs.o wcwidth.o wcswidth.o

ifeq ($(UCLIBC_HAS_LOCALE),y)
	MOBJ1 += iconv.o
endif

# The stdio and time related wide functions are now built in the normal
# directories.
#
# stdio: 
#	fwide fgetwc getwchar fgetws fputwc putwchar fputws ungetwc
#	getwc (fgetwc alias) getwc_unlocked (fgetwc_unlocked alias)
#	putwc (fputwc alias) putwc_unlocked (fputwc_unlocked alias)
# time:
# wcsftime

OBJS=$(MOBJ1)

OBJ_LIST=../../obj.misc.wchar

all: $(OBJ_LIST)

$(OBJ_LIST): $(OBJS)
	echo $(patsubst %, misc/wchar/%, $(OBJS)) > $(OBJ_LIST)

$(MOBJ1): $(MSRC1)
	$(CC) $(CFLAGS) -DL_$* $< -c -o $*.o
	$(STRIPTOOL) -x -R .note -R .comment $*.o

$(MOBJ2): $(MSRC2)
	$(CC) $(CFLAGS) -DL_$* $< -c -o $*.o
	$(STRIPTOOL) -x -R .note -R .comment $*.o

clean:
	$(RM) *.[oa] *~ core
