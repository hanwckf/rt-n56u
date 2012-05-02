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

# Turn on malloc debugging if requested
ifeq ($(UCLIBC_MALLOC_DEBUGGING),y)
CFLAGS += -D__MALLOC_DEBUGGING
endif

# calloc.c can be found at uClibc/libc/stdlib/calloc.c
# valloc.c can be found at uClibc/libc/stdlib/valloc.c
CSRC=malloc.c calloc.c realloc.c free.c memalign.c mallopt.c mallinfo.c
COBJS=$(patsubst %.c,%.o, $(CSRC))
OBJS=$(COBJS)

OBJ_LIST=../../obj.stdlib.malloc-standard

all: $(OBJ_LIST)

$(OBJ_LIST): $(OBJS)
	echo $(patsubst %, stdlib/malloc-standard/%, $(OBJS)) > $(OBJ_LIST)

$(COBJS): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	$(STRIPTOOL) -x -R .note -R .comment $*.o

clean:
	$(RM) *.[oa] *~ core
