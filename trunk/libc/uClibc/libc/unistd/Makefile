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

TOPDIR=../../
include $(TOPDIR)Rules.mak
DIRS:=

MSRC1 = exec.c
MOBJ1 = execl.o execv.o execle.o execlp.o execvp.o

CSRC=	sleep.c usleep.c ualarm.c getpass.c sysconf.c getlogin.c \
	fpathconf.c confstr.c pathconf.c swab.c usershell.c \
	getsubopt.c

ifeq ($(strip $(ARCH_HAS_MMU)),y)
	CSRC += daemon.c
else
	MOBJ1 += __exec_alloc.o
endif

ifeq ($(strip $(UCLIBC_HAS_GNU_GETOPT)),y)
	CSRC += getopt.c
else
	CSRC += getopt-susv3.c
endif

COBJS=$(patsubst %.c,%.o, $(CSRC))
OBJS=$(COBJS) $(MOBJ1)

OBJ_LIST=../obj.unistd

all: $(OBJ_LIST)

$(OBJ_LIST): $(OBJS)
	echo $(patsubst %, unistd/%, $(OBJS)) > $(OBJ_LIST)

$(MOBJ1): $(MSRC1)
	$(CC) $(CFLAGS) -DL_$* $< -c -o $*.o
	$(STRIPTOOL) -x -R .note -R .comment $*.o

$(COBJS): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	$(STRIPTOOL) -x -R .note -R .comment $*.o

$(OBJ): Makefile

subdirs: $(patsubst %, _dir_%, $(DIRS))
subdirs_clean: $(patsubst %, _dirclean_%, $(DIRS))

$(patsubst %, _dir_%, $(DIRS)) : dummy
	$(MAKE) -C $(patsubst _dir_%, %, $@)

$(patsubst %, _dirclean_%, $(DIRS)) : dummy
	$(MAKE) -C $(patsubst _dirclean_%, %, $@) clean

.PHONY: dummy
dummy:

clean:
	$(RM) *.[oa] *~ core

