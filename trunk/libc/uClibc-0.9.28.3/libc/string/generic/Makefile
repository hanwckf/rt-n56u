# Makefile for uClibc
#
# Copyright (C) 2000-2003 Erik Andersen <andersen@uclibc.org>
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

TOPDIR=../../../
include $(TOPDIR)Rules.mak

CSRC=	memchr.c memcmp.c memcpy.c memmem.c memmove.c mempcpy.c memrchr.c \
	memset.c rawmemchr.c strcat.c strchr.c strchrnul.c strcmp.c strcpy.c \
	strcspn.c strlen.c strncat.c strncmp.c strncpy.c strnlen.c \
	strrchr.c strsep.c strspn.c strstr.c strtok_r.c

COBJS=$(patsubst %.c,%.o, $(CSRC))
OBJS=$(COBJS)

OBJ_LIST=../../obj.string.generic

all: $(OBJ_LIST)

$(OBJ_LIST): $(OBJS)
	echo $(patsubst %, string/generic/%, $(OBJS)) > $(OBJ_LIST)

# $(MOBJ): $(MSRC)
# 	$(CC) $(CFLAGS) -DL_$* $< -c -o $*.o
# 	$(STRIPTOOL) -x -R .note -R .comment $*.o

$(COBJS): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	$(STRIPTOOL) -x -R .note -R .comment $*.o

clean:
	$(RM) *.[oa] *~ core
