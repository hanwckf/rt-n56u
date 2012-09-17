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
#include <stdio.h>
#include <stdlib.h>


static void *
tf1 (void *arg)
{
  pthread_join ((pthread_t) arg, NULL);

  puts ("1st join returned");

  return (void *) 1l;
}


static void *
tf2 (void *arg)
{
  int a;
  a = pthread_join ((pthread_t) arg, NULL);

  puts ("2nd join returned");
  printf("a = %i\n", a);

  return (void *) 1l;
}


static int
do_test (void)
{
  pthread_t th;

  int err = pthread_join (pthread_self (), NULL);
  if (err == 0)
    {
      puts ("1st circular join succeeded");
      exit (1);
    }
  if (err != EDEADLK)
    {
      printf ("1st circular join %d, not EDEADLK\n", err);
      exit (1);
    }

  if (pthread_create (&th, NULL, tf1, (void *) pthread_self ()) != 0)
    {
      puts ("1st create failed");
      exit (1);
    }

  if (pthread_cancel (th) != 0)
    {
      puts ("cannot cancel 1st thread");
      exit (1);
    }

  void *r;
  err = pthread_join (th, &r);
  if (err != 0)
    {
      printf ("cannot join 1st thread: %d\n", err);
      exit (1);
    }
  if (r != PTHREAD_CANCELED)
    {
      puts ("1st thread not canceled");
      exit (1);
    }

  err = pthread_join (pthread_self (), NULL);
  if (err == 0)
    {
      puts ("2nd circular join succeeded");
      exit (1);
    }
  if (err != EDEADLK)
    {
      printf ("2nd circular join %d, not EDEADLK\n", err);
      exit (1);
    }

  if (pthread_create (&th, NULL, tf2, (void *) pthread_self ()) != 0)
    {
      puts ("2nd create failed");
      exit (1);
    }

  if (pthread_cancel (th) != 0)
    {
      puts ("cannot cancel 2nd thread");
      exit (1);
    }

  if (pthread_join (th, &r) != 0)
    {
      puts ("cannot join 2nd thread");
      exit (1);
    }
  if (r != PTHREAD_CANCELED)
    {
      puts ("2nd thread not canceled");
      exit (1);
    }

  err = pthread_join (pthread_self (), NULL);
  if (err == 0)
    {
      puts ("2nd circular join succeeded");
      exit (1);
    }
  if (err != EDEADLK)
    {
      printf ("2nd circular join %d, not EDEADLK\n", err);
      exit (1);
    }

  exit (0);
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
