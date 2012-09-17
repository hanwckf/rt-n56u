/* Linuxthreads - a simple clone()-based implementation of Posix        */
/* threads for Linux.                                                   */
/* Copyright (C) 1998 Xavier Leroy (Xavier.Leroy@inria.fr)              */
/*                                                                      */
/* This program is free software; you can redistribute it and/or        */
/* modify it under the terms of the GNU Library General Public License  */
/* as published by the Free Software Foundation; either version 2       */
/* of the License, or (at your option) any later version.               */
/*                                                                      */
/* This program is distributed in the hope that it will be useful,      */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
/* GNU Library General Public License for more details.                 */

#include <bits/initspin.h>


/* There are 2 compare and swap synchronization primitives with
   different semantics:

	1. compare_and_swap, which has acquire semantics (i.e. it
	completes befor subsequent writes.)
	2. compare_and_swap_with_release_semantics, which has release
	semantics (it completes after previous writes.)

   For those platforms on which they are the same. HAS_COMPARE_AND_SWAP
   should be defined. For those platforms on which they are different,
   HAS_COMPARE_AND_SWAP_WITH_RELEASE_SEMANTICS has to be defined.  */

#ifndef HAS_COMPARE_AND_SWAP
#ifdef HAS_COMPARE_AND_SWAP_WITH_RELEASE_SEMANTICS
#define HAS_COMPARE_AND_SWAP
#endif
#endif

#if defined(TEST_FOR_COMPARE_AND_SWAP)

extern int __pthread_has_cas;
extern int __pthread_compare_and_swap(long * ptr, long oldval, long newval,
                                      int * spinlock);

static __inline__ int compare_and_swap(long * ptr, long oldval, long newval,
                                   int * spinlock)
{
  if (__builtin_expect (__pthread_has_cas, 1))
    return __compare_and_swap(ptr, oldval, newval);
  else
    return __pthread_compare_and_swap(ptr, oldval, newval, spinlock);
}

#elif defined(HAS_COMPARE_AND_SWAP)

#ifdef IMPLEMENT_TAS_WITH_CAS
#define testandset(p) !__compare_and_swap((long int *) p, 0, 1)
#endif

#ifdef HAS_COMPARE_AND_SWAP_WITH_RELEASE_SEMANTICS

static __inline__ int
compare_and_swap_with_release_semantics (long * ptr, long oldval,
					 long newval, int * spinlock)
{
  return __compare_and_swap_with_release_semantics (ptr, oldval,
						    newval);
}

#endif

static __inline__ int compare_and_swap(long * ptr, long oldval, long newval,
                                   int * spinlock)
{
  return __compare_and_swap(ptr, oldval, newval);
}

#else

extern int __pthread_compare_and_swap(long * ptr, long oldval, long newval,
                                      int * spinlock);

static __inline__ int compare_and_swap(long * ptr, long oldval, long newval,
                                   int * spinlock)
{
  return __pthread_compare_and_swap(ptr, oldval, newval, spinlock);
}

#endif

#ifndef HAS_COMPARE_AND_SWAP_WITH_RELEASE_SEMANTICS
#define compare_and_swap_with_release_semantics compare_and_swap
#define __compare_and_swap_with_release_semantics __compare_and_swap
#endif

/* Internal locks */

extern void internal_function __pthread_lock(struct _pthread_fastlock * lock,
					     pthread_descr self);
extern int __pthread_unlock(struct _pthread_fastlock *lock);

static __inline__ void __pthread_init_lock(struct _pthread_fastlock * lock)
{
  lock->__status = 0;
  lock->__spinlock = __LT_SPINLOCK_INIT;
}

static __inline__ int __pthread_trylock (struct _pthread_fastlock * lock)
{
#if defined TEST_FOR_COMPARE_AND_SWAP
  if (!__pthread_has_cas)
#endif
#if !defined HAS_COMPARE_AND_SWAP || defined TEST_FOR_COMPARE_AND_SWAP
  {
    return (testandset(&lock->__spinlock) ? EBUSY : 0);
  }
#endif

#if defined HAS_COMPARE_AND_SWAP
  do {
    if (lock->__status != 0) return EBUSY;
  } while(! __compare_and_swap(&lock->__status, 0, 1));
  return 0;
#endif
}

/* Variation of internal lock used for pthread_mutex_t, supporting
   timed-out waits.  Warning: do not mix these operations with the above ones
   over the same lock object! */

extern void __pthread_alt_lock(struct _pthread_fastlock * lock,
			       pthread_descr self);

extern int __pthread_alt_timedlock(struct _pthread_fastlock * lock,
			       pthread_descr self, const struct timespec *abstime);

extern void __pthread_alt_unlock(struct _pthread_fastlock *lock);

static __inline__ void __pthread_alt_init_lock(struct _pthread_fastlock * lock)
{
  lock->__status = 0;
  lock->__spinlock = __LT_SPINLOCK_INIT;
}

static __inline__ int __pthread_alt_trylock (struct _pthread_fastlock * lock)
{
#if defined TEST_FOR_COMPARE_AND_SWAP
  if (!__pthread_has_cas)
#endif
#if !defined HAS_COMPARE_AND_SWAP || defined TEST_FOR_COMPARE_AND_SWAP
  {
    int res = EBUSY;

    if (testandset(&lock->__spinlock) == 0)
      {
	if (lock->__status == 0)
	  {
	    lock->__status = 1;
	    WRITE_MEMORY_BARRIER();
	    res = 0;
	  }
	lock->__spinlock = __LT_SPINLOCK_INIT;
      }
    return res;
  }
#endif

#if defined HAS_COMPARE_AND_SWAP
  do {
    if (lock->__status != 0) return EBUSY;
  } while(! compare_and_swap(&lock->__status, 0, 1, &lock->__spinlock));
  return 0;
#endif
}

/* Operations on pthread_atomic, which is defined in internals.h */

static __inline__ long
pthread_atomic_increment (struct pthread_atomic *pa)
{
    long oldval;

    do {
	oldval = pa->p_count;
    } while (!compare_and_swap(&pa->p_count, oldval, oldval + 1, &pa->p_spinlock));

    return oldval;
}


static __inline__ long
pthread_atomic_decrement (struct pthread_atomic *pa)
{
    long oldval;

    do {
	oldval = pa->p_count;
    } while (!compare_and_swap(&pa->p_count, oldval, oldval - 1, &pa->p_spinlock));

    return oldval;
}


static __inline__ __attribute__((always_inline)) void
__pthread_set_own_extricate_if (pthread_descr self, pthread_extricate_if *peif)
{
  /* Only store a non-null peif if the thread has cancellation enabled.
     Otherwise pthread_cancel will unconditionally call the extricate handler,
     and restart the thread giving rise to forbidden spurious wakeups. */
  if (peif == NULL
      || THREAD_GETMEM(self, p_cancelstate) == PTHREAD_CANCEL_ENABLE)
    {
      /* If we are removing the extricate interface, we need to synchronize
	 against pthread_cancel so that it does not continue with a pointer
         to a deallocated pthread_extricate_if struct! The thread lock
         is (ab)used for this synchronization purpose. */
      if (peif == NULL)
	__pthread_lock (THREAD_GETMEM(self, p_lock), self);
      THREAD_SETMEM(self, p_extricate, peif);
      if (peif == NULL)
	__pthread_unlock (THREAD_GETMEM(self, p_lock));
    }
}
