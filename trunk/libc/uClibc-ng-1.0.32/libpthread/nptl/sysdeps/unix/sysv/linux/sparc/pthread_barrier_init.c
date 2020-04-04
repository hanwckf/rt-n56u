/* Copyright (C) 2002, 2006, 2007 Free Software Foundation, Inc.
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
#include "pthreadP.h"
#include <lowlevellock.h>

int
pthread_barrier_init (
     pthread_barrier_t *barrier,
     const pthread_barrierattr_t *attr,
     unsigned int count)
{
  union sparc_pthread_barrier *ibarrier;

  if (__builtin_expect (count == 0, 0))
    return EINVAL;

  struct pthread_barrierattr *iattr = (struct pthread_barrierattr *) attr;
  if (iattr != NULL)
    {
      if (iattr->pshared != PTHREAD_PROCESS_PRIVATE
	  && __builtin_expect (iattr->pshared != PTHREAD_PROCESS_SHARED, 0))
	/* Invalid attribute.  */
	return EINVAL;
    }

  ibarrier = (union sparc_pthread_barrier *) barrier;

  /* Initialize the individual fields.  */
  ibarrier->b.lock = LLL_LOCK_INITIALIZER;
  ibarrier->b.left = count;
  ibarrier->b.init_count = count;
  ibarrier->b.curr_event = 0;
  ibarrier->s.left_lock = 0;
  ibarrier->s.pshared = (iattr && iattr->pshared == PTHREAD_PROCESS_SHARED);

  return 0;
}
