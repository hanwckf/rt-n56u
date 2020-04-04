/* Copyright (C) 2002, 2003, 2007 Free Software Foundation, Inc.
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
__pthread_rwlock_trywrlock (
     pthread_rwlock_t *rwlock)
{
  int result = EBUSY;

  lll_lock (rwlock->__data.__lock, rwlock->__data.__shared);

  if (rwlock->__data.__writer == 0 && rwlock->__data.__nr_readers == 0)
    {
      rwlock->__data.__writer = THREAD_GETMEM (THREAD_SELF, tid);
      result = 0;
    }

  lll_unlock (rwlock->__data.__lock, rwlock->__data.__shared);

  return result;
}
strong_alias (__pthread_rwlock_trywrlock, pthread_rwlock_trywrlock)
