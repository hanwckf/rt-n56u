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

/* mods for uClibc: removed strong alias and defined funcs properly */

/* The "atfork" stuff */

#include <errno.h>

#ifdef __ARCH_HAS_MMU__

#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include "pthread.h"
#include "internals.h"

#warning hack alert... should be sufficent for system(), but what about other libc mutexes?
#include <bits/uClibc_mutex.h>

__UCLIBC_MUTEX_EXTERN(__malloc_lock);

#define __MALLOC_LOCK		__UCLIBC_MUTEX_LOCK(__malloc_lock)
#define __MALLOC_UNLOCK		__UCLIBC_MUTEX_UNLOCK(__malloc_lock)
#warning hack alert block end

struct handler_list {
  void (*handler)(void);
  struct handler_list * next;
};

static pthread_mutex_t pthread_atfork_lock = PTHREAD_MUTEX_INITIALIZER;
static struct handler_list * pthread_atfork_prepare = NULL;
static struct handler_list * pthread_atfork_parent = NULL;
static struct handler_list * pthread_atfork_child = NULL;

static void pthread_insert_list(struct handler_list ** list,
                                void (*handler)(void),
                                struct handler_list * newlist,
                                int at_end)
{
  if (handler == NULL) return;
  if (at_end) {
    while(*list != NULL) list = &((*list)->next);
  }
  newlist->handler = handler;
  newlist->next = *list;
  *list = newlist;
}

struct handler_list_block {
  struct handler_list prepare, parent, child;
};

int pthread_atfork(void (*prepare)(void),
		     void (*parent)(void),
		     void (*child)(void))
{
  struct handler_list_block * block =
    (struct handler_list_block *) malloc(sizeof(struct handler_list_block));
  if (block == NULL) return ENOMEM;
  pthread_mutex_lock(&pthread_atfork_lock);
  /* "prepare" handlers are called in LIFO */
  pthread_insert_list(&pthread_atfork_prepare, prepare, &block->prepare, 0);
  /* "parent" handlers are called in FIFO */
  pthread_insert_list(&pthread_atfork_parent, parent, &block->parent, 1);
  /* "child" handlers are called in FIFO */
  pthread_insert_list(&pthread_atfork_child, child, &block->child, 1);
  pthread_mutex_unlock(&pthread_atfork_lock);
  return 0;
}
//strong_alias (__pthread_atfork, pthread_atfork)

static inline void pthread_call_handlers(struct handler_list * list)
{
  for (/*nothing*/; list != NULL; list = list->next) (list->handler)();
}

extern int __libc_fork(void);

pid_t __fork(void)
{
  pid_t pid;
  struct handler_list * prepare, * child, * parent;

  pthread_mutex_lock(&pthread_atfork_lock);
  prepare = pthread_atfork_prepare;
  child = pthread_atfork_child;
  parent = pthread_atfork_parent;
  pthread_mutex_unlock(&pthread_atfork_lock);
  pthread_call_handlers(prepare);

#warning hack alert
  __MALLOC_LOCK;

  pid = __libc_fork();

#warning hack alert
  __MALLOC_UNLOCK;

  if (pid == 0) {
    __pthread_reset_main_thread();
#warning need to reconsider __fresetlockfiles!
    __fresetlockfiles();
    pthread_call_handlers(child);
  } else {
    pthread_call_handlers(parent);
  }
  return pid;
}
weak_alias (__fork, fork);

pid_t __vfork(void)
{
  return __fork();
}
weak_alias (__vfork, vfork);

#else

/* We can't support pthread_atfork without MMU, since we don't have
   fork(), and we can't offer the correct semantics for vfork().  */
int pthread_atfork(void (*prepare)(void),
		   void (*parent)(void),
		   void (*child)(void))
{
  /* ENOMEM is probably pushing it a little bit.
     Take it as `no *virtual* memory' :-)  */
  errno = ENOMEM;
  return -1;
}

#endif
