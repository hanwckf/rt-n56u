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
# Makefile for uClibc

TOPDIR=../../
include $(TOPDIR)Rules.mak

#Adjust the soname version to avoid namespace collisions with glibc's libpthread
LIBTHREAD_DB:=../libthread_db.a
ifeq ($(strip $(TARGET_ARCH)),sparc)
SYSDEPS_DIR:=$(TARGET_ARCH)/sparc32
else
SYSDEPS_DIR:=$(TARGET_ARCH)
endif

# set up system dependencies include dirs (NOTE: order matters!)
PTDIR = $(TOPDIR)libpthread/linuxthreads/
SYSDEPINC = -I$(PTDIR)sysdeps/pthread \
            -I$(PTDIR)sysdeps/$(TARGET_ARCH) \
            -I$(TOPDIR)libc/sysdeps/linux/$(TARGET_ARCH)
#CFLAGS += $(SSP_ALL_CFLAGS)
CFLAGS += $(SYSDEPINC) -DLIBPTHREAD_SO="\"libpthread.so.$(MAJOR_VERSION)\""

CSRC=	td_init.c td_log.c td_ta_delete.c td_ta_get_nthreads.c		\
	td_ta_get_ph.c td_ta_map_id2thr.c td_ta_map_lwp2thr.c		\
	td_ta_new.c td_ta_thr_iter.c td_ta_tsd_iter.c			\
	td_thr_get_info.c td_thr_getfpregs.c td_thr_getgregs.c		\
	td_thr_getxregs.c td_thr_getxregsize.c td_thr_setfpregs.c	\
	td_thr_setgregs.c td_thr_setprio.c td_thr_setsigpending.c	\
	td_thr_setxregs.c td_thr_sigsetmask.c td_thr_tsd.c		\
	td_thr_validate.c td_thr_dbsuspend.c td_thr_dbresume.c		\
	td_ta_setconcurrency.c td_ta_enable_stats.c			\
	td_ta_reset_stats.c td_ta_get_stats.c td_ta_event_addr.c	\
	td_thr_event_enable.c td_thr_set_event.c			\
	td_thr_clear_event.c td_thr_event_getmsg.c			\
	td_ta_set_event.c td_ta_event_getmsg.c				\
	td_ta_clear_event.c td_symbol_list.c td_thr_tls_get_addr.c

COBJS=$(patsubst %.c,%.o, $(CSRC))
OBJS=$(COBJS)

all: $(LIBTHREAD_DB)

$(LIBTHREAD_DB) ar-target: $(OBJS)
	$(AR) $(ARFLAGS) $(LIBTHREAD_DB) $(OBJS)

$(COBJS): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	$(STRIPTOOL) -x -R .note -R .comment $*.o

clean:
	$(RM) *.[oa] *~ core
