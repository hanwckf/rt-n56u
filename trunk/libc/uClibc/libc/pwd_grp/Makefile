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

TOPDIR=../../
include $(TOPDIR)Rules.mak

MSRC=pwd_grp.c
MOBJ=	fgetpwent_r.o fgetgrent_r.o fgetpwent.o fgetgrent.o \
	getpwnam_r.o getgrnam_r.o getpwuid_r.o getgrgid_r.o \
	getpwuid.o getgrgid.o getpwnam.o getgrnam.o getpw.o \
	getpwent_r.o getgrent_r.o getpwent.o getgrent.o \
	initgroups.o putpwent.o putgrent.o \
	__parsepwent.o __parsegrent.o __pgsreader.o

ifeq ($(HAS_SHADOW),y)
MOBJ+=	fgetspent_r.o fgetspent.o sgetspent_r.o getspnam_r.o \
	getspnam.o getspent_r.o getspent.o sgetspent.o \
	putspent.o __parsespent.o # getspuid_r.o getspuid.o
endif

CSRC=
ifeq ($(HAS_SHADOW),y)
CSRC+= lckpwdf.c
endif

COBJ=$(patsubst %.c,%.o, $(CSRC))
OBJS=$(MOBJ) $(COBJ)

OBJ_LIST=../obj.pwd_grp

all: $(OBJ_LIST)

$(OBJ_LIST): $(OBJS)
	echo $(patsubst %, pwd_grp/%, $(OBJS)) > $(OBJ_LIST)

$(MOBJ): $(MSRC)
	$(CC) $(CFLAGS) -DL_$* $< -c -o $*.o
	$(STRIPTOOL) -x -R .note -R .comment $*.o

$(COBJ): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	$(STRIPTOOL) -x -R .note -R .comment $*.o

$(OBJ): Makefile

clean:
	$(RM) *.[oa] *~ core
