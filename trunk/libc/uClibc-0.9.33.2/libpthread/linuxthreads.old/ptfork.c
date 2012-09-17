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

#ifdef __ARCH_USE_MMU__

#include <bits/uClibc_mutex.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include "pthread.h"
#include "internals.h"

struct handler_list {
  void (*handler)(void);
  struct handler_list * next;
};

static pthread_mutex_t pthread_atfork_lock = PTHREAD_MUTEX_INITIALIZER;
static struct handler_list * pthread_atfork_prepare = NULL;
static struct handler_list * pthread_atfork_parent = NULL;
static struct handler_list * pthread_atfork_child = NULL;

#ifdef __MALLOC__
__UCLIBC_MUTEX_EXTERN(__malloc_heap_lock);
__UCLIBC_MUTEX_EXTERN(__malloc_sbrk_lock);
#ifdef __UCLIBC_UCLINUX_BROKEN_MUNMAP__
__UCLIBC_MUTEX_EXTERN(__malloc_mmb_heap_lock);
#endif
#elif defined(__MALLOC_STANDARD__) || defined(__MALLOC_SIMPLE__)
__UCLIBC_MUTEX_EXTERN(__malloc_lock);
#endif

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
  __pthread_mutex_lock(&pthread_atfork_lock);
  /* "prepare" handlers are called in LIFO */
  pthread_insert_list(&pthread_atfork_prepare, prepare, &block->prepare, 0);
  /* "parent" handlers are called in FIFO */
  pthread_insert_list(&pthread_atfork_parent, parent, &block->parent, 1);
  /* "child" handlers are called in FIFO */
  pthread_insert_list(&pthread_atfork_child, child, &block->child, 1);
  __pthread_mutex_unlock(&pthread_atfork_lock);
  return 0;
}
/*strong_alias (__pthread_atfork, pthread_atfork)*/

static __inline__ void pthread_call_handlers(struct handler_list * list)
{
  for (/*nothing*/; list != NULL; list = list->next) (list->handler)();
}

void __pthread_once_fork_prepare(void);
void __pthread_once_fork_child(void);
void __pthread_once_fork_parent(void);

extern __typeof(fork) __libc_fork;

pid_t __fork(void) attribute_hidden;
pid_t __fork(void)
{
  pid_t pid;
  struct handler_list * prepare, * child, * parent;

  __pthread_mutex_lock(&pthread_atfork_lock);
  prepare = pthread_atfork_prepare;
  child = pthread_atfork_child;
  parent = pthread_atfork_parent;
  pthread_call_handlers(prepare);

  __pthread_once_fork_prepare();
#ifdef __MALLOC__
  __pthread_mutex_lock(&__malloc_sbrk_lock);
  __pthread_mutex_lock(&__malloc_heap_lock);
#ifdef __UCLIBC_UCLINUX_BROKEN_MUNMAP__
  __pthread_mutex_lock(&__malloc_mmb_heap_lock);
#endif
#elif defined(__MALLOC_STANDARD__) || defined(__MALLOC_SIMPLE__)
  __pthread_mutex_lock(&__malloc_lock);
#endif

  pid = __libc_fork();
  if (pid == 0) {
#if defined(__MALLOC_STANDARD__) || defined(__MALLOC_SIMPLE__)
    __libc_lock_init_recursive(__malloc_lock);
#elif defined(__MALLOC__)
#ifdef __UCLIBC_UCLINUX_BROKEN_MUNMAP__
    __libc_lock_init_adaptive(__malloc_mmb_heap_lock);
#endif
    __libc_lock_init_adaptive(__malloc_heap_lock);
    __libc_lock_init(__malloc_sbrk_lock);
#endif
    __libc_lock_init_adaptive(pthread_atfork_lock);
    __pthread_reset_main_thread();
    __fresetlockfiles();
    __pthread_once_fork_child();
    pthread_call_handlers(child);
  } else {
#if defined(__MALLOC_STANDARD__) || defined(__MALLOC_SIMPLE__)
    __pthread_mutex_unlock(&__malloc_lock);
#elif defined(__MALLOC__)
#ifdef __UCLIBC_UCLINUX_BROKEN_MUNMAP__
    __pthread_mutex_unlock(&__malloc_mmb_heap_lock);
#endif
    __pthread_mutex_unlock(&__malloc_heap_lock);
    __pthread_mutex_unlock(&__malloc_sbrk_lock);
#endif
    __pthread_mutex_unlock(&pthread_atfork_lock);
    __pthread_once_fork_parent();
    pthread_call_handlers(parent);
  }
  return pid;
}
strong_alias(__fork,fork)

pid_t vfork(void)
{
  return __fork();
}

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
