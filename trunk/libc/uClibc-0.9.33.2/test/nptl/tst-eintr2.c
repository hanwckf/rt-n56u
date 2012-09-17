/* Copyright (C) 2003 Free Software Foundation, Inc.
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
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "eintr.c"


static pthread_mutex_t m1 = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t m2 = PTHREAD_MUTEX_INITIALIZER;


static void *
tf1 (void *arg)
{
  struct timespec ts;
  struct timeval tv;

  gettimeofday (&tv, NULL);
  TIMEVAL_TO_TIMESPEC (&tv, &ts);
  ts.tv_sec += 10000;

  /* This call must never return.  */
  int e = pthread_mutex_timedlock (&m1, &ts);
  char buf[100];
  printf ("tf1: mutex_timedlock returned: %s\n",
	  strerror_r (e, buf, sizeof (buf)));

  exit (1);
}


static void *
tf2 (void *arg)
{
  while (1)
    {
      int e = pthread_mutex_lock (&m2);
      if (e != 0)
	{
	  puts ("tf2: mutex_lock failed");
	  exit (1);
	}
      e = pthread_mutex_unlock (&m2);
      if (e != 0)
	{
	  puts ("tf2: mutex_unlock failed");
	  exit (1);
	}
      struct timespec ts = { .tv_sec = 0, .tv_nsec = 10000000 };
      nanosleep (&ts, NULL);
    }
}


static int
do_test (void)
{
  if (pthread_mutex_lock (&m1) != 0)
    {
      puts ("mutex_lock failed");
      exit (1);
    }

  setup_eintr (SIGUSR1, NULL);

  pthread_t th;
  char buf[100];
  int e = pthread_create (&th, NULL, tf1, NULL);
  if (e != 0)
    {
      printf ("main: 1st pthread_create failed: %s\n",
	      strerror_r (e, buf, sizeof (buf)));
      exit (1);
    }

  e = pthread_create (&th, NULL, tf2, NULL);
  if (e != 0)
    {
      printf ("main: 2nd pthread_create failed: %s\n",
	      strerror_r (e, buf, sizeof (buf)));
      exit (1);
    }

  /* This call must never return.  */
  e = pthread_mutex_lock (&m1);
  printf ("main: mutex_lock returned: %s\n",
	  strerror_r (e, buf, sizeof (buf)));

  return 0;
}

#define EXPECTED_SIGNAL SIGALRM
#define TIMEOUT 3
#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
