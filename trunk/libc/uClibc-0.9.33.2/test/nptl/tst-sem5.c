/* Copyright (C) 2002, 2003 Free Software Foundation, Inc.
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
#include <semaphore.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>


static int
do_test (void)
{
  sem_t s;
  struct timespec ts;
  struct timeval tv;

  if (sem_init (&s, 0, 1) == -1)
    {
      puts ("sem_init failed");
      return 1;
    }

  if (TEMP_FAILURE_RETRY (sem_wait (&s)) == -1)
    {
      puts ("sem_wait failed");
      return 1;
    }

  if (gettimeofday (&tv, NULL) != 0)
    {
      puts ("gettimeofday failed");
      return 1;
    }

  TIMEVAL_TO_TIMESPEC (&tv, &ts);

  /* We wait for half a second.  */
  ts.tv_nsec += 500000000;
  if (ts.tv_nsec >= 1000000000)
    {
      ++ts.tv_sec;
      ts.tv_nsec -= 1000000000;
    }

  errno = 0;
  if (TEMP_FAILURE_RETRY (sem_timedwait (&s, &ts)) != -1)
    {
      puts ("sem_timedwait succeeded");
      return 1;
    }
  if (errno != ETIMEDOUT)
    {
      printf ("sem_timedwait return errno = %d instead of ETIMEDOUT\n",
	      errno);
      return 1;
    }

  return 0;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
