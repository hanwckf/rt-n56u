/* Copyright (C) 2002 Free Software Foundation, Inc.
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
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>


static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;


static void *
tf (void *arg)
{
  if (pthread_mutex_lock (&lock) != 0)
    {
      puts ("child: mutex_lock failed");
      return NULL;
    }

  return (void *) 42l;
}


static int
do_test (void)
{
  pthread_t th;

  if (pthread_mutex_lock (&lock) != 0)
    {
      puts ("mutex_lock failed");
      exit (1);
    }

  if (pthread_create (&th, NULL, tf, NULL) != 0)
    {
      puts ("mutex_create failed");
      exit (1);
    }

  void *status;
  struct timespec ts;
  struct timeval tv;
  (void) gettimeofday (&tv, NULL);
  TIMEVAL_TO_TIMESPEC (&tv, &ts);
  ts.tv_nsec += 200000000;
  if (ts.tv_nsec >= 1000000000)
    {
      ts.tv_nsec -= 1000000000;
      ++ts.tv_sec;
    }
  int val = pthread_timedjoin_np (th, &status, &ts);
  if (val == 0)
    {
      puts ("1st timedjoin succeeded");
      exit (1);
    }
  else if (val != ETIMEDOUT)
    {
      puts ("1st timedjoin didn't return ETIMEDOUT");
      exit (1);
    }

  if (pthread_mutex_unlock (&lock) != 0)
    {
      puts ("mutex_unlock failed");
      exit (1);
    }

  while (1)
    {
      (void) gettimeofday (&tv, NULL);
      TIMEVAL_TO_TIMESPEC (&tv, &ts);
      ts.tv_nsec += 200000000;
      if (ts.tv_nsec >= 1000000000)
	{
	  ts.tv_nsec -= 1000000000;
	  ++ts.tv_sec;
	}

      val = pthread_timedjoin_np (th, &status, &ts);
      if (val == 0)
	break;

      if (val != ETIMEDOUT)
	{
	  printf ("timedjoin returned %s (%d), expected only 0 or ETIMEDOUT\n",
		  strerror (val), val);
	  exit (1);
	}
    }

  if (status != (void *) 42l)
    {
      printf ("return value %p, expected %p\n", status, (void *) 42l);
      exit (1);
    }

  return 0;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
