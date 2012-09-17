/* clock_gettime -- Get current time from a POSIX clockid_t.  Linux version.
   Copyright (C) 2003, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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

#include <sysdep.h>
#include <errno.h>
#include <time.h>
#include "kernel-posix-cpu-timers.h"
#include <bits/kernel-features.h>


#define SYSCALL_GETTIME \
  retval = INLINE_SYSCALL (clock_gettime, 2, clock_id, tp); \
  break

#ifdef __ASSUME_POSIX_TIMERS

/* This means the REALTIME and MONOTONIC clock are definitely
   supported in the kernel.  */
# define SYSDEP_GETTIME							      \
  SYSDEP_GETTIME_CPUTIME						      \
  case CLOCK_REALTIME:							      \
  case CLOCK_MONOTONIC:							      \
    SYSCALL_GETTIME

# define __libc_missing_posix_timers 0
#elif defined __NR_clock_gettime
/* Is the syscall known to exist?  */
int __libc_missing_posix_timers attribute_hidden;

static inline int
maybe_syscall_gettime (clockid_t clock_id, struct timespec *tp)
{
  int e = EINVAL;

  if (!__libc_missing_posix_timers)
    {
      INTERNAL_SYSCALL_DECL (err);
      int r = INTERNAL_SYSCALL (clock_gettime, err, 2, clock_id, tp);
      if (!INTERNAL_SYSCALL_ERROR_P (r, err))
	return 0;

      e = INTERNAL_SYSCALL_ERRNO (r, err);
      if (e == ENOSYS)
	{
	  __libc_missing_posix_timers = 1;
	  e = EINVAL;
	}
    }

  return e;
}

/* The REALTIME and MONOTONIC clock might be available.  Try the
   syscall first.  */
# define SYSDEP_GETTIME							      \
  SYSDEP_GETTIME_CPUTIME						      \
  case CLOCK_REALTIME:							      \
  case CLOCK_MONOTONIC:							      \
    retval = maybe_syscall_gettime (clock_id, tp);			      \
    if (retval == 0)							      \
      break;								      \
    /* Fallback code.  */						      \
    if (retval == EINVAL && clock_id == CLOCK_REALTIME)			      \
      retval = realtime_gettime (tp);					      \
    else 								      \
      {									      \
	__set_errno (retval);						      \
	retval = -1;							      \
      }									      \
    break;
#endif

#ifdef __NR_clock_gettime
/* We handled the REALTIME clock here.  */
# define HANDLED_REALTIME	1
# define HANDLED_CPUTIME	1

# if __ASSUME_POSIX_CPU_TIMERS > 0

#  define SYSDEP_GETTIME_CPU SYSCALL_GETTIME
#  define SYSDEP_GETTIME_CPUTIME	/* Default catches them too.  */

# else

int __libc_missing_posix_cpu_timers attribute_hidden;

static int
maybe_syscall_gettime_cpu (clockid_t clock_id, struct timespec *tp)
{
  int e = EINVAL;

  if (!__libc_missing_posix_cpu_timers)
    {
      INTERNAL_SYSCALL_DECL (err);
      int r = INTERNAL_SYSCALL (clock_gettime, err, 2, clock_id, tp);
      if (!INTERNAL_SYSCALL_ERROR_P (r, err))
	return 0;

      e = INTERNAL_SYSCALL_ERRNO (r, err);
# ifndef __ASSUME_POSIX_TIMERS
      if (e == ENOSYS)
	{
	  __libc_missing_posix_timers = 1;
	  __libc_missing_posix_cpu_timers = 1;
	  e = EINVAL;
	}
      else
# endif
	{
	  if (e == EINVAL)
	    {
	      /* Check whether the kernel supports CPU clocks at all.
		 If not, record it for the future.  */
	      r = INTERNAL_SYSCALL (clock_getres, err, 2,
				    MAKE_PROCESS_CPUCLOCK (0, CPUCLOCK_SCHED),
				    NULL);
	      if (INTERNAL_SYSCALL_ERROR_P (r, err))
		__libc_missing_posix_cpu_timers = 1;
	    }
	}
    }

  return e;
}

#  define SYSDEP_GETTIME_CPU						      \
  retval = maybe_syscall_gettime_cpu (clock_id, tp);			      \
  if (retval == 0)							      \
    break;								      \
  if (retval != EINVAL || !__libc_missing_posix_cpu_timers)		      \
    {									      \
      __set_errno (retval);						      \
      retval = -1;							      \
      break;								      \
    }									      \
  retval = -1 /* Otherwise continue on to the HP_TIMING version.  */;

static inline int
maybe_syscall_gettime_cputime (clockid_t clock_id, struct timespec *tp)
{
  return maybe_syscall_gettime_cpu
    (clock_id == CLOCK_THREAD_CPUTIME_ID
     ? MAKE_THREAD_CPUCLOCK (0, CPUCLOCK_SCHED)
     : MAKE_PROCESS_CPUCLOCK (0, CPUCLOCK_SCHED),
     tp);
}

#  define SYSDEP_GETTIME_CPUTIME					      \
    case CLOCK_PROCESS_CPUTIME_ID:					      \
    case CLOCK_THREAD_CPUTIME_ID:					      \
      retval = maybe_syscall_gettime_cputime (clock_id, tp);		      \
      if (retval == 0)							      \
	break;								      \
      if (retval != EINVAL || !__libc_missing_posix_cpu_timers)		      \
	{								      \
	  __set_errno (retval);						      \
	  retval = -1;							      \
	  break;							      \
	}								      \
      retval = hp_timing_gettime (clock_id, tp);			      \
      break;
#  if !HP_TIMING_AVAIL
#   define hp_timing_gettime(clock_id, tp) (__set_errno (EINVAL), -1)
#  endif

# endif
#endif

#include <errno.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <libc-internal.h>
#include <ldsodefs.h>


#if HP_TIMING_AVAIL
/* Clock frequency of the processor.  We make it a 64-bit variable
   because some jokers are already playing with processors with more
   than 4GHz.  */
static hp_timing_t freq;


/* This function is defined in the thread library.  */
extern int __pthread_clock_gettime (clockid_t clock_id, hp_timing_t freq,
				    struct timespec *tp)
     __attribute__ ((__weak__));

static int
hp_timing_gettime (clockid_t clock_id, struct timespec *tp)
{
  hp_timing_t tsc;

  if (__builtin_expect (freq == 0, 0))
    {
      /* This can only happen if we haven't initialized the `freq'
	 variable yet.  Do this now. We don't have to protect this
	 code against multiple execution since all of them should
	 lead to the same result.  */
      freq = __get_clockfreq ();
      if (__builtin_expect (freq == 0, 0))
	/* Something went wrong.  */
	return -1;
    }

  if (clock_id != CLOCK_PROCESS_CPUTIME_ID
      && __pthread_clock_gettime != NULL)
    return __pthread_clock_gettime (clock_id, freq, tp);

  /* Get the current counter.  */
  HP_TIMING_NOW (tsc);

  /* Compute the offset since the start time of the process.  */
  tsc -= GL(dl_cpuclock_offset);

  /* Compute the seconds.  */
  tp->tv_sec = tsc / freq;

  /* And the nanoseconds.  This computation should be stable until
     we get machines with about 16GHz frequency.  */
  tp->tv_nsec = ((tsc % freq) * UINT64_C (1000000000)) / freq;

  return 0;
}
#endif


static inline int
realtime_gettime (struct timespec *tp)
{
  struct timeval tv;
  int retval = gettimeofday (&tv, NULL);
  if (retval == 0)
    /* Convert into `timespec'.  */
    TIMEVAL_TO_TIMESPEC (&tv, tp);
  return retval;
}

librt_hidden_proto (clock_gettime)
/* Get current value of CLOCK and store it in TP.  */
int
clock_gettime (clockid_t clock_id, struct timespec *tp)
{
  int retval = -1;
#ifndef HANDLED_REALTIME
  struct timeval tv;
#endif

  switch (clock_id)
    {
#ifdef SYSDEP_GETTIME
      SYSDEP_GETTIME;
#endif

#ifndef HANDLED_REALTIME
    case CLOCK_REALTIME:
      retval = gettimeofday (&tv, NULL);
      if (retval == 0)
	TIMEVAL_TO_TIMESPEC (&tv, tp);
      break;
#endif

    default:
#ifdef SYSDEP_GETTIME_CPU
      SYSDEP_GETTIME_CPU;
#endif
#if HP_TIMING_AVAIL
      if ((clock_id & ((1 << CLOCK_IDFIELD_SIZE) - 1))
	  == CLOCK_THREAD_CPUTIME_ID)
	retval = hp_timing_gettime (clock_id, tp);
      else
#endif
	__set_errno (EINVAL);
      break;

#if HP_TIMING_AVAIL && !defined HANDLED_CPUTIME
    case CLOCK_PROCESS_CPUTIME_ID:
      retval = hp_timing_gettime (clock_id, tp);
      break;
#endif
    }

  return retval;
}
librt_hidden_def (clock_gettime)
