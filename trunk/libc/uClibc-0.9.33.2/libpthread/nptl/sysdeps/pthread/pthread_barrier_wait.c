/* Copyright (C) 2003, 2004, 2007 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <errno.h>
#include <sysdep.h>
#include <lowlevellock.h>
#include <pthreadP.h>


/* Wait on barrier.  */
int
pthread_barrier_wait (
     pthread_barrier_t *barrier)
{
  struct pthread_barrier *ibarrier = (struct pthread_barrier *) barrier;
  int result = 0;

  /* Make sure we are alone.  */
  lll_lock (ibarrier->lock, ibarrier->private ^ FUTEX_PRIVATE_FLAG);

  /* One more arrival.  */
  --ibarrier->left;

  /* Are these all?  */
  if (ibarrier->left == 0)
    {
      /* Yes. Increment the event counter to avoid invalid wake-ups and
	 tell the current waiters that it is their turn.  */
      ++ibarrier->curr_event;

      /* Wake up everybody.  */
      lll_futex_wake (&ibarrier->curr_event, INT_MAX,
		      ibarrier->private ^ FUTEX_PRIVATE_FLAG);

      /* This is the thread which finished the serialization.  */
      result = PTHREAD_BARRIER_SERIAL_THREAD;
    }
  else
    {
      /* The number of the event we are waiting for.  The barrier's event
	 number must be bumped before we continue.  */
      unsigned int event = ibarrier->curr_event;

      /* Before suspending, make the barrier available to others.  */
      lll_unlock (ibarrier->lock, ibarrier->private ^ FUTEX_PRIVATE_FLAG);

      /* Wait for the event counter of the barrier to change.  */
      do
	lll_futex_wait (&ibarrier->curr_event, event,
			ibarrier->private ^ FUTEX_PRIVATE_FLAG);
      while (event == ibarrier->curr_event);
    }

  /* Make sure the init_count is stored locally or in a register.  */
  unsigned int init_count = ibarrier->init_count;

  /* If this was the last woken thread, unlock.  */
  if (atomic_increment_val (&ibarrier->left) == init_count)
    /* We are done.  */
    lll_unlock (ibarrier->lock, ibarrier->private ^ FUTEX_PRIVATE_FLAG);

  return result;
}
