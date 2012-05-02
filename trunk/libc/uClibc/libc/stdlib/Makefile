# Makefile for uClibc
#
# Copyright (C) 2000 by Lineo, inc.
# Copyright (C) 2000-2005 Erik Andersen <andersen@uclibc.org>
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
#
# Derived in part from the Linux-8086 C library, the GNU C Library, and several
# other sundry sources.  Files within this library are copyright by their
# respective copyright holders.

TOPDIR=../../
include $(TOPDIR)Rules.mak

DIRS:=
ifeq ($(MALLOC),y)
    DIRS+=malloc
endif
ifeq ($(MALLOC_SIMPLE),y)
    DIRS+=malloc-simple
endif
ifeq ($(MALLOC_STANDARD),y)
    DIRS+=malloc-standard
endif


MSRC = stdlib.c
MOBJ = abs.o labs.o atoi.o atol.o strtol.o strtoul.o _stdlib_strto_l.o \
       qsort.o bsearch.o \
       llabs.o atoll.o strtoll.o strtoull.o _stdlib_strto_ll.o
# (aliases) strtoq.o strtouq.o
ifeq ($(UCLIBC_HAS_XLOCALE),y)

MOBJx =
MOBJx += strtol_l.o strtoul_l.o _stdlib_strto_l_l.o \
	 strtoll_l.o strtoull_l.o _stdlib_strto_ll_l.o
endif

MSRC1 = strtod.c
MOBJ1 = 
MOBJ1x =

ifeq ($(UCLIBC_HAS_FLOATS),y)
	MOBJ += atof.o
	MOBJ1 += strtod.o strtof.o strtold.o __strtofpmax.o __fp_range_check.o
ifeq ($(UCLIBC_HAS_XLOCALE),y)
	MOBJ1x += strtod_l.o strtof_l.o strtold_l.o __strtofpmax_l.o
endif
ifeq ($(UCLIBC_HAS_WCHAR),y)
	MOBJ1 += wcstod.o wcstof.o wcstold.o __wcstofpmax.o
ifeq ($(UCLIBC_HAS_XLOCALE),y)
	MOBJ1x += wcstod_l.o wcstof_l.o wcstold_l.o __wcstofpmax_l.o
endif
endif
endif

ifeq ($(UCLIBC_HAS_WCHAR),y)
	MOBJ += mblen.o mbtowc.o wctomb.o mbstowcs.o wcstombs.o \
		_stdlib_mb_cur_max.o _stdlib_wcsto_l.o _stdlib_wcsto_ll.o \
		wcstol.o wcstoul.o wcstoll.o wcstoull.o
ifeq ($(UCLIBC_HAS_XLOCALE),y)
	MOBJx += _stdlib_wcsto_l_l.o _stdlib_wcsto_ll_l.o \
		 wcstol_l.o wcstoul_l.o wcstoll_l.o wcstoull_l.o
endif
endif
# (aliases) wcstoq.o wcstouq.o
# wcstod wcstof wcstold

MSRC2 = atexit.c
MOBJ2 = atexit.o on_exit.o __exit_handler.o exit.o

CSRC =	abort.c getenv.c mkdtemp.c mktemp.c realpath.c mkstemp.c mkstemp64.c \
	rand.c random.c random_r.c setenv.c system.c div.c ldiv.c lldiv.c \
	getpt.c ptsname.c grantpt.c unlockpt.c gcvt.c drand48-iter.c jrand48.c \
	jrand48_r.c lrand48.c lrand48_r.c mrand48.c mrand48_r.c nrand48.c \
	nrand48_r.c rand_r.c srand48.c srand48_r.c seed48.c seed48_r.c \
	valloc.c posix_memalign.c
ifeq ($(UCLIBC_HAS_FLOATS),y)
	CSRC += drand48.c drand48_r.c erand48.c erand48_r.c
endif
COBJS=$(patsubst %.c,%.o, $(CSRC))

OBJS=$(MOBJ) $(MOBJx) $(MOBJ1) $(MOBJ1x) $(MOBJ2) $(COBJS)

OBJ_LIST=../obj.stdlib

all: $(OBJ_LIST) subdirs

$(OBJ_LIST): $(OBJS)
	echo $(patsubst %, stdlib/%, $(OBJS)) > $(OBJ_LIST)

$(MOBJ): $(MSRC)
	$(CC) $(CFLAGS) -DL_$* $< -c -o $*.o
	$(STRIPTOOL) -x -R .note -R .comment $*.o

$(MOBJx): $(MSRC)
	$(CC) $(CFLAGS) -DL_$* -D__UCLIBC_DO_XLOCALE $< -c -o $*.o
	$(STRIPTOOL) -x -R .note -R .comment $*.o

$(MOBJ1): $(MSRC1)
	$(CC) $(CFLAGS) -DL_$* $< -c -o $*.o
	$(STRIPTOOL) -x -R .note -R .comment $*.o

$(MOBJ1x): $(MSRC1)
	$(CC) $(CFLAGS) -DL_$* -D__UCLIBC_DO_XLOCALE $< -c -o $*.o
	$(STRIPTOOL) -x -R .note -R .comment $*.o

$(MOBJ2): $(MSRC2)
	$(CC) $(CFLAGS) -DL_$* $< -c -o $*.o
	$(STRIPTOOL) -x -R .note -R .comment $*.o

$(COBJS): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	$(STRIPTOOL) -x -R .note -R .comment $*.o

$(OBJ): Makefile

subdirs: $(patsubst %, _dir_%, $(DIRS))
subdirs_clean: $(patsubst %, _dirclean_%, $(ALL_SUBDIRS))

$(patsubst %, _dir_%, $(DIRS)) : dummy
	$(MAKE) -C $(patsubst _dir_%, %, $@)

$(patsubst %, _dirclean_%, $(ALL_SUBDIRS)) : dummy
	$(MAKE) -C $(patsubst _dirclean_%, %, $@) clean

clean: subdirs_clean
	$(RM) *.[oa] *~ core

.PHONY: dummy
