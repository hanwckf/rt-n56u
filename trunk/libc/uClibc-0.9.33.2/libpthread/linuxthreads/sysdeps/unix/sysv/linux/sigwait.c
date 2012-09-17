/* Copyright (C) 1997, 1998, 2000, 2002, 2003 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <errno.h>
#include <signal.h>
#define __need_NULL
#include <stddef.h>

#include <sysdep-cancel.h>
#include <sys/syscall.h>
#include <bits/libc-lock.h>

extern int __syscall_rt_sigtimedwait (const sigset_t *, siginfo_t *,
				      const struct timespec *, size_t);


/* Return any pending signal or wait for one for the given time.  */
static __inline__ int
do_sigwait (const sigset_t *set, int *sig)
{
  int ret;

  /* XXX The size argument hopefully will have to be changed to the
     real size of the user-level sigset_t.  */
#ifdef INTERNAL_SYSCALL
  INTERNAL_SYSCALL_DECL (err);
  ret = INTERNAL_SYSCALL (rt_sigtimedwait, err, 4, set,
			  NULL, NULL, _NSIG / 8);
  if (! INTERNAL_SYSCALL_ERROR_P (ret, err))
    {
      *sig = ret;
      ret = 0;
    }
  else
    ret = INTERNAL_SYSCALL_ERRNO (ret, err);
#else
  ret = INLINE_SYSCALL (rt_sigtimedwait, 4, set,
			NULL, NULL, _NSIG / 8);
  if (ret != -1)
    {
      *sig = ret;
      ret = 0;
    }
  else
    ret = errno;
#endif

  return ret;
}

#ifndef SHARED
weak_extern (__pthread_sigwait)
#endif

int
sigwait (const sigset_t *set, int *sig)
{
#ifndef NOT_IN_libc
  return __libc_maybe_call2 (pthread_sigwait, (set, sig),
			     do_sigwait (set, sig));
#else
  return do_sigwait (set, sig);
#endif
}
strong_alias(sigwait, __libc_sigwait)

/* Cancellation is handled in __pthread_sigwait.  */
LIBC_CANCEL_HANDLED ();
