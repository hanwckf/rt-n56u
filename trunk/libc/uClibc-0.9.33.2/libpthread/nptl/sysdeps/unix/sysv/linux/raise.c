/* Copyright (C) 2002, 2003, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

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

#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <sysdep.h>
#include <pthreadP.h>
#include <bits/kernel-features.h>


int
raise (
     int sig)
{
  struct pthread *pd = THREAD_SELF;
#if (defined(__ASSUME_TGKILL) && __ASSUME_TGKILL) || defined __NR_tgkill
  pid_t pid = THREAD_GETMEM (pd, pid);
#endif
  pid_t selftid = THREAD_GETMEM (pd, tid);
  if (selftid == 0)
    {
      /* This system call is not supposed to fail.  */
#ifdef INTERNAL_SYSCALL
      INTERNAL_SYSCALL_DECL (err);
      selftid = INTERNAL_SYSCALL (gettid, err, 0);
#else
      selftid = INLINE_SYSCALL (gettid, 0);
#endif
      THREAD_SETMEM (pd, tid, selftid);

#if (defined(__ASSUME_TGKILL) && __ASSUME_TGKILL) || defined __NR_tgkill
      /* We do not set the PID field in the TID here since we might be
	 called from a signal handler while the thread executes fork.  */
      pid = selftid;
#endif
    }
#if (defined(__ASSUME_TGKILL) && __ASSUME_TGKILL) || defined __NR_tgkill
  else
    /* raise is an async-safe function.  It could be called while the
       fork/vfork function temporarily invalidated the PID field.  Adjust for
       that.  */
    if (__builtin_expect (pid <= 0, 0))
      pid = (pid & INT_MAX) == 0 ? selftid : -pid;
#endif

#if defined(__ASSUME_TGKILL) && __ASSUME_TGKILL
  return INLINE_SYSCALL (tgkill, 3, pid, selftid, sig);
#else
# ifdef __NR_tgkill
  int res = INLINE_SYSCALL (tgkill, 3, pid, selftid, sig);
  if (res != -1 || errno != ENOSYS)
    return res;
# endif
  return INLINE_SYSCALL (tkill, 2, selftid, sig);
#endif
}
libc_hidden_def (raise)
