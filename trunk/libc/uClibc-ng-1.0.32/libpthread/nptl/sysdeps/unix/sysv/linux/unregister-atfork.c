/* Copyright (C) 2002, 2003, 2005, 2007 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <stdlib.h>
#include <fork.h>
#include <atomic.h>
#include <tls.h>

#ifdef __ARCH_USE_MMU__
void
__unregister_atfork (
     void *dso_handle)
{
  /* Check whether there is any entry in the list which we have to
     remove.  It is likely that this is not the case so don't bother
     getting the lock.

     We do not worry about other threads adding entries for this DSO
     right this moment.  If this happens this is a race and we can do
     whatever we please.  The program will crash anyway seen.  */
  struct fork_handler *runp = __fork_handlers;
  struct fork_handler *lastp = NULL;

  while (runp != NULL)
    if (runp->dso_handle == dso_handle)
      break;
    else
      {
	lastp = runp;
	runp = runp->next;
      }

  if (runp == NULL)
    /* Nothing to do.  */
    return;

  /* Get the lock to not conflict with additions or deletions.  Note
     that there couldn't have been another thread deleting something.
     The __unregister_atfork function is only called from the
     dlclose() code which itself serializes the operations.  */
  lll_lock (__fork_lock, LLL_PRIVATE);

  /* We have to create a new list with all the entries we don't remove.  */
  struct deleted_handler
  {
    struct fork_handler *handler;
    struct deleted_handler *next;
  } *deleted = NULL;

  /* Remove the entries for the DSO which is unloaded from the list.
     It's a single linked list so readers are.  */
  do
    {
    again:
      if (runp->dso_handle == dso_handle)
	{
	  if (lastp == NULL)
	    {
	      /* We have to use an atomic operation here because
		 __linkin_atfork also uses one.  */
	      if (catomic_compare_and_exchange_bool_acq (&__fork_handlers,
							 runp->next, runp)
		  != 0)
		{
		  runp = __fork_handlers;
		  goto again;
		}
	    }
	  else
	    lastp->next = runp->next;

	  /* We cannot overwrite the ->next element now.  Put the deleted
	     entries in a separate list.  */
	  struct deleted_handler *newp = alloca (sizeof (*newp));
	  newp->handler = runp;
	  newp->next = deleted;
	  deleted = newp;
	}
      else
	lastp = runp;

      runp = runp->next;
    }
  while (runp != NULL);

  /* Release the lock.  */
  lll_unlock (__fork_lock, LLL_PRIVATE);

  /* Walk the list of all entries which have to be deleted.  */
  while (deleted != NULL)
    {
      /* We need to be informed by possible current users.  */
      deleted->handler->need_signal = 1;
      /* Make sure this gets written out first.  */
      atomic_write_barrier ();

      /* Decrement the reference counter.  If it does not reach zero
	 wait for the last user.  */
      atomic_decrement (&deleted->handler->refcntr);
      unsigned int val;
      while ((val = deleted->handler->refcntr) != 0)
	lll_futex_wait (&deleted->handler->refcntr, val, LLL_PRIVATE);

      deleted = deleted->next;
    }
}
#else
void
__unregister_atfork (
     void *dso_handle)
{
    /* Nothing to do.  */
}
#endif
