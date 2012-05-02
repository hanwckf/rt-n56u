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

MSRC= wctype.c
MOBJ=	iswalnum.o iswalpha.o iswcntrl.o iswdigit.o iswgraph.o \
	iswlower.o iswprint.o iswpunct.o iswspace.o iswupper.o \
	iswxdigit.o iswblank.o wctrans.o towctrans.o \
	wctype.o iswctype.o towlower.o towupper.o

MOBJx=	iswalnum_l.o iswalpha_l.o iswcntrl_l.o iswdigit_l.o iswgraph_l.o \
	iswlower_l.o iswprint_l.o iswpunct_l.o iswspace_l.o iswupper_l.o \
	iswxdigit_l.o iswblank_l.o \
	wctype_l.o iswctype_l.o wctrans_l.o towctrans_l.o towlower_l.o towupper_l.o

OBJS=$(MOBJ)

ifeq ($(UCLIBC_HAS_XLOCALE),y)
	OBJS += $(MOBJx)
endif

OBJ_LIST=../../obj.misc.wctype

all: $(OBJ_LIST)

$(OBJ_LIST): $(OBJS)
	echo $(patsubst %, misc/wctype/%, $(OBJS)) > $(OBJ_LIST)

$(MOBJ): $(MSRC)
	$(CC) $(CFLAGS) -DL_$* $< -c -o $*.o
	$(STRIPTOOL) -x -R .note -R .comment $*.o

$(MOBJx): $(MSRC)
	$(CC) $(CFLAGS) -DL_$* -D__UCLIBC_DO_XLOCALE $< -c -o $*.o
	$(STRIPTOOL) -x -R .note -R .comment $*.o

clean:
	$(RM) *.[oa] *~ core
