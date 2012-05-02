# Makefile for uClibc
#
# Copyright (C) 2000 by Lineo, inc.
# Copyright (C) 2000-2002 Erik Andersen <andersen@uclibc.org>
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

XXFLAGS=$(XWARNINGS) $(SSP_CFLAGS) $(SSP_ALL_CFLAGS)
ifeq ($(DODEBUG),y)
XXFLAGS+=-O0 -g3
else
XXFLAGS+=$(OPTIMIZATION)
endif
XXFLAGS+= $(XARCH_CFLAGS) $(CPU_CFLAGS) \
	-DUCLIBC_RUNTIME_PREFIX=\"$(RUNTIME_PREFIX)\" \
	-fno-builtin -nostdinc -D_LIBC \
	-DLDSO_ELFINTERP=\"$(TARGET_ARCH)/elfinterp.c\" \
	-I$(TOPDIR)ldso/ldso/$(TARGET_ARCH) -I$(TOPDIR)ldso/include -I$(TOPDIR)ldso/ldso -I$(TOPDIR)include

XXFLAGS+=-isystem $(shell $(CC) -print-file-name=include)
XXFLAGS_NOPIC:=$(XXFLAGS)
ifeq ($(DOPIC),y)
    XXFLAGS += $(PICFLAG) -DSHARED
endif
ifeq ($(strip $(SUPPORT_LD_DEBUG)),y)
XXFLAGS+=-D__SUPPORT_LD_DEBUG__
endif

LIBDL=libdl.a
LIBDL_PIC=libdl_pic.a
LIBDL_SHARED=libdl.so
LIBDL_SHARED_FULLNAME=libdl-$(MAJOR_VERSION).$(MINOR_VERSION).$(SUBLEVEL).so

CSRC=libdl.c
OBJS=libdl.o
PIC_OBJS=libdl_pic.o

all: $(LIBDL) $(LIBDL_PIC)

ar-target: $(LIBDL) $(LIBDL_PIC)

$(LIBDL): $(OBJS)
	$(AR) $(ARFLAGS) $(LIBDL) ../ldso/$(TARGET_ARCH)/resolve.o $(OBJS)
	$(INSTALL) -d $(TOPDIR)lib
	$(RM) $(TOPDIR)lib/$(LIBDL)
	$(INSTALL) -m 644 $(LIBDL) $(TOPDIR)lib

$(LIBDL_PIC): $(PIC_OBJS)
	$(AR) $(ARFLAGS) $(LIBDL_PIC) $(PIC_OBJS)

libdl.o: libdl.c $(TOPDIR)ldso/include/*.h
	$(CC) $(XXFLAGS_NOPIC) -c libdl.c -o libdl.o
	$(STRIPTOOL) -x -R .note -R .comment $*.o

libdl_pic.o: libdl.c $(TOPDIR)ldso/include/*.h
	$(CC) $(XXFLAGS) -c libdl.c -o libdl_pic.o
	$(STRIPTOOL) -x -R .note -R .comment $*.o

shared: $(LIBDL_PIC)
	$(LD) $(LDFLAGS) -soname=$(LIBDL_SHARED).$(MAJOR_VERSION) \
		-o $(LIBDL_SHARED_FULLNAME) -fini dl_cleanup --whole-archive $(LIBDL_PIC) \
		--no-whole-archive $(TOPDIR)/libc/misc/internals/interp.o \
		-L$(TOPDIR)/lib -lc $(LDADD_LIBFLOAT) $(TOPDIR)lib/$(UCLIBC_LDSO) $(LIBGCC);
	$(INSTALL) -d $(TOPDIR)lib
	$(RM) $(TOPDIR)lib/$(LIBDL_SHARED_FULLNAME) $(TOPDIR)lib/$(LIBDL_SHARED).$(MAJOR_VERSION)
	$(INSTALL) -m 644 $(LIBDL_SHARED_FULLNAME) $(TOPDIR)lib
	$(LN) -sf $(LIBDL_SHARED_FULLNAME) $(TOPDIR)lib/$(LIBDL_SHARED)
	$(LN) -sf $(LIBDL_SHARED_FULLNAME) $(TOPDIR)lib/$(LIBDL_SHARED).$(MAJOR_VERSION)

clean:
	$(RM) .depend $(LIBDL_SHARED)* $(LIBDL_SHARED_FULLNAME) core *.o *.a *.s *.i tmp_make foo *~
