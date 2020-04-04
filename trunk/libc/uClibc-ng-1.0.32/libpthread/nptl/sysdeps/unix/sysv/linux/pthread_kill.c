/* Copyright (C) 2002, 2003, 2004, 2006 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <pthreadP.h>
#include <tls.h>
#include <sysdep.h>
#include <bits/kernel-features.h>


int
__pthread_kill (
     pthread_t threadid,
     int signo)
{
  struct pthread *pd = (struct pthread *) threadid;

  /* Make sure the descriptor is valid.  */
  if (DEBUGGING_P && INVALID_TD_P (pd))
    /* Not a valid thread handle.  */
    return ESRCH;

  /* Force load of pd->tid into local variable or register.  Otherwise
     if a thread exits between ESRCH test and tgkill, we might return
     EINVAL, because pd->tid would be cleared by the kernel.  */
  pid_t tid = atomic_forced_read (pd->tid);
  if (__builtin_expect (tid <= 0, 0))
    /* Not a valid thread handle.  */
    return ESRCH;

  /* Disallow sending the signal we use for cancellation, timers, for
     for the setxid implementation.  */
  if (signo == SIGCANCEL || signo == SIGTIMER || signo == SIGSETXID)
    return EINVAL;

  /* We have a special syscall to do the work.  */
  INTERNAL_SYSCALL_DECL (err);

  /* One comment: The PID field in the TCB can temporarily be changed
     (in fork).  But this must not affect this code here.  Since this
     function would have to be called while the thread is executing
     fork, it would have to happen in a signal handler.  But this is
     no allowed, pthread_kill is not guaranteed to be async-safe.  */

  pid_t pid = getpid ();
  int val;
  val = INTERNAL_SYSCALL (tgkill, err, 3, pid, tid, signo);

  return (INTERNAL_SYSCALL_ERROR_P (val, err)
	  ? INTERNAL_SYSCALL_ERRNO (val, err) : 0);
}
strong_alias (__pthread_kill, pthread_kill)
