# Makefile for uClibc
#
# Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
# ported to FR-V by Alexandre Oliva <aoliva@redhat.com>
#
# Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.

TOPDIR=../../../../
include $(TOPDIR)Rules.mak

CRT_SRC := crt0.S
CRT_OBJ := crt0.o crt1.o
SCRT_OBJ := $(patsubst %,S%, $(CRT_OBJ))

CTOR_TARGETS := crti.o crtn.o

SSRC := __longjmp.S setjmp.S clone.S vfork.S
SOBJ := $(patsubst %.S,%.o, $(SSRC))

CSRC = mmap.c sysdep.c syscall.c brk.c sbrk.c __init_brk.c dl-iterate-phdr.c
CSRC += xstatconv.c stat.c stat64.c fstat.c fstat64.c lstat.c lstat64.c
COBJ := $(patsubst %.c,%.o, $(CSRC))

OBJS := $(SOBJ) $(COBJ)

OBJ_LIST := ../../../obj.sysdeps.$(TARGET_ARCH)

all: $(OBJ_LIST)

$(OBJ_LIST): $(OBJS) $(CRT_OBJ) $(SCRT_OBJ) $(CTOR_TARGETS)
	$(STRIPTOOL) -x -R .note -R .comment $^
	$(INSTALL) -d $(TOPDIR)lib/
	cp $(CRT_OBJ) $(SCRT_OBJ) $(CTOR_TARGETS) $(TOPDIR)lib/
	echo $(patsubst %, sysdeps/linux/$(TARGET_ARCH)/%, $(OBJS)) > $@

$(CRT_OBJ): $(CRT_SRC) crtreloc.o
	$(CC) $(ASFLAGS) -DL_$* -r -nostdlib $^ -o $*.o

crtreloc.o: crtreloc.c
	$(CC) $(CFLAGS) -c $< -o $@

$(SCRT_OBJ): $(CRT_SRC) Scrtreloc.o
	$(CC) $(ASFLAGS) $(PIEFLAG) -DL_$* -r -nostdlib $^ -o $*.o

Scrtreloc.o: crtreloc.c
	$(CC) $(CFLAGS) $(PIEFLAG) -c $< -o $@

$(CTOR_TARGETS): %.o : %.S
	$(CC) $(ASFLAGS) $(SSP_DISABLE_FLAGS) -c $< -o $@

$(SOBJ): %.o : %.S
	$(CC) $(ASFLAGS) -c $< -o $@

$(COBJ): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) *.o *~ core
