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

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <tls.h>

#if HAVE___THREAD
struct test_s
{
  int a;
  int b;
};

#define INIT_A 1
#define INIT_B 42
/* Deliberately not static.  */
__thread struct test_s s __attribute__ ((tls_model ("initial-exec"))) =
{
  .a = INIT_A,
  .b = INIT_B
};


static void *
tf (void *arg)
{
  if (s.a != INIT_A || s.b != INIT_B)
    {
      puts ("initial value of s in child thread wrong");
      exit (1);
    }

  ++s.a;

  return NULL;
}
#endif


int
do_test (void)
{
#if !HAVE___THREAD

  puts ("No __thread support in compiler, test skipped.");

  return 0;
#else

  if (s.a != INIT_A || s.b != INIT_B)
    {
      puts ("initial value of s in main thread wrong");
      exit (1);
    }

  pthread_attr_t a;

  if (pthread_attr_init (&a) != 0)
    {
      puts ("attr_init failed");
      exit (1);
    }

  if (pthread_attr_setstacksize (&a, 1 * 1024 * 1024) != 0)
    {
      puts ("attr_setstacksize failed");
      return 1;
    }

#define N 10
  int i;
  for (i = 0; i < N; ++i)
    {
#define M 10
      pthread_t th[M];
      int j;
      for (j = 0; j < M; ++j, ++s.a)
	if (pthread_create (&th[j], &a, tf, NULL) != 0)
	  {
	    puts ("pthread_create failed");
	    exit (1);
	  }

      for (j = 0; j < M; ++j)
	if (pthread_join (th[j], NULL) != 0)
	  {
	    puts ("pthread_join failed");
	    exit (1);
	  }
    }

  if (pthread_attr_destroy (&a) != 0)
    {
      puts ("attr_destroy failed");
      exit (1);
    }

  return 0;
#endif
}


#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
