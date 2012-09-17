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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <fork.h>
#include <atomic.h>
#include <tls.h>


/* Lock to protect allocation and deallocation of fork handlers.  */
int __fork_lock = LLL_LOCK_INITIALIZER;


/* Number of pre-allocated handler entries.  */
#define NHANDLER 48

/* Memory pool for fork handler structures.  */
static struct fork_handler_pool
{
  struct fork_handler_pool *next;
  struct fork_handler mem[NHANDLER];
} fork_handler_pool;


static struct fork_handler *
fork_handler_alloc (void)
{
  struct fork_handler_pool *runp = &fork_handler_pool;
  struct fork_handler *result = NULL;
  unsigned int i;

  do
    {
      /* Search for an empty entry.  */
      for (i = 0; i < NHANDLER; ++i)
	if (runp->mem[i].refcntr == 0)
	  goto found;
    }
  while ((runp = runp->next) != NULL);

  /* We have to allocate a new entry.  */
  runp = (struct fork_handler_pool *) calloc (1, sizeof (*runp));
  if (runp != NULL)
    {
      /* Enqueue the new memory pool into the list.  */
      runp->next = fork_handler_pool.next;
      fork_handler_pool.next = runp;

      /* We use the last entry on the page.  This means when we start
	 searching from the front the next time we will find the first
	 entry unused.  */
      i = NHANDLER - 1;

    found:
      result = &runp->mem[i];
      result->refcntr = 1;
      result->need_signal = 0;
    }

  return result;
}


int
__register_atfork (
     void (*prepare) (void),
     void (*parent) (void),
     void (*child) (void),
     void *dso_handle)
{
  /* Get the lock to not conflict with other allocations.  */
  lll_lock (__fork_lock, LLL_PRIVATE);

  struct fork_handler *newp = fork_handler_alloc ();

  if (newp != NULL)
    {
      /* Initialize the new record.  */
      newp->prepare_handler = prepare;
      newp->parent_handler = parent;
      newp->child_handler = child;
      newp->dso_handle = dso_handle;

      __linkin_atfork (newp);
    }

  /* Release the lock.  */
  lll_unlock (__fork_lock, LLL_PRIVATE);

  return newp == NULL ? ENOMEM : 0;
}
libc_hidden_def (__register_atfork)


void
attribute_hidden
__linkin_atfork (struct fork_handler *newp)
{
  do
    newp->next = __fork_handlers;
  while (catomic_compare_and_exchange_bool_acq (&__fork_handlers,
						newp, newp->next) != 0);
}

#if 0
libc_freeres_fn (free_mem)
{
  /* Get the lock to not conflict with running forks.  */
  lll_lock (__fork_lock, LLL_PRIVATE);

  /* No more fork handlers.  */
  __fork_handlers = NULL;

  /* Free eventually alloated memory blocks for the object pool.  */
  struct fork_handler_pool *runp = fork_handler_pool.next;

  memset (&fork_handler_pool, '\0', sizeof (fork_handler_pool));

  /* Release the lock.  */
  lll_unlock (__fork_lock, LLL_PRIVATE);

  /* We can free the memory after releasing the lock.  */
  while (runp != NULL)
    {
      struct fork_handler_pool *oldp = runp;
      runp = runp->next;
      free (oldp);
    }
}
#endif

