/* Copyright (C) 1992, 1994, 1996, 2002 Free Software Foundation, Inc.
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

#include <stddef.h>
#include <signal.h>
#include <errno.h>


/* If INTERRUPT is nonzero, make signal SIG interrupt system calls
   (causing them to fail with EINTR); if INTERRUPT is zero, make system
   calls be restarted after signal SIG.  */
#ifdef SA_RESTART
extern sigset_t _sigintr attribute_hidden;	/* Defined in signal.c.  */
#endif

int siginterrupt (int sig, int interrupt)
{
#ifdef	SA_RESTART
  struct sigaction action;

  /* Fails if sig is bad.  */
  if (sigaction (sig, NULL, &action) < 0)
    return -1;

  if (interrupt)
    {
      __sigaddset (&_sigintr, sig);
      action.sa_flags &= ~SA_RESTART;
    }
  else
    {
      __sigdelset (&_sigintr, sig);
      action.sa_flags |= SA_RESTART;
    }

  return sigaction (sig, &action, NULL);
#else
  __set_errno (ENOSYS);
  return -1;
#endif
}
