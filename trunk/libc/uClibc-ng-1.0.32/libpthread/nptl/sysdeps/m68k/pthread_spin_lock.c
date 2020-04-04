/* pthread_spin_lock -- lock a spin lock.  Generic version.
   Copyright (C) 2012-2016 Free Software Foundation, Inc.

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

#include <atomic.h>
#include "pthreadP.h"

/* A machine-specific version can define SPIN_LOCK_READS_BETWEEN_CMPXCHG
  to the number of plain reads that it's optimal to spin on between uses
  of atomic_compare_and_exchange_val_acq.  If spinning forever is optimal
  then use -1.  If no plain reads here would ever be optimal, use 0.  */
#define SPIN_LOCK_READS_BETWEEN_CMPXCHG 1000

int
pthread_spin_lock (pthread_spinlock_t *lock)
{
  /* atomic_exchange usually takes less instructions than
     atomic_compare_and_exchange.  On the other hand,
     atomic_compare_and_exchange potentially generates less bus traffic
     when the lock is locked.
     We assume that the first try mostly will be successful, and we use
     atomic_exchange.  For the subsequent tries we use
     atomic_compare_and_exchange.  */
  if (atomic_exchange_acq (lock, 1) == 0)
    return 0;

  do
    {
      /* The lock is contended and we need to wait.  Going straight back
	 to cmpxchg is not a good idea on many targets as that will force
	 expensive memory synchronizations among processors and penalize other
	 running threads.
	 On the other hand, we do want to update memory state on the local core
	 once in a while to avoid spinning indefinitely until some event that
	 will happen to update local memory as a side-effect.  */
      if (SPIN_LOCK_READS_BETWEEN_CMPXCHG >= 0)
	{
	  int wait = SPIN_LOCK_READS_BETWEEN_CMPXCHG;

	  while (*lock != 0 && wait > 0)
	    --wait;
	}
      else
	{
	  while (*lock != 0)
	    ;
	}
    }
  while (atomic_compare_and_exchange_val_acq (lock, 1, 0) != 0);

  return 0;
}
