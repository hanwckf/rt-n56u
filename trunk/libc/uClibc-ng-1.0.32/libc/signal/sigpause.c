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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <signal.h>
#define __need_NULL
#include <stddef.h>
#include <cancel.h>

#include "sigset-cvt-mask.h"

/* Set the mask of blocked signals to MASK,
   wait for a signal to arrive, and then restore the mask.  */
static int __sigpause (int sig_or_mask, int is_sig)
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
  /* uClibc note: not true on uClibc, we call the non-cancellable version */
  return __NC(sigsuspend)(&set);
}

int __bsd_sigpause(int mask);
int __bsd_sigpause(int mask)
{
	return __sigpause(mask, 0);
}

/* We have to provide a default version of this function since the
   standards demand it.  The version which is a bit more reasonable is
   the BSD version.  So make this the default.  */
static int __NC(sigpause)(int sig)
{
	return __sigpause(sig, 1);
}
CANCELLABLE_SYSCALL(int, sigpause, (int sig), (sig))
