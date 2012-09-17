/* Copyright (C) 1991, 92, 1994-1998, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <errno.h>
#include <signal.h>
#include <stddef.h>		/* For NULL.  */

/* Set the mask of blocked signals to MASK,
   wait for a signal to arrive, and then restore the mask.  */
int __sigpause (int sig_or_mask, int is_sig)
{
  sigset_t set;
  int sig;

  if (is_sig != 0)
    {
      /* The modern X/Open implementation is requested.  */
      if (sigprocmask (0, NULL, &set) < 0
	  /* Yes, we call `sigdelset' and not `__sigdelset'.  */
	  || sigdelset (&set, sig_or_mask) < 0)
	return -1;
    }
  else
    {
      if (sigemptyset (&set) < 0)
	return -1;

      if (sizeof (sig_or_mask) == sizeof (set))
	*(int *) &set = sig_or_mask;
      else if (sizeof (unsigned long int) == sizeof (set))
	*(unsigned long int *) &set = (unsigned int) sig_or_mask;
      else
	for (sig = 1; sig < NSIG; ++sig)
	  if ((sig_or_mask & sigmask (sig)) && __sigaddset (&set, sig) < 0)
	    return -1;
    }

  return sigsuspend (&set);
}


#undef sigpause

/* We have to provide a default version of this function since the
   standards demand it.  The version which is a bit more reasonable is
   the BSD version.  So make this the default.  */
int sigpause (int mask)
{
  return __sigpause (mask, 0);
}
