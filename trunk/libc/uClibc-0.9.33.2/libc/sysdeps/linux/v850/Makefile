# Makefile for uClibc
#
# Copyright (C) 2001,2002  NEC Corporation
# Copyright (C) 2001,2002  Miles Bader <miles@gnu.org>
#
# Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.

TOPDIR=../../../../
include $(TOPDIR)Rules.mak

CFLAGS += -I..
ASFLAGS += -I.. -D__ASSEMBLER -DASM_GLOBAL_DIRECTIVE=.globl

TARGET_MACHINE_TYPE=$(shell $(CC) -dumpmachine)

CRT_SRC := crt0.S
CRT_OBJ := crt0.o crt1.o
CTOR_TARGETS := $(TOPDIR)lib/crti.o $(TOPDIR)lib/crtn.o

SSRC := setjmp.S __longjmp.S vfork.S
SOBJ := $(patsubst %.S,%.o, $(SSRC))

CSRC := mmap.c syscall.c clone.c
COBJ := $(patsubst %.c,%.o, $(CSRC))

OBJS := $(SOBJ) $(COBJ)

OBJ_LIST := ../../../obj.sysdeps.$(TARGET_ARCH)

all: $(OBJ_LIST) $(CTOR_TARGETS)

$(OBJ_LIST): $(OBJS) $(CRT_OBJ)
	$(STRIPTOOL) -x -R .note -R .comment $^
	$(INSTALL) -d $(TOPDIR)lib/
	cp $(CRT_OBJ) $(TOPDIR)lib/
	echo $(patsubst %, sysdeps/linux/$(TARGET_ARCH)/%, $(OBJS)) > $@

$(CRT_OBJ): $(CRT_SRC)
	$(CC) $(ASFLAGS) -DL_$* $< -c -o $*.o

$(SOBJ): %.o : %.S
	$(CC) $(ASFLAGS) -c $< -o $@

$(COBJ): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

ifeq ($(UCLIBC_CTOR_DTOR),y)
$(TOPDIR)lib/crti.o: crti.S
	$(INSTALL) -d $(TOPDIR)lib/
	$(CC) $(ASFLAGS) $(SSP_DISABLE_FLAGS) -c $< -o $@

$(TOPDIR)lib/crtn.o: crtn.S
	$(INSTALL) -d $(TOPDIR)lib/
	$(CC) $(ASFLAGS) $(SSP_DISABLE_FLAGS) -c $< -o $@
else
$(CTOR_TARGETS):
	$(INSTALL) -d $(TOPDIR)lib/
	$(AR) $(ARFLAGS) $@
endif

headers:

clean:
	$(RM) *.o *~ core
