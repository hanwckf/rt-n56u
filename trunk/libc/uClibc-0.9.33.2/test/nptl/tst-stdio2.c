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
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


static void *tf (void *a)
{
  puts ("start tf");

  /* Multiple locking, implicitly or explicitly, must be possible.  */
  flockfile (stdout);

  puts ("after first flockfile");

  flockfile (stdout);

  puts ("foo");

  funlockfile (stdout);

  puts ("after first funlockfile");

  funlockfile (stdout);

  puts ("all done");

  return a;
}


int
do_test (void)
{
  pthread_t th;

  if (pthread_create (&th, NULL, tf, NULL) != 0)
    {
      write (2, "create failed\n", 14);
      _exit (1);
    }

  void *result;
  if (pthread_join (th, &result) != 0)
    {
      puts ("join failed");
      exit (1);
    }
  else if (result != NULL)
    {
      printf ("wrong return value: %p, expected %p\n", result, NULL);
      exit (1);
    }

  puts ("join returned succsefully");

  return 0;
}


#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
