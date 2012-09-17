/* Copyright (C) 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2004.

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
#include <stdlib.h>
#include <time.h>


static pthread_barrier_t b;
static pthread_rwlock_t r = PTHREAD_RWLOCK_INITIALIZER;


static void *
tf (void *arg)
{
  /* Lock the read-write lock.  */
  if (pthread_rwlock_wrlock (&r) != 0)
    {
      puts ("tf: cannot lock rwlock");
      exit (EXIT_FAILURE);
    }

  pthread_t mt = *(pthread_t *) arg;

  pthread_barrier_wait (&b);

  /* This call will never return.  */
  pthread_join (mt, NULL);

  return NULL;
}


static int
do_test (void)
{
  int result = 0;
  struct timespec ts;

  if (clock_gettime (CLOCK_REALTIME, &ts) != 0)
    {
      puts ("clock_gettime failed");
      return 1;
    }

  if (pthread_barrier_init (&b, NULL, 2) != 0)
    {
      puts ("barrier_init failed");
      return 1;
    }

  pthread_t me = pthread_self ();
  pthread_t th;
  if (pthread_create (&th, NULL, tf, &me) != 0)
    {
      puts ("create failed");
      return 1;
    }

  /* Wait until the rwlock is locked.  */
  pthread_barrier_wait (&b);

  ts.tv_nsec = -1;

  int e = pthread_rwlock_timedrdlock (&r, &ts);
  if (e == 0)
    {
      puts ("first rwlock_timedrdlock did not fail");
      result = 1;
    }
  else if (e != EINVAL)
    {
      puts ("first rwlock_timedrdlock did not return EINVAL");
      result = 1;
    }

  e = pthread_rwlock_timedwrlock (&r, &ts);
  if (e == 0)
    {
      puts ("first rwlock_timedwrlock did not fail");
      result = 1;
    }
  else if (e != EINVAL)
    {
      puts ("first rwlock_timedwrlock did not return EINVAL");
      result = 1;
    }

  ts.tv_nsec = 1000000000;

  e = pthread_rwlock_timedrdlock (&r, &ts);
  if (e == 0)
    {
      puts ("second rwlock_timedrdlock did not fail");
      result = 1;
    }
  else if (e != EINVAL)
    {
      puts ("second rwlock_timedrdlock did not return EINVAL");
      result = 1;
    }

  e = pthread_rwlock_timedrdlock (&r, &ts);
  if (e == 0)
    {
      puts ("second rwlock_timedrdlock did not fail");
      result = 1;
    }
  else if (e != EINVAL)
    {
      puts ("second rwlock_timedrdlock did not return EINVAL");
      result = 1;
    }

  ts.tv_nsec = 0x100001000LL;
  if (ts.tv_nsec != 0x100001000LL)
    ts.tv_nsec = 2000000000;

  e = pthread_rwlock_timedrdlock (&r, &ts);
  if (e == 0)
    {
      puts ("third rwlock_timedrdlock did not fail");
      result = 1;
    }
  else if (e != EINVAL)
    {
      puts ("third rwlock_timedrdlock did not return EINVAL");
      result = 1;
    }

  e = pthread_rwlock_timedrdlock (&r, &ts);
  if (e == 0)
    {
      puts ("third rwlock_timedrdlock did not fail");
      result = 1;
    }
  else if (e != EINVAL)
    {
      puts ("third rwlock_timedrdlock did not return EINVAL");
      result = 1;
    }

  if (result == 0)
    puts ("no bugs");

  return result;
}


#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
