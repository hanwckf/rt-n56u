/* Copyright (C) 1991,92,94-98,2000,2002,2003,2004
   Free Software Foundation, Inc.
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

#define __UCLIBC_HIDE_DEPRECATED__
/* psm: need the BSD version of sigpause here */
#include <errno.h>
#define __FAVOR_BSD
#include <signal.h>
#include <stddef.h>		/* For NULL.  */
#ifdef __UCLIBC_HAS_THREADS_NATIVE__
#include <sysdep-cancel.h>
#endif

#include "sigset-cvt-mask.h"

/* Set the mask of blocked signals to MASK,
   wait for a signal to arrive, and then restore the mask.  */
int __sigpause (int sig_or_mask, int is_sig)
{
  sigset_t set;

  if (is_sig)
    {
      /* The modern X/Open implementation is requested.  */
      sigprocmask (SIG_BLOCK, NULL, &set);
      /* Bound-check sig_or_mask, remove it from the set.  */
      if (sigdelset (&set, sig_or_mask) < 0)
	return -1;
    }
  else
    sigset_set_old_mask (&set, sig_or_mask);

  /* Note the sigpause() is a cancellation point.  But since we call
     sigsuspend() which itself is a cancellation point we do not have
     to do anything here.  */
  return sigsuspend (&set);
}
libc_hidden_def(__sigpause)

#undef sigpause

/* We have to provide a default version of this function since the
   standards demand it.  The version which is a bit more reasonable is
   the BSD version.  So make this the default.  */
int sigpause (int mask)
{
#ifdef __UCLIBC_HAS_THREADS_NATIVE__
  if (SINGLE_THREAD_P)
    return __sigpause (mask, 0);

  int oldtype = LIBC_CANCEL_ASYNC ();

  int result = __sigpause (mask, 0);

  LIBC_CANCEL_RESET (oldtype);

  return result;
#else
  return __sigpause (mask, 0);
#endif
}
