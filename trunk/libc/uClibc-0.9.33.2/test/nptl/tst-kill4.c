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
#include <unistd.h>


static void *
tf (void *a)
{
  return NULL;
}


int
do_test (void)
{
  pthread_t th;
  if (pthread_create (&th, NULL, tf, NULL) != 0)
    {
      puts ("create failed");
      exit (1);
    }

  if (pthread_join (th, NULL) != 0)
    {
      puts ("join failed");
      exit (1);
    }

  /* The following only works because we assume here something about
     the implementation.  Namely, that the memory allocated for the
     thread descriptor is not going away, that the the TID field is
     cleared and therefore the signal is sent to process 0, and that
     we can savely assume there is no other process with this ID at
     that time.  */
  int e = pthread_kill (th, 0);
  if (e == 0)
    {
      puts ("pthread_kill succeeded");
      exit (1);
    }
  if (e != ESRCH)
    {
      puts ("pthread_kill didn't return ESRCH");
      exit (1);
    }

  return 0;
}


#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
