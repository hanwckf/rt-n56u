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

CSRC= allocrtsig.c killpg.c raise.c sigaction.c sigaddset.c sigandset.c \
	sigblock.c sigdelset.c sigempty.c sigfillset.c siggetmask.c sighold.c \
	sigignore.c sigintr.c sigisempty.c sigismem.c sigjmp.c signal.c \
	sigorset.c sigpause.c sigrelse.c sigset.c sigsetmask.c sigsetops.c \
	sysv_signal.c sigwait.c
COBJS=$(patsubst %.c,%.o, $(CSRC))

OBJS=$(COBJS)

OBJ_LIST=../obj.signal

all: $(OBJ_LIST)

$(OBJ_LIST): $(OBJS)
	echo $(patsubst %, signal/%, $(OBJS)) > $(OBJ_LIST)

$(COBJS): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	$(STRIPTOOL) -x -R .note -R .comment $*.o

clean:
	$(RM) *.[oa] *~ core

