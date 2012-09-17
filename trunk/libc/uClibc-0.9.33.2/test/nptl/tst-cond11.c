/* Copyright (C) 2003, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2003.

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
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>


#if defined _POSIX_CLOCK_SELECTION && _POSIX_CLOCK_SELECTION >= 0
static int
run_test (clockid_t cl)
{
  pthread_condattr_t condattr;
  pthread_cond_t cond;
  pthread_mutexattr_t mutattr;
  pthread_mutex_t mut;

  printf ("clock = %d\n", (int) cl);

  if (pthread_condattr_init (&condattr) != 0)
    {
      puts ("condattr_init failed");
      return 1;
    }

  if (pthread_condattr_setclock (&condattr, cl) != 0)
    {
      puts ("condattr_setclock failed");
      return 1;
    }

  clockid_t cl2;
  if (pthread_condattr_getclock (&condattr, &cl2) != 0)
    {
      puts ("condattr_getclock failed");
      return 1;
    }
  if (cl != cl2)
    {
      printf ("condattr_getclock returned wrong value: %d, expected %d\n",
	      (int) cl2, (int) cl);
      return 1;
    }

  if (pthread_cond_init (&cond, &condattr) != 0)
    {
      puts ("cond_init failed");
      return 1;
    }

  if (pthread_condattr_destroy (&condattr) != 0)
    {
      puts ("condattr_destroy failed");
      return 1;
    }

  if (pthread_mutexattr_init (&mutattr) != 0)
    {
      puts ("mutexattr_init failed");
      return 1;
    }

  if (pthread_mutexattr_settype (&mutattr, PTHREAD_MUTEX_ERRORCHECK) != 0)
    {
      puts ("mutexattr_settype failed");
      return 1;
    }

  if (pthread_mutex_init (&mut, &mutattr) != 0)
    {
      puts ("mutex_init failed");
      return 1;
    }

  if (pthread_mutexattr_destroy (&mutattr) != 0)
    {
      puts ("mutexattr_destroy failed");
      return 1;
    }

  if (pthread_mutex_lock (&mut) != 0)
    {
      puts ("mutex_lock failed");
      return 1;
    }

  if (pthread_mutex_lock (&mut) != EDEADLK)
    {
      puts ("2nd mutex_lock did not return EDEADLK");
      return 1;
    }

  struct timespec ts;
  if (clock_gettime (cl, &ts) != 0)
    {
      puts ("clock_gettime failed");
      return 1;
    }

  /* Wait one second.  */
  ++ts.tv_sec;

  int e = pthread_cond_timedwait (&cond, &mut, &ts);
  if (e == 0)
    {
      puts ("cond_timedwait succeeded");
      return 1;
    }
  else if (e != ETIMEDOUT)
    {
      puts ("cond_timedwait did not return ETIMEDOUT");
      return 1;
    }

  if (pthread_mutex_unlock (&mut) != 0)
    {
      puts ("mutex_unlock failed");
      return 1;
    }

  if (pthread_mutex_destroy (&mut) != 0)
    {
      puts ("mutex_destroy failed");
      return 1;
    }

  if (pthread_cond_destroy (&cond) != 0)
    {
      puts ("cond_destroy failed");
      return 1;
    }

  return 0;
}
#endif


static int
do_test (void)
{
#if !defined _POSIX_CLOCK_SELECTION || _POSIX_CLOCK_SELECTION == -1

  puts ("_POSIX_CLOCK_SELECTION not supported, test skipped");
  return 0;

#else

  int res = run_test (CLOCK_REALTIME);

# if defined _POSIX_MONOTONIC_CLOCK && _POSIX_MONOTONIC_CLOCK >= 0
#  if _POSIX_MONOTONIC_CLOCK == 0
  int e = sysconf (_SC_MONOTONIC_CLOCK);
  if (e < 0)
    puts ("CLOCK_MONOTONIC not supported");
  else if (e == 0)
    {
      puts ("sysconf (_SC_MONOTONIC_CLOCK) must not return 0");
      res = 1;
    }
  else
#  endif
    res |= run_test (CLOCK_MONOTONIC);
# else
  puts ("_POSIX_MONOTONIC_CLOCK not defined");
# endif

  return res;
#endif
}

#define TIMEOUT 3
#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
