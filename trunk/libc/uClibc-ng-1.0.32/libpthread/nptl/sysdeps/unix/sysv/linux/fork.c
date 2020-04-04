/* Copyright (C) 2002, 2003, 2007, 2008 Free Software Foundation, Inc.
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

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sysdep.h>
#include <tls.h>
#include "fork.h"
#include <ldsodefs.h>
#include <atomic.h>
#include <errno.h>

#ifdef __ARCH_USE_MMU__
unsigned long int *__fork_generation_pointer;



/* The single linked list of all currently registered for handlers.  */
struct fork_handler *__fork_handlers;


static void
fresetlockfiles (void)
{
  FILE *fp;
#ifdef __USE_STDIO_FUTEXES__
  for (fp = _stdio_openlist; fp != NULL; fp = fp->__nextopen)
    STDIO_INIT_MUTEX(fp->__lock);
#else
  pthread_mutexattr_t attr;

  pthread_mutexattr_init(&attr);
  pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);

  for (fp = _stdio_openlist; fp != NULL; fp = fp->__nextopen)
    pthread_mutex_init(&fp->__lock, &attr);

  pthread_mutexattr_destroy(&attr);
#endif
}

pid_t
#if defined __arm__ && defined __thumb__ && __GNUC_PREREQ (4,6)
/* GCC PR target/53735
 * In thumb1 we run out of registers when compiling with Os so relax that
 * to have more registers available for spilling by using O2 here.
 */
attribute_optimize("O2")
#endif
fork (void)
{
  pid_t pid;
  struct used_handler
  {
    struct fork_handler *handler;
    struct used_handler *next;
  } *allp = NULL;

  /* Run all the registered preparation handlers.  In reverse order.
     While doing this we build up a list of all the entries.  */
  struct fork_handler *runp;
  while ((runp = __fork_handlers) != NULL)
    {
      /* Make sure we read from the current RUNP pointer.  */
      atomic_full_barrier ();

      unsigned int oldval = runp->refcntr;

      if (oldval == 0)
	/* This means some other thread removed the list just after
	   the pointer has been loaded.  Try again.  Either the list
	   is empty or we can retry it.  */
	continue;

      /* Bump the reference counter.  */
      if (atomic_compare_and_exchange_bool_acq (&__fork_handlers->refcntr,
						oldval + 1, oldval))
	/* The value changed, try again.  */
	continue;

      /* We bumped the reference counter for the first entry in the
	 list.  That means that none of the following entries will
	 just go away.  The unloading code works in the order of the
	 list.

         While executing the registered handlers we are building a
         list of all the entries so that we can go backward later on.  */
      while (1)
	{
	  /* Execute the handler if there is one.  */
	  if (runp->prepare_handler != NULL)
	    runp->prepare_handler ();

	  /* Create a new element for the list.  */
	  struct used_handler *newp
	    = (struct used_handler *) alloca (sizeof (*newp));
	  newp->handler = runp;
	  newp->next = allp;
	  allp = newp;

	  /* Advance to the next handler.  */
	  runp = runp->next;
	  if (runp == NULL)
	    break;

	  /* Bump the reference counter for the next entry.  */
	  atomic_increment (&runp->refcntr);
	}

      /* We are done.  */
      break;
    }

  __UCLIBC_IO_MUTEX_LOCK_CANCEL_UNSAFE(_stdio_openlist_add_lock);

#ifndef NDEBUG
  pid_t ppid = THREAD_GETMEM (THREAD_SELF, tid);
#endif

#ifdef ARCH_FORK
  pid = ARCH_FORK ();
#else
# error "ARCH_FORK must be defined so that the CLONE_SETTID flag is used"
  pid = INLINE_SYSCALL (fork, 0);
#endif


  if (pid == 0)
    {
      assert (THREAD_GETMEM (THREAD_SELF, tid) != ppid);

      if (__fork_generation_pointer != NULL)
	*__fork_generation_pointer += 4;

      /* Reset the file list.  These are recursive mutexes.  */
      fresetlockfiles ();

      /* Reset locks in the I/O code.  */
      STDIO_INIT_MUTEX(_stdio_openlist_add_lock);

      /* XXX reset any locks in dynamic loader */

      /* Run the handlers registered for the child.  */
      while (allp != NULL)
	{
	  if (allp->handler->child_handler != NULL)
	    allp->handler->child_handler ();

	  /* Note that we do not have to wake any possible waiter.
 	     This is the only thread in the new process.  The count
 	     may have been bumped up by other threads doing a fork.
 	     We reset it to 1, to avoid waiting for non-existing
 	     thread(s) to release the count.  */
	  allp->handler->refcntr = 1;

	  /* XXX We could at this point look through the object pool
	     and mark all objects not on the __fork_handlers list as
	     unused.  This is necessary in case the fork() happened
	     while another thread called dlclose() and that call had
	     to create a new list.  */

	  allp = allp->next;
	}

      /* Initialize the fork lock.  */
      __fork_lock = LLL_LOCK_INITIALIZER;
    }
  else
    {
      assert (THREAD_GETMEM (THREAD_SELF, tid) == ppid);

      /* We execute this even if the 'fork' call failed.  */
      __UCLIBC_IO_MUTEX_UNLOCK_CANCEL_UNSAFE(_stdio_openlist_add_lock);

      /* Run the handlers registered for the parent.  */
      while (allp != NULL)
	{
	  if (allp->handler->parent_handler != NULL)
	    allp->handler->parent_handler ();

	  if (atomic_decrement_and_test (&allp->handler->refcntr)
	      && allp->handler->need_signal)
	    lll_futex_wake (allp->handler->refcntr, 1, LLL_PRIVATE);

	  allp = allp->next;
	}
    }

  return pid;
}
libc_hidden_def(fork)

#endif /* __ARCH_USE_MMU__ */
