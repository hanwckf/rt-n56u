# Makefile for uClibc
#
# Copyright (C) 2002,03  NEC Electronics Corporation
# Copyright (C) 2002,03  Miles Bader <miles@gnu.org>
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

CSRC = malloc.c calloc.c free.c realloc.c memalign.c \
	heap_alloc.c heap_alloc_at.c heap_free.c

# Turn on malloc debugging if requested
ifeq ($(UCLIBC_MALLOC_DEBUGGING),y)
CSRC += malloc_debug.c heap_debug.c
CFLAGS += -DMALLOC_DEBUGGING -DHEAP_DEBUGGING
ifeq ($(UCLIBC_UCLINUX_BROKEN_MUNMAP),y)
CFLAGS += -DMALLOC_MMB_DEBUGGING
endif
endif

COBJS=$(patsubst %.c,%.o, $(CSRC))
OBJS=$(COBJS)

OBJ_LIST=../../obj.stdlib.malloc

all: $(OBJ_LIST)

$(OBJ_LIST): $(OBJS)
	echo $(patsubst %, stdlib/malloc/%, $(OBJS)) > $(OBJ_LIST)

malloc.o free.o realloc.o memalign.o: malloc.h
$(COBJS): heap.h

# Depend on uClinux_config.h to cache changes in __UCLIBC_MALLOC_DEBUGGING__
$(COBJS): %.o : %.c ../../../include/bits/uClibc_config.h
	$(CC) $(CFLAGS) -c $< -o $@
	$(STRIPTOOL) -x -R .note -R .comment $*.o

clean:
	$(RM) *.[oa] *~ core
