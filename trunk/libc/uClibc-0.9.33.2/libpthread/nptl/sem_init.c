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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <errno.h>
#include <semaphore.h>
#include <lowlevellock.h>
#include "semaphoreP.h"
#include <bits/kernel-features.h>


int
__new_sem_init (
     sem_t *sem,
     int pshared,
     unsigned int value)
{
  /* Parameter sanity check.  */
  if (__builtin_expect (value > SEM_VALUE_MAX, 0))
    {
      __set_errno (EINVAL);
      return -1;
    }

  /* Map to the internal type.  */
  struct new_sem *isem = (struct new_sem *) sem;

  /* Use the values the user provided.  */
  isem->value = value;
#ifdef __ASSUME_PRIVATE_FUTEX
  isem->private = pshared ? 0 : FUTEX_PRIVATE_FLAG;
#else
  isem->private = pshared ? 0 : THREAD_GETMEM (THREAD_SELF,
					       header.private_futex);
#endif

  isem->nwaiters = 0;

  return 0;
}
weak_alias(__new_sem_init, sem_init)
