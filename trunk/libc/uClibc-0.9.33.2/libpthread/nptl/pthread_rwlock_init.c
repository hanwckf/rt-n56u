/* Copyright (C) 2002, 2007, 2009 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include "pthreadP.h"
#include <bits/kernel-features.h>
#include <string.h>

static const struct pthread_rwlockattr default_attr =
  {
    .lockkind = PTHREAD_RWLOCK_DEFAULT_NP,
    .pshared = PTHREAD_PROCESS_PRIVATE
  };


int
__pthread_rwlock_init (
     pthread_rwlock_t *rwlock,
     const pthread_rwlockattr_t *attr)
{
  const struct pthread_rwlockattr *iattr;

  iattr = ((const struct pthread_rwlockattr *) attr) ?: &default_attr;

  memset (rwlock, '\0', sizeof (*rwlock));

  rwlock->__data.__flags
    = iattr->lockkind == PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP;

  /* The __SHARED field is computed to minimize the work that needs to
     be done while handling the futex.  There are two inputs: the
     availability of private futexes and whether the rwlock is shared
     or private.  Unfortunately the value of a private rwlock is
     fixed: it must be zero.  The PRIVATE_FUTEX flag has the value
     0x80 in case private futexes are available and zero otherwise.
     This leads to the following table:

		 |     pshared     |     result
		 | shared  private | shared  private |
     ------------+-----------------+-----------------+
     !avail 0    |     0       0   |     0       0   |
      avail 0x80 |  0x80       0   |     0    0x80   |

     If the pshared value is in locking functions XORed with avail
     we get the expected result.  */
#ifdef __ASSUME_PRIVATE_FUTEX
  rwlock->__data.__shared = (iattr->pshared == PTHREAD_PROCESS_PRIVATE
			     ? 0 : FUTEX_PRIVATE_FLAG);
#else
  rwlock->__data.__shared = (iattr->pshared == PTHREAD_PROCESS_PRIVATE
			     ? 0
			     : THREAD_GETMEM (THREAD_SELF,
					      header.private_futex));
#endif

  return 0;
}
strong_alias (__pthread_rwlock_init, pthread_rwlock_init)
