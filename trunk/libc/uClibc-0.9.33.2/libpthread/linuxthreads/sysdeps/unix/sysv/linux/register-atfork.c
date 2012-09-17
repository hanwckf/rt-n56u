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

#include <errno.h>
#include <stdlib.h>
#include "fork.h"


int
__register_atfork (prepare, parent, child, dso_handle)
     void (*prepare) (void);
     void (*parent) (void);
     void (*child) (void);
     void *dso_handle;
{
  struct fork_handler *new_prepare = NULL;
  struct fork_handler *new_parent = NULL;
  struct fork_handler *new_child = NULL;

  if (prepare != NULL)
    {
      new_prepare = (struct fork_handler *) malloc (sizeof (*new_prepare));
      if (new_prepare == NULL)
	goto out1;

      new_prepare->handler = prepare;
      new_prepare->dso_handle = dso_handle;
    }

  if (parent != NULL)
    {
      new_parent = (struct fork_handler *) malloc (sizeof (*new_parent));
      if (new_parent == NULL)
	goto out2;

      new_parent->handler = parent;
      new_parent->dso_handle = dso_handle;
    }

  if (child != NULL)
    {
      new_child = (struct fork_handler *) malloc (sizeof (*new_child));
      if (new_child == NULL)
	{
	  free (new_parent);
	out2:
	  free (new_prepare);
	out1:
	  return errno;
	}

      new_child->handler = child;
      new_child->dso_handle = dso_handle;
    }

  /* Get the lock to not conflict with running forks.  */
  __libc_lock_lock (__fork_block.lock);

  /* Now that we have all the handlers allocate enqueue them.  */
  if (new_prepare != NULL)
    list_add_tail (&new_prepare->list, &__fork_block.prepare_list);
  if (new_parent != NULL)
    list_add_tail (&new_parent->list, &__fork_block.parent_list);
  if (new_child != NULL)
    list_add_tail (&new_child->list, &__fork_block.child_list);

  /* Release the lock.  */
  __libc_lock_unlock (__fork_block.lock);

  return 0;
}
