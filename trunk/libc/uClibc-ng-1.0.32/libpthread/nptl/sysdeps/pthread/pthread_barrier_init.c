/* Copyright (C) 2002, 2007 Free Software Foundation, Inc.
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
#include <bits/kernel-features.h>


static const struct pthread_barrierattr default_attr =
  {
    .pshared = PTHREAD_PROCESS_PRIVATE
  };


int
pthread_barrier_init (
     pthread_barrier_t *barrier,
     const pthread_barrierattr_t *attr,
     unsigned int count)
{
  struct pthread_barrier *ibarrier;

  if (__builtin_expect (count == 0, 0))
    return EINVAL;

  const struct pthread_barrierattr *iattr
    = (attr != NULL
       ? iattr = (struct pthread_barrierattr *) attr
       : &default_attr);

  if (iattr->pshared != PTHREAD_PROCESS_PRIVATE
      && __builtin_expect (iattr->pshared != PTHREAD_PROCESS_SHARED, 0))
    /* Invalid attribute.  */
    return EINVAL;

  ibarrier = (struct pthread_barrier *) barrier;

  /* Initialize the individual fields.  */
  ibarrier->lock = LLL_LOCK_INITIALIZER;
  ibarrier->left = count;
  ibarrier->init_count = count;
  ibarrier->curr_event = 0;

#ifdef __ASSUME_PRIVATE_FUTEX
  ibarrier->private = (iattr->pshared != PTHREAD_PROCESS_PRIVATE
		       ? 0 : FUTEX_PRIVATE_FLAG);
#else
  ibarrier->private = (iattr->pshared != PTHREAD_PROCESS_PRIVATE
		       ? 0 : THREAD_GETMEM (THREAD_SELF,
					    header.private_futex));
#endif

  return 0;
}
