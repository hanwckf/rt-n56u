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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <signal.h>
#include <pthreadP.h>
#include <sysdep.h>


int
pthread_sigmask (
     int how,
     const sigset_t *newmask,
     sigset_t *oldmask)
{
  sigset_t local_newmask;

  /* The only thing we have to make sure here is that SIGCANCEL and
     SIGSETXID is not blocked.  */
  if (newmask != NULL
      && (__builtin_expect (__sigismember (newmask, SIGCANCEL), 0)
	  || __builtin_expect (__sigismember (newmask, SIGSETXID), 0)))
    {
      local_newmask = *newmask;
      __sigdelset (&local_newmask, SIGCANCEL);
      __sigdelset (&local_newmask, SIGSETXID);
      newmask = &local_newmask;
    }

#ifdef INTERNAL_SYSCALL
  /* We know that realtime signals are available if NPTL is used.  */
  INTERNAL_SYSCALL_DECL (err);
  int result = INTERNAL_SYSCALL (rt_sigprocmask, err, 4, how, newmask,
				 oldmask, _NSIG / 8);

  return (INTERNAL_SYSCALL_ERROR_P (result, err)
	  ? INTERNAL_SYSCALL_ERRNO (result, err)
	  : 0);
#else
  return sigprocmask (how, newmask, oldmask) == -1 ? errno : 0;
#endif
}
