/* Copyright (C) 2004-2013 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include "pthreadP.h"
#include <lowlevellock.h>

unsigned long int __fork_generation attribute_hidden;

static void
clear_once_control (void *arg)
{
  pthread_once_t *once_control = (pthread_once_t *) arg;

  *once_control = 0;
  lll_futex_wake (once_control, INT_MAX, LLL_PRIVATE);
}

int
__pthread_once (pthread_once_t *once_control, void (*init_routine) (void))
{
  for (;;)
    {
      int oldval;
      int newval;

      /* Pseudo code:
	 newval = __fork_generation | 1;
	 oldval = *once_control;
	 if ((oldval & 2) == 0)
	   *once_control = newval;
	 Do this atomically.
      */
      do
	{
	  newval = __fork_generation | 1;
	  oldval = *once_control;
	  if (oldval & 2)
	    break;
	} while (atomic_compare_and_exchange_val_acq (once_control, newval, oldval) != oldval);

      /* Check if the initializer has already been done.  */
      if ((oldval & 2) != 0)
	return 0;

      /* Check if another thread already runs the initializer.	*/
      if ((oldval & 1) == 0)
	break;

      /* Check whether the initializer execution was interrupted by a fork.  */
      if (oldval != newval)
	break;

      /* Same generation, some other thread was faster. Wait.  */
      lll_futex_wait (once_control, oldval, LLL_PRIVATE);
    }

  /* This thread is the first here.  Do the initialization.
     Register a cleanup handler so that in case the thread gets
     interrupted the initialization can be restarted.  */
  pthread_cleanup_push (clear_once_control, once_control);

  init_routine ();

  pthread_cleanup_pop (0);

  /* Say that the initialisation is done.  */
  *once_control = __fork_generation | 2;

  /* Wake up all other threads.  */
  lll_futex_wake (once_control, INT_MAX, LLL_PRIVATE);

  return 0;
}
weak_alias (__pthread_once, pthread_once)
strong_alias (__pthread_once, __pthread_once_internal)
