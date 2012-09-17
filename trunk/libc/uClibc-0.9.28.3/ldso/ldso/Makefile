# Makefile for uClibc
#
# Copyright (C) 2000-2004 Erik Andersen <andersen@uclibc.org>
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
LDSO_FULLNAME=ld-uClibc-$(MAJOR_VERSION).$(MINOR_VERSION).$(SUBLEVEL).so

# reset to initial (disabling predefined CFLAGS)
ASFLAGS=
ifeq ($(UCLIBC_BUILD_NOEXECSTACK),y)
ASFLAGS+=$(call check_as,--noexecstack)
endif

XXFLAGS=$(XWARNINGS) $(SSP_DISABLE_FLAGS)
ifeq ($(DL_FINI_CRT_COMPAT),y)
XXFLAGS+=-D_DL_FINI_CRT_COMPAT
endif

ifeq ($(DODEBUG),y)
# Not really much point in including debugging info, since gdb
# can't really debug ldso, since gdb requires help from ldso to
# debug things....
XXFLAGS+=-Os -g
else
XXFLAGS+=$(OPTIMIZATION)
endif
XXFLAGS+= -DUCLIBC_LDSO=\"$(UCLIBC_LDSO)\" $(XARCH_CFLAGS) $(CPU_CFLAGS) $(PICFLAG) \
	-DUCLIBC_RUNTIME_PREFIX=\"$(RUNTIME_PREFIX)\" \
	-fno-builtin -nostdinc -D_LIBC \
	-DLDSO_ELFINTERP=\"$(TARGET_ARCH)/elfinterp.c\" \
	-I$(TOPDIR)ldso/ldso/$(TARGET_ARCH) -I$(TOPDIR)ldso/include -I$(TOPDIR)ldso/ldso -I$(TOPDIR)include
XXFLAGS += -D_GNU_SOURCE

# BEWARE!!! At least mips* will die if -O0 is used!!!
XXFLAGS:=$(XXFLAGS:-O0=-O1)
XXFLAGS+=-isystem $(shell $(CC) -print-file-name=include)
LDFLAGS=$(CPU_LDFLAGS-y) -e _start -z now -Bsymbolic -shared --warn-common --export-dynamic --sort-common \
	-z combreloc --discard-locals --discard-all --no-undefined
ifeq ($(UCLIBC_BUILD_RELRO),y)
	LDFLAGS+=-z relro
endif

CSRC= ldso.c
COBJS=$(patsubst %.c,%.o, $(CSRC))
ASRC=$(shell ls $(TARGET_ARCH)/*.S)
AOBJS=$(patsubst %.S,%.o, $(ASRC))
OBJS=$(AOBJS) $(COBJS)

ifneq ($(strip $(SUPPORT_LD_DEBUG)),y)
LDFLAGS+=-s
endif

ifeq ($(strip $(SUPPORT_LD_DEBUG)),y)
XXFLAGS+=-D__SUPPORT_LD_DEBUG__
endif

ifeq ($(strip $(SUPPORT_LD_DEBUG_EARLY)),y)
XXFLAGS+=-D__SUPPORT_LD_DEBUG_EARLY__
endif

#This stuff will not work with -fomit-frame-pointer
XXFLAGS := $(XXFLAGS:-fomit-frame-pointer=)

all: $(LDSO_FULLNAME)

$(LDSO_FULLNAME): $(OBJS) $(DLINK_OBJS)
	$(LD) $(LDFLAGS) -soname=$(UCLIBC_LDSO) \
		-o $(LDSO_FULLNAME) $(OBJS) $(LIBGCC)
	$(INSTALL) -d $(TOPDIR)lib
	$(INSTALL) -m 755 $(LDSO_FULLNAME) $(TOPDIR)lib
	$(LN) -sf $(LDSO_FULLNAME) $(TOPDIR)lib/$(UCLIBC_LDSO)

$(COBJS): %.o : %.c
	$(CC) $(XXFLAGS) -c $< -o $@
	$(STRIPTOOL) -x -R .note -R .comment $*.o

$(AOBJS): %.o : %.S
	$(CC) $(XXFLAGS) $(ASFLAGS) -c $< -o $@
	$(STRIPTOOL) -x -R .note -R .comment $*.o

ldso.o: $(CSRC) dl-elf.c dl-hash.c dl-startup.c dl-debug.c \
	$(TARGET_ARCH)/*.h $(TARGET_ARCH)/*.c $(TOPDIR)ldso/include/*.h

clean:
	$(RM) $(UCLIBC_LDSO)* $(OBJS) $(LDSO_FULLNAME)* core *.o *.a *.s *.i ldso.h *~
