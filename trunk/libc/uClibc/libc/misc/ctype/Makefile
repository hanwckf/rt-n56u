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

MSRC=ctype.c
MOBJ=	isalnum.o isalpha.o isascii.o iscntrl.o isdigit.o \
	isgraph.o islower.o isprint.o ispunct.o isspace.o \
	isupper.o isxdigit.o toascii.o tolower.o toupper.o \
	isblank.o isxlower.o isxupper.o

ifeq ($(UCLIBC_HAS_CTYPE_TABLES),y)
MOBJ += __C_ctype_b.o __C_ctype_tolower.o __C_ctype_toupper.o \
	__ctype_b_loc.o __ctype_tolower_loc.o __ctype_toupper_loc.o \
	__ctype_assert.o isctype.o
endif

MOBJx=	isalnum_l.o isalpha_l.o isascii_l.o iscntrl_l.o isdigit_l.o \
	isgraph_l.o islower_l.o isprint_l.o ispunct_l.o isspace_l.o \
	isupper_l.o isxdigit_l.o toascii_l.o tolower_l.o toupper_l.o \
	isblank_l.o # isxlower_l.o isxupper_l.o

OBJS=$(MOBJ)

ifeq ($(UCLIBC_HAS_XLOCALE),y)
	OBJS += $(MOBJx)
endif

OBJ_LIST=../../obj.misc.ctype

all: $(OBJ_LIST)

$(OBJ_LIST): $(OBJS)
	echo $(patsubst %, misc/ctype/%, $(OBJS)) > $(OBJ_LIST)

$(MOBJ): $(MSRC)
	$(CC) $(CFLAGS) -DL_$* $< -c -o $*.o
	$(STRIPTOOL) -x -R .note -R .comment $*.o

$(MOBJx): $(MSRC)
	$(CC) $(CFLAGS) -DL_$* -D__UCLIBC_DO_XLOCALE $< -c -o $*.o
	$(STRIPTOOL) -x -R .note -R .comment $*.o

$(OBJS): Makefile

clean:
	$(RM) *.[oa] *~ core
