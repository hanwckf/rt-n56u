/*
 * This file contains the old semaphore code that we need to
 * preserve for glibc-2.0 backwards compatibility. Port to glibc 2.1
 * done by Cristian Gafton.
 */

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

/* Semaphores a la POSIX 1003.1b */

#include <errno.h>
#include "pthread.h"
#include "internals.h"
#include "spinlock.h"
#include "restart.h"
#include "queue.h"

typedef struct {
    long int sem_status;
    int sem_spinlock;
} old_sem_t;

/* Maximum value the semaphore can have.  */
#define SEM_VALUE_MAX   ((int) ((~0u) >> 1))

static inline int sem_compare_and_swap(old_sem_t *sem, long oldval, long newval)
{
    return compare_and_swap(&sem->sem_status, oldval, newval, &sem->sem_spinlock);
}

/* The state of a semaphore is represented by a long int encoding
   either the semaphore count if >= 0 and no thread is waiting on it,
   or the head of the list of threads waiting for the semaphore.
   To distinguish the two cases, we encode the semaphore count N
   as 2N+1, so that it has the lowest bit set.

   A sequence of sem_wait operations on a semaphore initialized to N
   result in the following successive states:
     2N+1, 2N-1, ..., 3, 1, &first_waiting_thread, &second_waiting_thread, ...
*/

static void sem_restart_list(pthread_descr waiting);

int __old_sem_init(old_sem_t *sem, int pshared, unsigned int value)
{
    if (value > SEM_VALUE_MAX) {
	errno = EINVAL;
	return -1;
    }
    if (pshared) {
	errno = ENOSYS;
	return -1;
    }
  sem->sem_spinlock = 0;
  sem->sem_status = ((long)value << 1) + 1;
  return 0;
}

/* Function called by pthread_cancel to remove the thread from
   waiting inside __old_sem_wait. Here we simply unconditionally
   indicate that the thread is to be woken, by returning 1. */

static int old_sem_extricate_func(void *obj, pthread_descr th)
{
    return 1;
}

int __old_sem_wait(old_sem_t * sem)
{
    long oldstatus, newstatus;
    volatile pthread_descr self = thread_self();
    pthread_descr * th;
    pthread_extricate_if extr;

    /* Set up extrication interface */
    extr.pu_object = 0;
    extr.pu_extricate_func = old_sem_extricate_func;

    while (1) {
	/* Register extrication interface */
	__pthread_set_own_extricate_if(self, &extr); 
	do {
            oldstatus = sem->sem_status;
            if ((oldstatus & 1) && (oldstatus != 1))
		newstatus = oldstatus - 2;
            else {
		newstatus = (long) self;
		self->p_nextwaiting = (pthread_descr) oldstatus;
	    }
	}
	while (! sem_compare_and_swap(sem, oldstatus, newstatus));
	if (newstatus & 1) {
	    /* We got the semaphore. */
	  __pthread_set_own_extricate_if(self, 0); 
	    return 0;
	}
	/* Wait for sem_post or cancellation */
	suspend(self);
	__pthread_set_own_extricate_if(self, 0); 

	/* This is a cancellation point */
	if (self->p_canceled && self->p_cancelstate == PTHREAD_CANCEL_ENABLE) {
	    /* Remove ourselves from the waiting list if we're still on it */
	    /* First check if we're at the head of the list. */
            do {
		oldstatus = sem->sem_status;
		if (oldstatus != (long) self) break;
		newstatus = (long) self->p_nextwaiting;
	    }
            while (! sem_compare_and_swap(sem, oldstatus, newstatus));
            /* Now, check if we're somewhere in the list.
	       There's a race condition with sem_post here, but it does not matter:
	       the net result is that at the time pthread_exit is called,
	       self is no longer reachable from sem->sem_status. */
            if (oldstatus != (long) self && (oldstatus & 1) == 0) {
		for (th = &(((pthread_descr) oldstatus)->p_nextwaiting);
		     *th != NULL && *th != (pthread_descr) 1;
		     th = &((*th)->p_nextwaiting)) {
		    if (*th == self) {
			*th = self->p_nextwaiting;
			break;
		    }
		}
	    }
            pthread_exit(PTHREAD_CANCELED);
	}
    }
}

int __old_sem_trywait(old_sem_t * sem)
{
  long oldstatus, newstatus;

  do {
    oldstatus = sem->sem_status;
    if ((oldstatus & 1) == 0 || (oldstatus == 1)) {
      errno = EAGAIN;
      return -1;
    }
    newstatus = oldstatus - 2;
  }
  while (! sem_compare_and_swap(sem, oldstatus, newstatus));
  return 0;
}

int __old_sem_post(old_sem_t * sem)
{
  long oldstatus, newstatus;

  do {
    oldstatus = sem->sem_status;
    if ((oldstatus & 1) == 0)
      newstatus = 3;
    else {
      if (oldstatus >= SEM_VALUE_MAX) {
        /* Overflow */
        errno = ERANGE;
        return -1;
      }
      newstatus = oldstatus + 2;
    }
  }
  while (! sem_compare_and_swap(sem, oldstatus, newstatus));
  if ((oldstatus & 1) == 0)
    sem_restart_list((pthread_descr) oldstatus);
  return 0;
}

int __old_sem_getvalue(old_sem_t * sem, int * sval)
{
  long status = sem->sem_status;
  if (status & 1)
    *sval = (int)((unsigned long) status >> 1);
  else
    *sval = 0;
  return 0;
}

int __old_sem_destroy(old_sem_t * sem)
{
  if ((sem->sem_status & 1) == 0) {
    errno = EBUSY;
    return -1;
  }
  return 0;
}

/* Auxiliary function for restarting all threads on a waiting list,
   in priority order. */

static void sem_restart_list(pthread_descr waiting)
{
  pthread_descr th, towake, *p;

  /* Sort list of waiting threads by decreasing priority (insertion sort) */
  towake = NULL;
  while (waiting != (pthread_descr) 1) {
    th = waiting;
    waiting = waiting->p_nextwaiting;
    p = &towake;
    while (*p != NULL && th->p_priority < (*p)->p_priority)
      p = &((*p)->p_nextwaiting);
    th->p_nextwaiting = *p;
    *p = th;
  }
  /* Wake up threads in priority order */
  while (towake != NULL) {
    th = towake;
    towake = towake->p_nextwaiting;
    th->p_nextwaiting = NULL;
    restart(th);
  }
}

#if defined __PIC__ && defined DO_VERSIONING
symbol_version (__old_sem_init, sem_init, GLIBC_2.0);
symbol_version (__old_sem_wait, sem_wait, GLIBC_2.0);
symbol_version (__old_sem_trywait, sem_trywait, GLIBC_2.0);
symbol_version (__old_sem_post, sem_post, GLIBC_2.0);
symbol_version (__old_sem_getvalue, sem_getvalue, GLIBC_2.0);
symbol_version (__old_sem_destroy, sem_destroy, GLIBC_2.0);
#endif

