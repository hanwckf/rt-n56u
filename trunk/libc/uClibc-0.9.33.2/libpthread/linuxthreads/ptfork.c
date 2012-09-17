/* Linuxthreads - a simple clone()-based implementation of Posix        */
/* threads for Linux.                                                   */
/* Copyright (C) 1996 Xavier Leroy (Xavier.Leroy@inria.fr)              */
/*                                                                      */
/* This program is free software; you can redistribute it and/or        */
/* modify it under the terms of the GNU Library General Public License  */
/* as published by the Free Software Foundation; either version 2       */
/* of the License, or (at your option) any later version.               */
/*                                                                      */
/* This program is distributed in the hope that it will be useful,      */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
/* GNU Library General Public License for more details.                 */

/* The "atfork" stuff */

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include "pthread.h"
#include "internals.h"
#include <bits/libc-lock.h>
#include "fork.h"

extern int __libc_fork (void);

pid_t __pthread_fork (struct fork_block *b)
{
  pid_t pid;
  list_t *runp;

  __libc_lock_lock (b->lock);

  /* Run all the registered preparation handlers.  In reverse order.  */
  list_for_each_prev (runp, &b->prepare_list)
    {
      struct fork_handler *curp;
      curp = list_entry (runp, struct fork_handler, list);
      curp->handler ();
    }

  __pthread_once_fork_prepare();
  __flockfilelist();

  pid = ARCH_FORK ();

  if (pid == 0) {
    __pthread_reset_main_thread();

    __fresetlockfiles();
    __pthread_once_fork_child();

    /* Run the handlers registered for the child.  */
    list_for_each (runp, &b->child_list)
      {
	struct fork_handler *curp;
	curp = list_entry (runp, struct fork_handler, list);
	curp->handler ();
      }

    __libc_lock_init (b->lock);
  } else {
    __funlockfilelist();
    __pthread_once_fork_parent();

    /* Run the handlers registered for the parent.  */
    list_for_each (runp, &b->parent_list)
      {
	struct fork_handler *curp;
	curp = list_entry (runp, struct fork_handler, list);
	curp->handler ();
      }

    __libc_lock_unlock (b->lock);
  }

  return pid;
}

/* psm: have no idea why these are here, sjhill? */
#if 0 /*def SHARED*/
pid_t __fork (void)
{
  return __libc_fork ();
}
weak_alias (__fork, fork)

pid_t __vfork(void)
{
  return __libc_fork ();
}
weak_alias (__vfork, vfork)
#endif
