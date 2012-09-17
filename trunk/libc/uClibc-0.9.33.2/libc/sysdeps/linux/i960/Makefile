# Makefile for uClibc
#
# Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
#
# Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.

TOPDIR=../../../../
include $(TOPDIR)Rules.mak

#FIXME -- this arch should include its own crti.S and crtn.S
UCLIBC_CTOR_DTOR=n

CRT_SRC := crt0.S
CRT_OBJ := crt0.o crt1.o
CTOR_TARGETS := $(TOPDIR)lib/crti.o $(TOPDIR)lib/crtn.o

SSRC := clone.S _mmap.S setjmp.S vfork.S
SOBJ := $(patsubst %.S,%.o, $(SSRC))

OBJS := $(SOBJ)

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
