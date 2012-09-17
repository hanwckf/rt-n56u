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


void
__unregister_atfork (dso_handle)
     void *dso_handle;
{
  /* Get the lock to not conflict with running forks.  */
  __libc_lock_lock (__fork_block.lock);

  list_t *runp;
  list_t *prevp;

  list_for_each_prev_safe (runp, prevp, &__fork_block.prepare_list)
    if (list_entry (runp, struct fork_handler, list)->dso_handle == dso_handle)
      list_del (runp);

  list_for_each_prev_safe (runp, prevp, &__fork_block.parent_list)
    if (list_entry (runp, struct fork_handler, list)->dso_handle == dso_handle)
      list_del (runp);

  list_for_each_prev_safe (runp, prevp, &__fork_block.child_list)
    if (list_entry (runp, struct fork_handler, list)->dso_handle == dso_handle)
      list_del (runp);

  /* Release the lock.  */
  __libc_lock_unlock (__fork_block.lock);
}
