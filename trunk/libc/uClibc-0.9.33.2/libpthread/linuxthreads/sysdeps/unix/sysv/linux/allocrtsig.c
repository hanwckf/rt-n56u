/* Handle real-time signal allocation.
   Copyright (C) 1997,98,99,2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

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

#include <signal.h>

/* Sanity check.  */
#if !defined __SIGRTMIN || (__SIGRTMAX - __SIGRTMIN) < 3
# error "This must not happen"
#endif

static int current_rtmin;
static int current_rtmax;

static int initialized;

#include <testrtsig.h>

static void
init (void)
{
  if (!kernel_has_rtsig ())
    {
      current_rtmin = -1;
      current_rtmax = -1;
    }
  else
    {
      current_rtmin = __SIGRTMIN + 3;
      current_rtmax = __SIGRTMAX;
    }
  initialized = 1;
}

/* Return number of available real-time signal with highest priority.  */
int
__libc_current_sigrtmin (void)
{
  if (!initialized)
    init ();
  return current_rtmin;
}
strong_alias (__libc_current_sigrtmin, __libc_current_sigrtmin_private)
libc_hidden_def (__libc_current_sigrtmin)

/* Return number of available real-time signal with lowest priority.  */
int
__libc_current_sigrtmax (void)
{
  if (!initialized)
    init ();
  return current_rtmax;
}
strong_alias (__libc_current_sigrtmax, __libc_current_sigrtmax_private)
libc_hidden_def (__libc_current_sigrtmax)

/* Allocate real-time signal with highest/lowest available
   priority.  Please note that we don't use a lock since we assume
   this function to be called at program start.  */
int
__libc_allocate_rtsig (int high)
{
  if (!initialized)
    init ();
  if (current_rtmin == -1 || current_rtmin > current_rtmax)
    /* We don't have anymore signal available.  */
    return -1;

  return high ? current_rtmin++ : current_rtmax--;
}
strong_alias (__libc_allocate_rtsig, __libc_allocate_rtsig_private)
