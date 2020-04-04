/* Copyright (C) 2000, 2001, 2002, 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <pthreadP.h>
#include <sys/time.h>
#include <tls.h>

#define CPUCLOCK_PERTHREAD_MASK	4
#define CPUCLOCK_SCHED		2

int
pthread_getcpuclockid (
     pthread_t threadid,
     clockid_t *clockid)
{
  struct pthread *pd = (struct pthread *) threadid;

  /* Make sure the descriptor is valid.  */
  if (INVALID_TD_P (pd))
    /* Not a valid thread handle.  */
    return ESRCH;

#ifdef CLOCK_THREAD_CPUTIME_ID
  /* We need to store the thread ID in the CLOCKID variable together
     with a number identifying the clock.  We reserve the low 3 bits
     for the clock ID and the rest for the thread ID.  This is
     problematic if the thread ID is too large.  But 29 bits should be
     fine.

     If some day more clock IDs are needed the ID part can be
     enlarged.  The IDs are entirely internal.  */
  if (pd->tid >= 1 << (8 * sizeof (*clockid) - CLOCK_IDFIELD_SIZE))
    return ERANGE;

  /* Store the number.  */
  *clockid = ((~(clockid_t) (pd->tid)) << CLOCK_IDFIELD_SIZE)
    | CPUCLOCK_SCHED | CPUCLOCK_PERTHREAD_MASK;

  return 0;
#else
  /* We don't have a timer for that.  */
  return ENOENT;
#endif
}
