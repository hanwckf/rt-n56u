/* Copyright (C) 2009 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2009.

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
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <pthreadP.h>
#include <tls.h>
#include <sysdep.h>
#include <bits/kernel-features.h>


int
pthread_sigqueue (
     pthread_t threadid,
     int signo,
     const union sigval value)
{
#ifdef __NR_rt_tgsigqueueinfo
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

  /* Set up the siginfo_t structure.  */
  siginfo_t info;
  memset (&info, '\0', sizeof (siginfo_t));
  info.si_signo = signo;
  info.si_code = SI_QUEUE;
  info.si_pid = THREAD_GETMEM (THREAD_SELF, pid);
  info.si_uid = getuid ();
  info.si_value = value;

  /* We have a special syscall to do the work.  */
  INTERNAL_SYSCALL_DECL (err);

  /* One comment: The PID field in the TCB can temporarily be changed
     (in fork).  But this must not affect this code here.  Since this
     function would have to be called while the thread is executing
     fork, it would have to happen in a signal handler.  But this is
     no allowed, pthread_sigqueue is not guaranteed to be async-safe.  */
  int val = INTERNAL_SYSCALL (rt_tgsigqueueinfo, err, 4,
			      THREAD_GETMEM (THREAD_SELF, pid),
			      tid, signo, &info);

  return (INTERNAL_SYSCALL_ERROR_P (val, err)
	  ? INTERNAL_SYSCALL_ERRNO (val, err) : 0);
#else
  return ENOSYS;
#endif
}
