/* Copyright (C) 2002, 2003, 2005 Free Software Foundation, Inc.
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
#include <stdlib.h>

#include <atomic.h>
#include "pthreadP.h"


int
pthread_tryjoin_np (
     pthread_t threadid,
     void **thread_return)
{
  struct pthread *self;
  struct pthread *pd = (struct pthread *) threadid;

  /* Make sure the descriptor is valid.  */
  if (DEBUGGING_P && __find_in_stack_list (pd) == NULL)
    /* Not a valid thread handle.  */
    return ESRCH;

  /* Is the thread joinable?.  */
  if (IS_DETACHED (pd))
    /* We cannot wait for the thread.  */
    return EINVAL;

  self = THREAD_SELF;
  if (pd == self || self->joinid == pd)
    /* This is a deadlock situation.  The threads are waiting for each
       other to finish.  Note that this is a "may" error.  To be 100%
       sure we catch this error we would have to lock the data
       structures but it is not necessary.  In the unlikely case that
       two threads are really caught in this situation they will
       deadlock.  It is the programmer's problem to figure this
       out.  */
    return EDEADLK;

  /* Return right away if the thread hasn't terminated yet.  */
  if (pd->tid != 0)
    return EBUSY;

  /* Wait for the thread to finish.  If it is already locked something
     is wrong.  There can only be one waiter.  */
  if (atomic_compare_and_exchange_bool_acq (&pd->joinid, self, NULL))
    /* There is already somebody waiting for the thread.  */
    return EINVAL;

  /* Store the return value if the caller is interested.  */
  if (thread_return != NULL)
    *thread_return = pd->result;


  /* Free the TCB.  */
  __free_tcb (pd);

  return 0;
}
