# Makefile for uClibc's pthread library
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

TOPDIR=../
include $(TOPDIR)Rules.mak

#Adjust the soname version to avoid namespace collisions with glibc's libpthread
LIBPTHREAD=libpthread.a
LIBPTHREAD_SHARED=libpthread.so
LIBPTHREAD_SHARED_FULLNAME=libpthread-$(MAJOR_VERSION).$(MINOR_VERSION).$(SUBLEVEL).so

LIBTHREAD_DB=libthread_db.a
LIBTHREAD_DB_SHARED=libthread_db.so
LIBTHREAD_DB_SHARED_FULLNAME=libthread_db-$(MAJOR_VERSION).$(MINOR_VERSION).$(SUBLEVEL).so

DIRS=
ifeq ($(strip $(UCLIBC_HAS_THREADS)),y)
	DIRS+=linuxthreads
ifeq ($(strip $(PTHREADS_DEBUG_SUPPORT)),y)
	DIRS+=linuxthreads_db
endif
endif

ifeq ($(strip $(UCLIBC_CTOR_DTOR)),y)
	SHARED_START_FILES = $(TOPDIR)lib/crti.o $(LIBGCC_DIR)crtbeginS.o
	SHARED_END_FILES = $(LIBGCC_DIR)crtendS.o $(TOPDIR)lib/crtn.o
endif

ALL_SUBDIRS = linuxthreads linuxthreads_db

# Remove any -z defs since this lib will have undefined symbols
LIBTHREAD_DB_LDFLAGS := $(subst -z defs,,$(LDFLAGS))

all: $(LIBPTHREAD) $(LIBTHREAD_DB)

$(LIBPTHREAD): subdirs
ifeq ($(strip $(UCLIBC_HAS_THREADS)),y)
	$(INSTALL) -d $(TOPDIR)lib
	$(RM) $(TOPDIR)lib/$(LIBPTHREAD)
	$(INSTALL) -m 644 $(LIBPTHREAD) $(TOPDIR)lib
endif

$(LIBTHREAD_DB): subdirs
ifeq ($(strip $(PTHREADS_DEBUG_SUPPORT)),y)
	$(INSTALL) -d $(TOPDIR)lib
	$(RM) $(TOPDIR)lib/$(LIBTHREAD_DB)
	$(INSTALL) -m 644 $(LIBTHREAD_DB) $(TOPDIR)lib
endif


$(OBJS): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	$(STRIPTOOL) -x -R .note -R .comment $*.o

$(OBJ): Makefile

shared: all
ifeq ($(strip $(UCLIBC_HAS_THREADS)),y)
	$(LD) $(LDFLAGS_NOSTRIP) -soname=$(LIBPTHREAD_SHARED).$(MAJOR_VERSION) \
		-o $(LIBPTHREAD_SHARED_FULLNAME) $(SHARED_START_FILES) --whole-archive $(LIBPTHREAD) \
		--no-whole-archive $(TOPDIR)libc/misc/internals/interp.o \
		-L$(TOPDIR)lib -lc $(LDADD_LIBFLOAT) $(LIBGCC) \
		$(SHARED_END_FILES)
	$(INSTALL) -d $(TOPDIR)lib
	$(RM) $(TOPDIR)lib/$(LIBPTHREAD_SHARED_FULLNAME) \
		$(TOPDIR)lib/$(LIBPTHREAD_SHARED).$(MAJOR_VERSION)
	$(INSTALL) -m 644 $(LIBPTHREAD_SHARED_FULLNAME) $(TOPDIR)lib
	$(LN) -sf $(LIBPTHREAD_SHARED_FULLNAME) \
		$(TOPDIR)lib/$(LIBPTHREAD_SHARED)
	$(LN) -sf $(LIBPTHREAD_SHARED_FULLNAME) \
		$(TOPDIR)lib/$(LIBPTHREAD_SHARED).$(MAJOR_VERSION)
endif
ifeq ($(strip $(PTHREADS_DEBUG_SUPPORT)),y)
	$(LD) $(LIBTHREAD_DB_LDFLAGS) -soname=$(LIBTHREAD_DB_SHARED).1 \
		-o $(LIBTHREAD_DB_SHARED_FULLNAME) --whole-archive $(LIBTHREAD_DB) \
		--no-whole-archive $(TOPDIR)libc/misc/internals/interp.o \
		-L$(TOPDIR)lib -lc $(LDADD_LIBFLOAT) $(LIBGCC)
	$(INSTALL) -d $(TOPDIR)lib
	$(RM) $(TOPDIR)lib/$(LIBTHREAD_DB_SHARED_FULLNAME) \
		$(TOPDIR)lib/$(LIBTHREAD_DB_SHARED).1
	$(INSTALL) -m 644 $(LIBTHREAD_DB_SHARED_FULLNAME) $(TOPDIR)lib
	$(LN) -sf $(LIBTHREAD_DB_SHARED_FULLNAME) \
		$(TOPDIR)lib/$(LIBTHREAD_DB_SHARED)
	$(LN) -sf $(LIBTHREAD_DB_SHARED_FULLNAME) \
		$(TOPDIR)lib/$(LIBTHREAD_DB_SHARED).1
endif

tags:
	ctags -R

subdirs: $(patsubst %, _dir_%, $(DIRS))
subdirs_clean: $(patsubst %, _dirclean_%, $(ALL_SUBDIRS))

$(patsubst %, _dir_%, $(DIRS)) : dummy
	$(MAKE) -C $(patsubst _dir_%, %, $@)

$(patsubst %, _dirclean_%, $(ALL_SUBDIRS)) : dummy
	$(MAKE) -C $(patsubst _dirclean_%, %, $@) clean

clean: subdirs_clean
	$(RM) *.[oa] *~ core $(LIBPTHREAD) $(LIBPTHREAD_SHARED_FULLNAME) \
		$(LIBTHREAD_DB) $(LIBTHREAD_DB_SHARED_FULLNAME)

.PHONY: dummy
