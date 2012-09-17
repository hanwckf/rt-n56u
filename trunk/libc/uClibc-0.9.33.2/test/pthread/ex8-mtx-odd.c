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

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

static int
do_test (void)
{

  if (pthread_mutex_lock (&lock) != 0)
    {
      puts ("mutex_lock failed");
      exit (1);
    }

  if (pthread_mutex_unlock (&lock) != 0)
    {
      puts ("mutex_unlock 1 failed");
      exit (1);
    }

  if (pthread_mutex_unlock (&lock) == 0)
    {
      puts ("mutex_unlock 2 succeded");
      exit (1);
    }

  return 0;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
