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


static int
do_test (void)
{
  pthread_t self = pthread_self ();

  setup_eintr (SIGUSR1, &self);

  pthread_barrier_t b;
  if (pthread_barrier_init (&b, NULL, 2) != 0)
    {
      puts ("barrier_init failed");
      exit (1);
    }

  /* This call must never return.  */
  int e = pthread_barrier_wait (&b);

  if (e == EINTR)
    puts ("pthread_join returned with EINTR");

  return 0;
}

#define EXPECTED_SIGNAL SIGALRM
#define TIMEOUT 1
#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
