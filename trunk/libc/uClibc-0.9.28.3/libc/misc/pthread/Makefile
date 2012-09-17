# Makefile for uClibc
#
# Copyright (C) 2002 Erik Andersen <andersen@uclibc.org>
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

PTDIR = $(TOPDIR)libpthread/linuxthreads/
SYSDEPINC = -I$(PTDIR)sysdeps/unix/sysv/linux \
            -I$(PTDIR)sysdeps/pthread \
            -I$(PTDIR)sysdeps/unix/sysv \
            -I$(PTDIR)sysdeps/unix/unix \
            -I$(PTDIR)sysdeps/$(TARGET_ARCH) \
            -I$(PTDIR)sysdeps \
            -I$(TOPDIR)libc/sysdeps/linux/$(TARGET_ARCH)
CFLAGS += $(SYSDEPINC) -D_GNU_SOURCE

CSRC=no-tsd.c weaks.c
COBJS=$(patsubst %.c,%.o, $(CSRC))
OBJS=$(COBJS)

OBJ_LIST=../../obj.misc.pthread

all: $(OBJ_LIST)

$(OBJ_LIST): $(OBJS)
	echo $(patsubst %, misc/pthread/%, $(OBJS)) > $(OBJ_LIST)

$(COBJS): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	$(STRIPTOOL) -x -R .note -R .comment $*.o

clean:
	$(RM) *.[oa] *~ core
