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

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#define N 2


static int cnt0;
static void
f0 (void *p)
{
  ++cnt0;
}


static int cnt1;
static void
f1 (void *p)
{
  ++cnt1;
}


static void (*fcts[N]) (void *) =
{
  f0,
  f1
};


static pthread_barrier_t b;


static void *
tf (void *arg)
{
  pthread_key_t *key = (pthread_key_t *) arg;

  if (pthread_setspecific (*key, (void *) -1l) != 0)
    {
      write (2, "setspecific failed\n", 19);
      _exit (1);
    }

  pthread_barrier_wait (&b);

  const struct timespec t = { .tv_sec = 1000, .tv_nsec = 0 };
  while (1)
    nanosleep (&t, NULL);

  /* NOTREACHED */
  return NULL;
}


int
do_test (void)
{
  pthread_key_t keys[N];

  int i;
  for (i = 0; i < N; ++i)
    if (pthread_key_create (&keys[i], fcts[i]) != 0)
      {
	write (2, "key_create failed\n", 18);
	_exit (1);
      }

  if (pthread_barrier_init (&b, NULL, 2) != 0)
    {
      write (2, "barrier_init failed\n", 20);
      _exit (1);
    }

  pthread_t th;
  if (pthread_create (&th, NULL, tf, &keys[1]) != 0)
    {
      write (2, "create failed\n", 14);
      _exit (1);
    }

  pthread_barrier_wait (&b);

  if (pthread_cancel (th) != 0)
    {
      write (2, "cancel failed\n", 14);
      _exit (1);
    }

  void *status;
  if (pthread_join (th, &status) != 0)
    {
      write (2, "join failed\n", 12);
      _exit (1);
    }

  if (status != PTHREAD_CANCELED)
    {
      write (2, "thread not canceled\n", 20);
      _exit (1);
    }

  /* Note that the TSD destructors not necessarily have to have
     finished by the time pthread_join returns.  At least according to
     POSIX.  We implement the stronger requirement that they indeed
     have run and therefore these tests succeed.  */
  if (cnt0 != 0)
    {
      write (2, "cnt0 != 0\n", 10);
      _exit (1);
    }

  if (cnt1 != 1)
    {
      write (2, "cnt1 != 1\n", 10);
      _exit (1);
    }

  for (i = 0; i < N; ++i)
    if (pthread_key_delete (keys[i]) != 0)
      {
	write (2, "key_delete failed\n", 18);
	_exit (1);
      }

  if (pthread_barrier_destroy (&b) != 0)
    {
      write (2, "barrier_destroy failed\n", 23);
      _exit (1);
    }

  return 0;
}


#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
