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


int
do_test (void)
{
  /* XXX This test might require architecture and system specific changes.
     There is no guarantee that this signal number is invalid.  */
  int e = pthread_kill (pthread_self (), SIGRTMAX + 10);
  if (e == 0)
    {
      puts ("kill didn't failed");
      exit (1);
    }
  if (e != EINVAL)
    {
      puts ("error not EINVAL");
      exit (1);
    }

  return 0;
}


#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
