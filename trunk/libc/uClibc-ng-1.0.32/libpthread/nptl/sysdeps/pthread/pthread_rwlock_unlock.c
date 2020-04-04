/* Copyright (C) 2003, 2007 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Martin Schwidefsky <schwidefsky@de.ibm.com>, 2003.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <sysdep.h>
#include <lowlevellock.h>
#include <pthread.h>
#include <pthreadP.h>

/* Unlock RWLOCK.  */
int
attribute_protected
__pthread_rwlock_unlock (pthread_rwlock_t *rwlock)
{
  lll_lock (rwlock->__data.__lock, rwlock->__data.__shared);
  if (rwlock->__data.__writer)
    rwlock->__data.__writer = 0;
  else
    --rwlock->__data.__nr_readers;
  if (rwlock->__data.__nr_readers == 0)
    {
      if (rwlock->__data.__nr_writers_queued)
	{
	  ++rwlock->__data.__writer_wakeup;
	  lll_unlock (rwlock->__data.__lock, rwlock->__data.__shared);
	  lll_futex_wake (&rwlock->__data.__writer_wakeup, 1,
			  rwlock->__data.__shared);
	  return 0;
	}
      else if (rwlock->__data.__nr_readers_queued)
	{
	  ++rwlock->__data.__readers_wakeup;
	  lll_unlock (rwlock->__data.__lock, rwlock->__data.__shared);
	  lll_futex_wake (&rwlock->__data.__readers_wakeup, INT_MAX,
			  rwlock->__data.__shared);
	  return 0;
	}
    }
  lll_unlock (rwlock->__data.__lock, rwlock->__data.__shared);
  return 0;
}

weak_alias (__pthread_rwlock_unlock, pthread_rwlock_unlock)
strong_alias (__pthread_rwlock_unlock, __pthread_rwlock_unlock_internal)
