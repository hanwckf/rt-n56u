/* Basic platform-independent macro definitions for mutexes,
   thread-specific data and parameters for malloc.
   Copyright (C) 2003, 2007, 2008 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _MALLOC_MACHINE_H
#define _MALLOC_MACHINE_H

#undef thread_atfork_static

#include <atomic.h>
#include <bits/libc-lock.h>

__libc_lock_define (typedef, mutex_t)

#define mutex_init(m)		__libc_lock_init (*(m))
#define mutex_lock(m)		__libc_lock_lock (*(m))
#define mutex_trylock(m)	__libc_lock_trylock (*(m))
#define mutex_unlock(m)		__libc_lock_unlock (*(m))

/* This is defined by newer gcc version unique for each module.  */
extern void *__dso_handle __attribute__ ((__weak__));

#include <fork.h>

#define ATFORK_MEM static struct fork_handler atfork_mem

#ifdef SHARED
# define thread_atfork(prepare, parent, child) \
  atfork_mem.prepare_handler = prepare;					      \
  atfork_mem.parent_handler = parent;					      \
  atfork_mem.child_handler = child;					      \
  atfork_mem.dso_handle = __dso_handle;					      \
  atfork_mem.refcntr = 1;						      \
  __linkin_atfork (&atfork_mem)
#else
# define thread_atfork(prepare, parent, child) \
  atfork_mem.prepare_handler = prepare;					      \
  atfork_mem.parent_handler = parent;					      \
  atfork_mem.child_handler = child;					      \
  atfork_mem.dso_handle = &__dso_handle == NULL ? NULL : __dso_handle;	      \
  atfork_mem.refcntr = 1;						      \
  __linkin_atfork (&atfork_mem)
#endif

/* thread specific data for glibc */

#include <bits/libc-tsd.h>

typedef int tsd_key_t[1];	/* no key data structure, libc magic does it */
__libc_tsd_define (static, void *, MALLOC)	/* declaration/common definition */
#define tsd_key_create(key, destr)	((void) (key))
#define tsd_setspecific(key, data)	__libc_tsd_set (void *, MALLOC, (data))
#define tsd_getspecific(key, vptr)	((vptr) = __libc_tsd_get (void *, MALLOC))

#include <sysdeps/generic/malloc-machine.h>

#endif /* !defined(_MALLOC_MACHINE_H) */
