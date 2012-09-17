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

#include "eintr.c"


static void *
tf2 (void *arg)
{
  return arg;
}


static void *
tf1 (void *arg)
{
  while (1)
    {
      pthread_t th;

      int e = pthread_create (&th, NULL, tf2, NULL);
      if (e != 0)
	{
	  if (e == EINTR)
	    {
	      puts ("pthread_create returned EINTR");
	      exit (1);
	    }

	  char buf[100];
	  printf ("tf1: pthread_create failed: %s\n",
		  strerror_r (e, buf, sizeof (buf)));
	  exit (1);
	}

      e = pthread_join (th, NULL);
      if (e != 0)
	{
	  if (e == EINTR)
	    {
	      puts ("pthread_join returned EINTR");
	      exit (1);
	    }

	  char buf[100];
	  printf ("tf1: pthread_join failed: %s\n",
		  strerror_r (e, buf, sizeof (buf)));
	  exit (1);
	}
    }
}


static int
do_test (void)
{
  setup_eintr (SIGUSR1, NULL);

  int i;
  for (i = 0; i < 10; ++i)
    {
      pthread_t th;
      int e = pthread_create (&th, NULL, tf1, NULL);
      if (e != 0)
	{
	  char buf[100];
	  printf ("main: pthread_create failed: %s\n",
		  strerror_r (e, buf, sizeof (buf)));
	  exit (1);
	}
    }

  (void) tf1 (NULL);
  /* NOTREACHED */

  return 0;
}

#define EXPECTED_SIGNAL SIGALRM
#define TIMEOUT 3
#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
