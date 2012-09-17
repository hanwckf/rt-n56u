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
#include <unistd.h>


static int
do_test (void)
{
  pthread_spinlock_t s;

  if (pthread_spin_init (&s, PTHREAD_PROCESS_PRIVATE) != 0)
    {
      puts ("spin_init failed");
      return 1;
    }

  if (pthread_spin_lock (&s) != 0)
    {
      puts ("1st spin_lock failed");
      return 1;
    }

  /* Set an alarm for 1 second.  The wrapper will expect this.  */
  alarm (1);

  /* This call should never return.  */
  pthread_spin_lock (&s);

  puts ("2nd spin_lock returned");
  return 1;
}

#define EXPECTED_SIGNAL SIGALRM
#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
