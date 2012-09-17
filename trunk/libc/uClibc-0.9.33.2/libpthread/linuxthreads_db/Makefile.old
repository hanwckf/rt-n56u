# Copyright (C) 1999, 2000, 2001, 2002, 2003 Free Software Foundation, Inc.
# This file is part of the GNU C Library.

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.

# You should have received a copy of the GNU Lesser General Public
# License along with the GNU C Library; if not, write to the Free
# Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
# 02111-1307 USA.

# Makefile for linuxthreads debug library subdirectory of GNU C Library.

subdir		:= linuxthreads_db

linuxthreads_db-version = 1.0

extra-libs = libthread_db
extra-libs-others := $(extra-libs)

headers		= thread_db.h
libthread_db-routines = td_init td_log td_ta_delete td_ta_get_nthreads      \
			td_ta_get_ph td_ta_map_id2thr td_ta_map_lwp2thr     \
			td_ta_new td_ta_thr_iter td_ta_tsd_iter		    \
			td_thr_get_info td_thr_getfpregs td_thr_getgregs    \
			td_thr_getxregs td_thr_getxregsize td_thr_setfpregs \
			td_thr_setgregs td_thr_setprio td_thr_setsigpending \
			td_thr_setxregs td_thr_sigsetmask td_thr_tsd	    \
			td_thr_validate td_thr_dbsuspend td_thr_dbresume    \
			td_ta_setconcurrency td_ta_enable_stats		    \
			td_ta_reset_stats td_ta_get_stats td_ta_event_addr  \
			td_thr_event_enable td_thr_set_event 		    \
			td_thr_clear_event td_thr_event_getmsg		    \
			td_ta_set_event td_ta_event_getmsg		    \
			td_ta_clear_event td_symbol_list \
			td_thr_tlsbase td_thr_tls_get_addr

libthread_db-inhibit-o = $(filter-out .os,$(object-suffixes))

# The ps_* callback functions are not defined.
libthread_db.so-no-z-defs = yes

distribute = thread_dbP.h shlib-versions proc_service.h
include ../Rules

# Depend on libc.so so a DT_NEEDED is generated in the shared objects.
# This ensures they will load libc.so for needed symbols if loaded by
# a statically-linked program that hasn't already loaded it.
$(objpfx)libthread_db.so: $(common-objpfx)libc.so \
			  $(common-objpfx)libc_nonshared.a
