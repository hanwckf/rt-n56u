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
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/time.h>

int gettimeofday(struct timeval *tv, struct timezone *tz);


static int
do_test (void)
{
  size_t ps = sysconf (_SC_PAGESIZE);
  char tmpfname[] = "/tmp/tst-mutex9.XXXXXX";
  char data[ps];
  void *mem;
  int fd;
  pthread_mutex_t *m;
  pthread_mutexattr_t a;
  pid_t pid;
  char *p;

  fd = mkstemp (tmpfname);
  if (fd == -1)
    {
      printf ("cannot open temporary file: %m\n");
      return 1;
    }

  /* Make sure it is always removed.  */
  unlink (tmpfname);

  /* Create one page of data.  */
  memset (data, '\0', ps);

  /* Write the data to the file.  */
  if (write (fd, data, ps) != (ssize_t) ps)
    {
      puts ("short write");
      return 1;
    }

  mem = mmap (NULL, ps, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (mem == MAP_FAILED)
    {
      printf ("mmap failed: %m\n");
      return 1;
    }

  m = (pthread_mutex_t *) (((uintptr_t) mem + __alignof (pthread_mutex_t))
			   & ~(__alignof (pthread_mutex_t) - 1));
  p = (char *) (m + 1);

  if (pthread_mutexattr_init (&a) != 0)
    {
      puts ("mutexattr_init failed");
      return 1;
    }

  if (pthread_mutexattr_setpshared (&a, PTHREAD_PROCESS_SHARED) != 0)
    {
      puts ("mutexattr_setpshared failed");
      return 1;
    }

  if (pthread_mutexattr_settype (&a, PTHREAD_MUTEX_RECURSIVE) != 0)
    {
      puts ("mutexattr_settype failed");
      return 1;
    }

  if (pthread_mutex_init (m, &a) != 0)
    {
      puts ("mutex_init failed");
      return 1;
    }

  if (pthread_mutex_lock (m) != 0)
    {
      puts ("mutex_lock failed");
      return 1;
    }

  if (pthread_mutexattr_destroy (&a) != 0)
    {
      puts ("mutexattr_destroy failed");
      return 1;
    }

  puts ("going to fork now");
  pid = fork ();
  if (pid == -1)
    {
      puts ("fork failed");
      return 1;
    }
  else if (pid == 0)
    {
      if (pthread_mutex_trylock (m) == 0)
	{
	  puts ("child: mutex_trylock succeeded");
	  exit (1);
	}

      if (pthread_mutex_unlock (m) == 0)
	{
	  puts ("child: mutex_unlock succeeded");
	  exit (1);
	}

      struct timeval tv;
      gettimeofday (&tv, NULL);
      struct timespec ts;
      TIMEVAL_TO_TIMESPEC (&tv, &ts);
      ts.tv_nsec += 500000000;
      if (ts.tv_nsec >= 1000000000)
	{
	  ++ts.tv_sec;
	  ts.tv_nsec -= 1000000000;
	}

      int e = pthread_mutex_timedlock (m, &ts);
      if (e == 0)
	{
	  puts ("child: mutex_timedlock succeeded");
	  exit (1);
	}
      if (e != ETIMEDOUT)
	{
	  puts ("child: mutex_timedlock didn't time out");
	  exit (1);
	}

      alarm (1);

      pthread_mutex_lock (m);

      puts ("child: mutex_lock returned");

      exit (0);
    }

  sleep (2);

  int status;
  if (TEMP_FAILURE_RETRY (waitpid (pid, &status, 0)) != pid)
    {
      puts ("waitpid failed");
      return 1;
    }
  if (! WIFSIGNALED (status))
    {
      puts ("child not killed by signal");
      return 1;
    }
  if (WTERMSIG (status) != SIGALRM)
    {
      puts ("child not killed by SIGALRM");
      return 1;
    }

  return 0;
}

#define TIMEOUT 3
#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
