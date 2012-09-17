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

MSRC1=tsearch.c
MOBJ1=tsearch.o tfind.o tdelete.o twalk.o tdestroy.o

MSRC2=lsearch.c
MOBJ2=lfind.o lsearch.o

MSRC3=insremque.c
MOBJ3=insque.o remque.o

MSRC4=hsearch_r.c
MOBJ4=hcreate_r.o hdestroy_r.o hsearch_r.o

CSRC=hsearch.c
COBJS=$(patsubst %.c,%.o, $(CSRC))

OBJS=$(COBJS) $(MOBJ1) $(MOBJ2) $(MOBJ3) $(MOBJ4)

OBJ_LIST=../../obj.misc.search

all: $(OBJ_LIST)

$(OBJ_LIST): $(OBJS)
	echo $(patsubst %, misc/search/%, $(OBJS)) > $(OBJ_LIST)

$(MOBJ1): $(MSRC1)
	$(CC) $(CFLAGS) -DL_$* $< -c -o $*.o
	$(STRIPTOOL) -x -R .note -R .comment $*.o

$(MOBJ2): $(MSRC2)
	$(CC) $(CFLAGS) -DL_$* $< -c -o $*.o
	$(STRIPTOOL) -x -R .note -R .comment $*.o

$(MOBJ3): $(MSRC3)
	$(CC) $(CFLAGS) -DL_$* $< -c -o $*.o
	$(STRIPTOOL) -x -R .note -R .comment $*.o

$(MOBJ4): $(MSRC4)
	$(CC) $(CFLAGS) -DL_$* $< -c -o $*.o
	$(STRIPTOOL) -x -R .note -R .comment $*.o

$(COBJS): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	$(STRIPTOOL) -x -R .note -R .comment $*.o

clean:
	$(RM) *.[oa] *~ core
