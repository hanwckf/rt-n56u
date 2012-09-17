/* Copyright (C) 2002, 2003, 2004 Free Software Foundation, Inc.
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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>


static int
do_test (void)
{
  size_t ps = sysconf (_SC_PAGESIZE);
  char tmpfname[] = "/tmp/tst-mutex4.XXXXXX";
  char data[ps];
  void *mem;
  int fd;
  pthread_mutex_t *m;
  pthread_mutexattr_t a;
  pid_t pid;
  char *p;
  int err;
  int s;

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

  if (pthread_mutexattr_getpshared (&a, &s) != 0)
    {
      puts ("1st mutexattr_getpshared failed");
      return 1;
    }

  if (s != PTHREAD_PROCESS_PRIVATE)
    {
      puts ("default pshared value wrong");
      return 1;
    }

  if (pthread_mutexattr_setpshared (&a, PTHREAD_PROCESS_SHARED) != 0)
    {
      puts ("mutexattr_setpshared failed");
      return 1;
    }

  if (pthread_mutexattr_getpshared (&a, &s) != 0)
    {
      puts ("2nd mutexattr_getpshared failed");
      return 1;
    }

  if (s != PTHREAD_PROCESS_SHARED)
    {
      puts ("pshared value after setpshared call wrong");
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

  err = pthread_mutex_trylock (m);
  if (err == 0)
    {
      puts ("mutex_trylock succeeded");
      return 1;
    }
  else if (err != EBUSY)
    {
      puts ("mutex_trylock didn't return EBUSY");
      return 1;
    }

  *p = 0;

  puts ("going to fork now");
  pid = fork ();
  if (pid == -1)
    {
      puts ("fork failed");
      return 1;
    }
  else if (pid == 0)
    {
      /* Play some lock ping-pong.  It's our turn to unlock first.  */
      if ((*p)++ != 0)
	{
	  puts ("child: *p != 0");
	  return 1;
	}

      if (pthread_mutex_unlock (m) != 0)
	{
	  puts ("child: 1st mutex_unlock failed");
	  return 1;
	}

      puts ("child done");
    }
  else
    {
      if (pthread_mutex_lock (m) != 0)
	{
	  puts ("parent: 2nd mutex_lock failed");
	  return 1;
	}

      if (*p != 1)
	{
	  puts ("*p != 1");
	  return 1;
	}

      puts ("parent done");
    }

  return 0;
}

#define TIMEOUT 4
#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
