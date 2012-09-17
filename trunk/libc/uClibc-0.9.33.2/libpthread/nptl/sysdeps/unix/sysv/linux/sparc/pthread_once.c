/* Copyright (C) 2003, 2004, 2007 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2003.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

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
attribute_protected
__pthread_once (once_control, init_routine)
     pthread_once_t *once_control;
     void (*init_routine) (void);
{
  while (1)
    {
      int oldval, val, newval;

      val = *once_control;
      do
	{
	  /* Check if the initialized has already been done.  */
	  if ((val & 2) != 0)
	    return 0;

	  oldval = val;
	  newval = (oldval & 3) | __fork_generation | 1;
	  val = atomic_compare_and_exchange_val_acq (once_control, newval,
						     oldval);
	}
      while (__builtin_expect (val != oldval, 0));

      /* Check if another thread already runs the initializer.	*/
      if ((oldval & 1) != 0)
	{
	  /* Check whether the initializer execution was interrupted
	     by a fork.	 */
	  if (((oldval ^ newval) & -4) == 0)
	    {
	      /* Same generation, some other thread was faster. Wait.  */
	      lll_futex_wait (once_control, newval, LLL_PRIVATE);
	      continue;
	    }
	}

      /* This thread is the first here.  Do the initialization.
	 Register a cleanup handler so that in case the thread gets
	 interrupted the initialization can be restarted.  */
      pthread_cleanup_push (clear_once_control, once_control);

      init_routine ();

      pthread_cleanup_pop (0);


      /* Add one to *once_control.  */
      atomic_increment (once_control);

      /* Wake up all other threads.  */
      lll_futex_wake (once_control, INT_MAX, LLL_PRIVATE);
      break;
    }

  return 0;
}
weak_alias (__pthread_once, pthread_once)
strong_alias (__pthread_once, __pthread_once_internal)
