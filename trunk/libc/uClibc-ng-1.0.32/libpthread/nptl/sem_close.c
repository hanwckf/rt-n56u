/* Copyright (C) 2002, 2003 Free Software Foundation, Inc.
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
#include <search.h>
#include <sys/mman.h>
#include "semaphoreP.h"


/* Global variables to parametrize the walk function.  This works
   since we always have to use locks.  And we have to use the twalk
   function since the entries are not sorted wrt the mapping
   address.  */
static sem_t *the_sem;
static struct inuse_sem *rec;

static void
walker (const void *inodep, const VISIT which, const int depth)
{
  struct inuse_sem *nodep = *(struct inuse_sem **) inodep;

  if (nodep->sem == the_sem)
    rec = nodep;
}


int
sem_close (
     sem_t *sem)
{
  int result = 0;

  /* Get the lock.  */
  lll_lock (__sem_mappings_lock, LLL_PRIVATE);

  /* Locate the entry for the mapping the caller provided.  */
  rec = NULL;
  the_sem = sem;
  twalk (__sem_mappings, walker);
  if  (rec != NULL)
    {
      /* Check the reference counter.  If it is going to be zero, free
	 all the resources.  */
      if (--rec->refcnt == 0)
	{
	  /* Remove the record from the tree.  */
	  (void) tdelete (rec, &__sem_mappings, __sem_search);

	  result = munmap (rec->sem, sizeof (sem_t));

	  free (rec);
	}
    }
  else
    {
      /* This is no valid semaphore.  */
      result = -1;
      __set_errno (EINVAL);
    }

  /* Release the lock.  */
  lll_unlock (__sem_mappings_lock, LLL_PRIVATE);

  return result;
}
