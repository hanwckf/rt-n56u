/* sem_wait -- wait on a semaphore.  Generic futex-using version.
   Copyright (C) 2003, 2007 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Paul Mackerras <paulus@au.ibm.com>, 2003.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <errno.h>
#include <sysdep.h>
#include <lowlevellock.h>
#include <internaltypes.h>
#include <semaphore.h>

#include <pthreadP.h>


void
attribute_hidden
__sem_wait_cleanup (void *arg)
{
  struct new_sem *isem = (struct new_sem *) arg;

  atomic_decrement (&isem->nwaiters);
}


int
__new_sem_wait (sem_t *sem)
{
  struct new_sem *isem = (struct new_sem *) sem;
  int err;

  if (atomic_decrement_if_positive (&isem->value) > 0)
    return 0;

  atomic_increment (&isem->nwaiters);

  pthread_cleanup_push (__sem_wait_cleanup, isem);

  while (1)
    {
      /* Enable asynchronous cancellation.  Required by the standard.  */
      int oldtype = __pthread_enable_asynccancel ();

      err = lll_futex_wait (&isem->value, 0,
			    isem->private ^ FUTEX_PRIVATE_FLAG);

      /* Disable asynchronous cancellation.  */
      __pthread_disable_asynccancel (oldtype);

      if (err != 0 && err != -EWOULDBLOCK)
	{
	  __set_errno (-err);
	  err = -1;
	  break;
	}

      if (atomic_decrement_if_positive (&isem->value) > 0)
	{
	  err = 0;
	  break;
	}
    }

  pthread_cleanup_pop (0);

  atomic_decrement (&isem->nwaiters);

  return err;
}
weak_alias(__new_sem_wait, sem_wait)
