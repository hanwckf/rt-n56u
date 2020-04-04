/* Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007
   Free Software Foundation, Inc.
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

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <bits/kernel-features.h>
#include "pthreadP.h"

static const struct pthread_mutexattr default_attr =
  {
    /* Default is a normal mutex, not shared between processes.  */
    .mutexkind = PTHREAD_MUTEX_NORMAL
  };


#ifndef __ASSUME_FUTEX_LOCK_PI
static int tpi_supported;
#endif


int
attribute_protected
__pthread_mutex_init (
     pthread_mutex_t *mutex,
     const pthread_mutexattr_t *mutexattr)
{
  const struct pthread_mutexattr *imutexattr;

  assert (sizeof (pthread_mutex_t) <= __SIZEOF_PTHREAD_MUTEX_T);

  imutexattr = (const struct pthread_mutexattr *) mutexattr ?: &default_attr;

  /* Sanity checks.  */
  switch (__builtin_expect (imutexattr->mutexkind
			    & PTHREAD_MUTEXATTR_PROTOCOL_MASK,
			    PTHREAD_PRIO_NONE
			    << PTHREAD_MUTEXATTR_PROTOCOL_SHIFT))
    {
    case PTHREAD_PRIO_NONE << PTHREAD_MUTEXATTR_PROTOCOL_SHIFT:
      break;

    case PTHREAD_PRIO_INHERIT << PTHREAD_MUTEXATTR_PROTOCOL_SHIFT:
#ifndef __ASSUME_FUTEX_LOCK_PI
      if (__builtin_expect (tpi_supported == 0, 0))
	{
	  int lock = 0;
	  INTERNAL_SYSCALL_DECL (err);
	  int ret = INTERNAL_SYSCALL (futex, err, 4, &lock, FUTEX_UNLOCK_PI,
				      0, 0);
	  assert (INTERNAL_SYSCALL_ERROR_P (ret, err));
	  tpi_supported = INTERNAL_SYSCALL_ERRNO (ret, err) == ENOSYS ? -1 : 1;
	}
      if (__builtin_expect (tpi_supported < 0, 0))
	return ENOTSUP;
#endif
      break;

    default:
      /* XXX: For now we don't support robust priority protected mutexes.  */
      if (imutexattr->mutexkind & PTHREAD_MUTEXATTR_FLAG_ROBUST)
	return ENOTSUP;
      break;
    }

  /* Clear the whole variable.  */
  memset (mutex, '\0', __SIZEOF_PTHREAD_MUTEX_T);

  /* Copy the values from the attribute.  */
  mutex->__data.__kind = imutexattr->mutexkind & ~PTHREAD_MUTEXATTR_FLAG_BITS;

  if ((imutexattr->mutexkind & PTHREAD_MUTEXATTR_FLAG_ROBUST) != 0)
    {
#ifndef __ASSUME_SET_ROBUST_LIST
      if ((imutexattr->mutexkind & PTHREAD_MUTEXATTR_FLAG_PSHARED) != 0
	  && __set_robust_list_avail < 0)
	return ENOTSUP;
#endif

      mutex->__data.__kind |= PTHREAD_MUTEX_ROBUST_NORMAL_NP;
    }

  switch (imutexattr->mutexkind & PTHREAD_MUTEXATTR_PROTOCOL_MASK)
    {
    case PTHREAD_PRIO_INHERIT << PTHREAD_MUTEXATTR_PROTOCOL_SHIFT:
      mutex->__data.__kind |= PTHREAD_MUTEX_PRIO_INHERIT_NP;
      break;

    case PTHREAD_PRIO_PROTECT << PTHREAD_MUTEXATTR_PROTOCOL_SHIFT:
      mutex->__data.__kind |= PTHREAD_MUTEX_PRIO_PROTECT_NP;

      int ceiling = (imutexattr->mutexkind
		     & PTHREAD_MUTEXATTR_PRIO_CEILING_MASK)
		    >> PTHREAD_MUTEXATTR_PRIO_CEILING_SHIFT;
      if (! ceiling)
	{
	  if (__sched_fifo_min_prio == -1)
	    __init_sched_fifo_prio ();
	  if (ceiling < __sched_fifo_min_prio)
	    ceiling = __sched_fifo_min_prio;
	}
      mutex->__data.__lock = ceiling << PTHREAD_MUTEX_PRIO_CEILING_SHIFT;
      break;

    default:
      break;
    }

  /* The kernel when waking robust mutexes on exit never uses
     FUTEX_PRIVATE_FLAG FUTEX_WAKE.  */
  if ((imutexattr->mutexkind & (PTHREAD_MUTEXATTR_FLAG_PSHARED
				| PTHREAD_MUTEXATTR_FLAG_ROBUST)) != 0)
    mutex->__data.__kind |= PTHREAD_MUTEX_PSHARED_BIT;

  /* Default values: mutex not used yet.  */
  // mutex->__count = 0;	already done by memset
  // mutex->__owner = 0;	already done by memset
  // mutex->__nusers = 0;	already done by memset
  // mutex->__spins = 0;	already done by memset
  // mutex->__next = NULL;	already done by memset

  return 0;
}
strong_alias (__pthread_mutex_init, pthread_mutex_init)
INTDEF(__pthread_mutex_init)
