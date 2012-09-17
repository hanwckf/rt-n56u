/* Copyright (C) 2005, 2007, 2008 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2005.

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

#include <stdio.h>
#include <pthreadP.h>
#include <semaphore.h>

static const struct
{
  const char *name;
  size_t expected;
  size_t is;
} types[] =
  {
#define T(t, c) \
    { #t, c, sizeof (t) }
    T (pthread_attr_t, __SIZEOF_PTHREAD_ATTR_T),
    T (pthread_mutex_t, __SIZEOF_PTHREAD_MUTEX_T),
    T (pthread_mutexattr_t, __SIZEOF_PTHREAD_MUTEXATTR_T),
    T (pthread_cond_t, __SIZEOF_PTHREAD_COND_T),
    T (pthread_condattr_t, __SIZEOF_PTHREAD_CONDATTR_T),
    T (pthread_rwlock_t, __SIZEOF_PTHREAD_RWLOCK_T),
    T (pthread_rwlockattr_t, __SIZEOF_PTHREAD_RWLOCKATTR_T),
    T (pthread_barrier_t, __SIZEOF_PTHREAD_BARRIER_T),
    T (pthread_barrierattr_t, __SIZEOF_PTHREAD_BARRIERATTR_T)
  };

static int
do_test (void)
{
  int result = 0;

#define TEST_TYPE(name) \
  printf ("%s: ", #name);						      \
  if (sizeof (name) != sizeof (((name *) 0)->__size))			      \
    {									      \
      printf ("expected %zu, is %zu\n",					      \
	      sizeof (((name *) 0)->__size), sizeof (name));		      \
      result = 1;							      \
    }									      \
  else									      \
    puts ("OK")

  TEST_TYPE (pthread_mutex_t);
  TEST_TYPE (pthread_cond_t);
  TEST_TYPE (pthread_rwlock_t);

#define TEST_TYPE2(name, internal)					      \
  printf ("%s: ", #name);						      \
  if (sizeof (((name *) 0)->__size) < sizeof (internal))		      \
    {									      \
      printf ("expected %zu, is %zu\n",					      \
	      sizeof (((name *) 0)->__size), sizeof (internal));	      \
      result = 1;							      \
    }									      \
  else									      \
    puts ("OK")

  TEST_TYPE2 (pthread_attr_t, struct pthread_attr);
  TEST_TYPE2 (pthread_mutexattr_t, struct pthread_mutexattr);
  TEST_TYPE2 (pthread_condattr_t, struct pthread_condattr);
  TEST_TYPE2 (pthread_rwlockattr_t, struct pthread_rwlockattr);
  TEST_TYPE2 (pthread_barrier_t, struct pthread_barrier);
  TEST_TYPE2 (pthread_barrierattr_t, struct pthread_barrierattr);
  TEST_TYPE2 (sem_t, struct new_sem);
  TEST_TYPE2 (sem_t, struct old_sem);

  for (size_t i = 0; i < sizeof (types) / sizeof (types[0]); ++i)
    if (types[i].expected != types[i].is)
      {
	printf ("%s: expected %zu, is %zu\n",
		types[i].name, types[i].expected, types[i].is);
	result = 1;
      }

  return result;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
