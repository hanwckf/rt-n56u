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

MSRC= time.c
MOBJ= asctime.o asctime_r.o clock.o ctime.o ctime_r.o gmtime.o gmtime_r.o \
	localtime.o localtime_r.o mktime.o strftime.o strptime.o tzset.o \
	_time_t2tm.o __time_tm.o _time_mktime.o dysize.o timegm.o \
	_time_mktime_tzi.o _time_localtime_tzi.o
MOBJx=

ifeq ($(UCLIBC_HAS_FLOATS),y)
	MOBJ += difftime.o 
endif
ifeq ($(UCLIBC_HAS_XLOCALE),y)
	MOBJx += strftime_l.o strptime_l.o
endif

ifeq ($(UCLIBC_HAS_WCHAR),y)
	MOBJ += wcsftime.o
ifeq ($(UCLIBC_HAS_XLOCALE),y)
	MOBJx += wcsftime_l.o
endif
endif

CSRC= adjtime.c ftime.c
COBJS=$(patsubst %.c,%.o, $(CSRC))

OBJS=$(COBJS) $(MOBJ) $(MOBJx)

OBJ_LIST=../../obj.misc.time

all: $(OBJ_LIST)

$(OBJ_LIST): $(OBJS)
	echo $(patsubst %, misc/time/%, $(OBJS)) > $(OBJ_LIST)

$(MOBJ): $(MSRC)
	$(CC) $(CFLAGS) -DL_$* $< -c -o $*.o
	$(STRIPTOOL) -x -R .note -R .comment $*.o

$(MOBJx): $(MSRC)
	$(CC) $(CFLAGS) -DL_$* -D__UCLIBC_DO_XLOCALE $< -c -o $*.o
	$(STRIPTOOL) -x -R .note -R .comment $*.o

$(COBJS): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	$(STRIPTOOL) -x -R .note -R .comment $*.o

clean:
	$(RM) *.[oa] *~ core
