/* Copyright (C) 2003, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2003.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <unistd.h>
#include <tls.h>
#include <sysdep.h>


#ifndef NOT_IN_libc
static inline __attribute__((always_inline)) pid_t really_getpid (pid_t oldval);

static inline __attribute__((always_inline)) pid_t
really_getpid (pid_t oldval)
{
  if (__builtin_expect (oldval == 0, 1))
    {
      pid_t selftid = THREAD_GETMEM (THREAD_SELF, tid);
      if (__builtin_expect (selftid != 0, 1))
	return selftid;
    }

  INTERNAL_SYSCALL_DECL (err);
  pid_t result = INTERNAL_SYSCALL (getpid, err, 0);

  /* We do not set the PID field in the TID here since we might be
     called from a signal handler while the thread executes fork.  */
  if (oldval == 0)
    THREAD_SETMEM (THREAD_SELF, tid, result);
  return result;
}
#endif

extern __typeof(getpid) __getpid;
pid_t
__getpid (void)
{
#ifdef NOT_IN_libc
  INTERNAL_SYSCALL_DECL (err);
  pid_t result = INTERNAL_SYSCALL (getpid, err, 0);
#else
  pid_t result = THREAD_GETMEM (THREAD_SELF, pid);
  if (__builtin_expect (result <= 0, 0))
    result = really_getpid (result);
#endif
  return result;
}
libc_hidden_proto(getpid)
weak_alias(__getpid, getpid)
libc_hidden_weak(getpid)
