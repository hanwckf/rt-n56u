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


static void *
tf (void *arg)
{
  pthread_key_t *key = (pthread_key_t *) arg;

  if (pthread_setspecific (*key, (void *) -1l) != 0)
    {
      write (2, "setspecific failed\n", 19);
      _exit (1);
    }

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

  pthread_t th;
  if (pthread_create (&th, NULL, tf, &keys[1]) != 0)
    {
      write (2, "create failed\n", 14);
      _exit (1);
    }

  if (pthread_join (th, NULL) != 0)
    {
      write (2, "join failed\n", 12);
      _exit (1);
    }

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

  return 0;
}


#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
