# Makefile for uClibc's pthread library
#
# Copyright (C) 2002 Erik Andersen <andersen@uclibc.org>
# Copyright (C) 2003 Paul Mundt <lethal@linux-sh.org>
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
# Makefile for uClibc

TOPDIR=../../
include $(TOPDIR)Rules.mak

# As long as there is only one subdir, we don't 
# have to worry about race conditions with multiple 
# $(AR)'s in subdirs running on linuxthreads.a.
DIRS = sysdeps

#Adjust the soname version to avoid namespace collisions with glibc's libpthread
LIBPTHREAD:=../libpthread.a
ifeq ($(strip $(TARGET_ARCH)),sparc)
SYSDEPS_DIR:=$(TARGET_ARCH)/sparc32
else
SYSDEPS_DIR:=$(TARGET_ARCH)
endif
CFLAGS += $(SSP_ALL_CFLAGS)
#This stuff will not compile without at least -O1
CFLAGS :=$(CFLAGS:-O0=-O1)


# set up system dependencies include dirs (NOTE: order matters!)
PTDIR = $(TOPDIR)libpthread/linuxthreads/
SYSDEPINC = -I$(PTDIR)sysdeps/pthread \
            -I$(PTDIR)sysdeps/$(TARGET_ARCH) \
            -I$(TOPDIR)libc/sysdeps/linux/$(TARGET_ARCH)
CFLAGS += $(SYSDEPINC)

CSRC=attr.c cancel.c condvar.c errno.c events.c join.c lockfile.c manager.c \
	mutex.c oldsemaphore.c pt-machine.c ptfork.c pthread.c \
	ptlongjmp.c rwlock.c semaphore.c signals.c specific.c spinlock.c \
	wrapsyscall.c

ifeq ($(UCLIBC_HAS_XLOCALE),y)
	CSRC += locale.c
endif

COBJS=$(patsubst %.c,%.o, $(CSRC))
OBJS=$(COBJS)

# We need to make sure that we put all the top-level $(OBJS) into
# our archive before executing subdirs.  That way, when $(AR) is 
# run in the subdirs, it'll bump the generic top-level objects 
# out of our archive in favor of the machine-specific ones.
all: $(LIBPTHREAD) subdirs

$(LIBPTHREAD) ar-target: $(OBJS)
	$(AR) $(ARFLAGS) $(LIBPTHREAD) $(OBJS)

$(COBJS): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
ifeq ($(strip $(PTHREADS_DEBUG_SUPPORT)),y)
	$(STRIPTOOL) -X --strip-debug -R .note -R .comment $*.o
else
	$(STRIPTOOL) -x -R .note -R .comment $*.o
endif

clean: subdirs_clean
	$(RM) *.[oa] *~ core

subdirs: $(patsubst %, _dir_%, $(DIRS))
subdirs_clean: $(patsubst %, _dirclean_%, $(DIRS))

$(patsubst %, _dir_%, $(DIRS)): $(LIBPTHREAD)
	$(MAKE) -C $(patsubst _dir_%, %, $@)

$(patsubst %, _dirclean_%, $(DIRS)):
	$(MAKE) -C $(patsubst _dirclean_%, %, $@) clean

.PHONY: dummy
