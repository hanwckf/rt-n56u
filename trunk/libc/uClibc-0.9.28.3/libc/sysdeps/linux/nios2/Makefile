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

TOPDIR=../../../../
include $(TOPDIR)Rules.mak

CRT0_SRC = crt0.S
CRT0_OBJ = crt0.o crt1.o
CTOR_TARGETS=$(TOPDIR)lib/crti.o $(TOPDIR)lib/crtn.o

SSRC= __longjmp.S bsd-_setjmp.S bsd-setjmp.S setjmp.S vfork.S clone.S
SOBJS=$(patsubst %.S,%.o, $(SSRC))

CSRC= brk.c syscall.c
COBJS=$(patsubst %.c,%.o, $(CSRC))

OBJS=$(SOBJS) $(MOBJ) $(COBJS)

OBJ_LIST=../../../obj.sysdeps.$(TARGET_ARCH)

all: $(OBJ_LIST)

$(OBJ_LIST): $(OBJS) $(CRT0_OBJ) $(CTOR_TARGETS)
	echo $(patsubst %, sysdeps/linux/$(TARGET_ARCH)/%, $(OBJS)) > $(OBJ_LIST)
	$(INSTALL) -d $(TOPDIR)lib/
	cp $(CRT0_OBJ) $(TOPDIR)lib/

$(CRT0_OBJ): $(CRT0_SRC)
	$(CC) $(ASFLAGS) -DL_$* $< -c -o $*.o
	$(STRIPTOOL) -x -R .note -R .comment $*.o

$(SOBJS): %.o : %.S
	$(CC) $(ASFLAGS) -c $< -o $@
	$(STRIPTOOL) -x -R .note -R .comment $*.o

$(COBJS): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	$(STRIPTOOL) -x -R .note -R .comment $*.o

ifeq ($(strip $(UCLIBC_CTOR_DTOR)),y)
crti.o: crti.S
	$(CC) $(ASFLAGS) -c crti.S -o crti.o

$(TOPDIR)lib/crti.o: crti.o
	$(INSTALL) -d $(TOPDIR)lib/
	cp crti.o $(TOPDIR)lib/

crtn.o: crtn.S
	$(CC) $(ASFLAGS) -c crtn.S -o crtn.o

$(TOPDIR)lib/crtn.o: crtn.o
	$(INSTALL) -d $(TOPDIR)lib/
	cp crtn.o $(TOPDIR)lib/
else
$(TOPDIR)lib/crti.o:
	$(INSTALL) -d $(TOPDIR)lib/
	$(AR) $(ARFLAGS) $(TOPDIR)lib/crti.o
$(TOPDIR)lib/crtn.o:
	$(INSTALL) -d $(TOPDIR)lib/
	$(AR) $(ARFLAGS) $(TOPDIR)lib/crtn.o
endif

headers:
	$(LN) -fs ../libc/sysdeps/linux/nios2/fpu_control.h $(TOPDIR)/include/

clean:
	$(RM) *.[oa] *~ core
	$(RM) bits/sysnum.h
	$(RM) $(TOPDIR)/include/fpu_control.h
