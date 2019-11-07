/* Copyright (C) 1991,96,97,2002,2003,2004 Free Software Foundation, Inc.
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
#ifdef __UCLIBC_HAS_THREADS_NATIVE__
# include <pthreadP.h>	/* SIGCANCEL */
#endif
#if 0
#define __need_NULL
#include <stddef.h>
#include <errno.h>
#endif

/* Set all signals in SET.  */
int
sigfillset (sigset_t *set)
{
#if 0 /* is it really required by standards?! */
  if (set == NULL)
    {
      __set_errno (EINVAL);
      return -1;
    }
#endif

  __sigfillset (set);

  /* If the implementation uses a cancellation signal don't set the bit.  */
#ifdef SIGCANCEL
  __sigdelset (set, SIGCANCEL);
#endif
  /* Likewise for the signal to implement setxid.  */
#ifdef SIGSETXID
  __sigdelset (set, SIGSETXID);
#endif

  return 0;
}
