/* Copyright (C) 2003, 2004, 2006, 2007 Free Software Foundation, Inc.
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
  union sparc_pthread_barrier *ibarrier
    = (union sparc_pthread_barrier *) barrier;
  int result = 0;
  int private = ibarrier->s.pshared ? LLL_SHARED : LLL_PRIVATE;

  /* Make sure we are alone.  */
  lll_lock (ibarrier->b.lock, private);

  /* One more arrival.  */
  --ibarrier->b.left;

  /* Are these all?  */
  if (ibarrier->b.left == 0)
    {
      /* Yes. Increment the event counter to avoid invalid wake-ups and
	 tell the current waiters that it is their turn.  */
      ++ibarrier->b.curr_event;

      /* Wake up everybody.  */
      lll_futex_wake (&ibarrier->b.curr_event, INT_MAX, private);

      /* This is the thread which finished the serialization.  */
      result = PTHREAD_BARRIER_SERIAL_THREAD;
    }
  else
    {
      /* The number of the event we are waiting for.  The barrier's event
	 number must be bumped before we continue.  */
      unsigned int event = ibarrier->b.curr_event;

      /* Before suspending, make the barrier available to others.  */
      lll_unlock (ibarrier->b.lock, private);

      /* Wait for the event counter of the barrier to change.  */
      do
	lll_futex_wait (&ibarrier->b.curr_event, event, private);
      while (event == ibarrier->b.curr_event);
    }

  /* Make sure the init_count is stored locally or in a register.  */
  unsigned int init_count = ibarrier->b.init_count;

  /* If this was the last woken thread, unlock.  */
  if (__atomic_is_v9 || ibarrier->s.pshared == 0)
    {
      if (atomic_increment_val (&ibarrier->b.left) == init_count)
	/* We are done.  */
	lll_unlock (ibarrier->b.lock, private);
    }
  else
    {
      unsigned int left;
      /* Slightly more complicated.  On pre-v9 CPUs, atomic_increment_val
	 is only atomic for threads within the same process, not for
	 multiple processes.  */
      __sparc32_atomic_do_lock24 (&ibarrier->s.left_lock);
      left = ++ibarrier->b.left;
      __sparc32_atomic_do_unlock24 (&ibarrier->s.left_lock);
      if (left == init_count)
        /* We are done.  */
	lll_unlock (ibarrier->b.lock, private);
    }

  return result;
}
