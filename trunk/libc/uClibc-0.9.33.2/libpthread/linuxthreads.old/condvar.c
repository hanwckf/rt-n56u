/* Linuxthreads - a simple clone()-based implementation of Posix        */
/* threads for Linux.                                                   */
/* Copyright (C) 1996 Xavier Leroy (Xavier.Leroy@inria.fr)              */
/* and Pavel Krauz (krauz@fsid.cvut.cz).                                */
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

/* Condition variables */

#include <errno.h>
#include <sched.h>
#include <stddef.h>
#include <sys/time.h>
#include "pthread.h"
#include "internals.h"
#include "spinlock.h"
#include "queue.h"
#include "restart.h"

libpthread_hidden_proto(pthread_cond_broadcast)
libpthread_hidden_proto(pthread_cond_destroy)
libpthread_hidden_proto(pthread_cond_init)
libpthread_hidden_proto(pthread_cond_signal)
libpthread_hidden_proto(pthread_cond_wait)
libpthread_hidden_proto(pthread_cond_timedwait)

libpthread_hidden_proto(pthread_condattr_destroy)
libpthread_hidden_proto(pthread_condattr_init)

int pthread_cond_init(pthread_cond_t *cond,
                      const pthread_condattr_t *cond_attr attribute_unused)
{
  __pthread_init_lock(&cond->__c_lock);
  cond->__c_waiting = NULL;
  return 0;
}
libpthread_hidden_def(pthread_cond_init)

int pthread_cond_destroy(pthread_cond_t *cond)
{
  if (cond->__c_waiting != NULL) return EBUSY;
  return 0;
}
libpthread_hidden_def(pthread_cond_destroy)

/* Function called by pthread_cancel to remove the thread from
   waiting on a condition variable queue. */

static int cond_extricate_func(void *obj, pthread_descr th)
{
  volatile pthread_descr self = thread_self();
  pthread_cond_t *cond = obj;
  int did_remove = 0;

  __pthread_lock(&cond->__c_lock, self);
  did_remove = remove_from_queue(&cond->__c_waiting, th);
  __pthread_unlock(&cond->__c_lock);

  return did_remove;
}

int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
  volatile pthread_descr self = thread_self();
  pthread_extricate_if extr;
  int already_canceled = 0;
  int spurious_wakeup_count;

  /* Check whether the mutex is locked and owned by this thread.  */
  if (mutex->__m_kind != PTHREAD_MUTEX_TIMED_NP
      && mutex->__m_kind != PTHREAD_MUTEX_ADAPTIVE_NP
      && mutex->__m_owner != self)
    return EINVAL;

  /* Set up extrication interface */
  extr.pu_object = cond;
  extr.pu_extricate_func = cond_extricate_func;

  /* Register extrication interface */
  THREAD_SETMEM(self, p_condvar_avail, 0);
  __pthread_set_own_extricate_if(self, &extr);

  /* Atomically enqueue thread for waiting, but only if it is not
     canceled. If the thread is canceled, then it will fall through the
     suspend call below, and then call pthread_exit without
     having to worry about whether it is still on the condition variable queue.
     This depends on pthread_cancel setting p_canceled before calling the
     extricate function. */

  __pthread_lock(&cond->__c_lock, self);
  if (!(THREAD_GETMEM(self, p_canceled)
      && THREAD_GETMEM(self, p_cancelstate) == PTHREAD_CANCEL_ENABLE))
    enqueue(&cond->__c_waiting, self);
  else
    already_canceled = 1;
  __pthread_unlock(&cond->__c_lock);

  if (already_canceled) {
    __pthread_set_own_extricate_if(self, 0);
    __pthread_do_exit(PTHREAD_CANCELED, CURRENT_STACK_FRAME);
  }

  __pthread_mutex_unlock(mutex);

  spurious_wakeup_count = 0;
  while (1)
    {
      suspend(self);
      if (THREAD_GETMEM(self, p_condvar_avail) == 0
	  && (THREAD_GETMEM(self, p_woken_by_cancel) == 0
	      || THREAD_GETMEM(self, p_cancelstate) != PTHREAD_CANCEL_ENABLE))
	{
	  /* Count resumes that don't belong to us. */
	  spurious_wakeup_count++;
	  continue;
	}
      break;
    }

  __pthread_set_own_extricate_if(self, 0);

  /* Check for cancellation again, to provide correct cancellation
     point behavior */

  if (THREAD_GETMEM(self, p_woken_by_cancel)
      && THREAD_GETMEM(self, p_cancelstate) == PTHREAD_CANCEL_ENABLE) {
    THREAD_SETMEM(self, p_woken_by_cancel, 0);
    __pthread_mutex_lock(mutex);
    __pthread_do_exit(PTHREAD_CANCELED, CURRENT_STACK_FRAME);
  }

  /* Put back any resumes we caught that don't belong to us. */
  while (spurious_wakeup_count--)
    restart(self);

  __pthread_mutex_lock(mutex);
  return 0;
}
libpthread_hidden_def(pthread_cond_wait)

static int
pthread_cond_timedwait_relative(pthread_cond_t *cond,
				pthread_mutex_t *mutex,
				const struct timespec * abstime)
{
  volatile pthread_descr self = thread_self();
  int already_canceled = 0;
  pthread_extricate_if extr;
  int spurious_wakeup_count;

  /* Check whether the mutex is locked and owned by this thread.  */
  if (mutex->__m_kind != PTHREAD_MUTEX_TIMED_NP
      && mutex->__m_kind != PTHREAD_MUTEX_ADAPTIVE_NP
      && mutex->__m_owner != self)
    return EINVAL;

  /* Set up extrication interface */
  extr.pu_object = cond;
  extr.pu_extricate_func = cond_extricate_func;

  /* Register extrication interface */
  THREAD_SETMEM(self, p_condvar_avail, 0);
  __pthread_set_own_extricate_if(self, &extr);

  /* Enqueue to wait on the condition and check for cancellation. */
  __pthread_lock(&cond->__c_lock, self);
  if (!(THREAD_GETMEM(self, p_canceled)
      && THREAD_GETMEM(self, p_cancelstate) == PTHREAD_CANCEL_ENABLE))
    enqueue(&cond->__c_waiting, self);
  else
    already_canceled = 1;
  __pthread_unlock(&cond->__c_lock);

  if (already_canceled) {
    __pthread_set_own_extricate_if(self, 0);
    __pthread_do_exit(PTHREAD_CANCELED, CURRENT_STACK_FRAME);
  }

  __pthread_mutex_unlock(mutex);

  spurious_wakeup_count = 0;
  while (1)
    {
      if (!timedsuspend(self, abstime)) {
	int was_on_queue;

	/* __pthread_lock will queue back any spurious restarts that
	   may happen to it. */

	__pthread_lock(&cond->__c_lock, self);
	was_on_queue = remove_from_queue(&cond->__c_waiting, self);
	__pthread_unlock(&cond->__c_lock);

	if (was_on_queue) {
	  __pthread_set_own_extricate_if(self, 0);
	  __pthread_mutex_lock(mutex);
	  return ETIMEDOUT;
	}

	/* Eat the outstanding restart() from the signaller */
	suspend(self);
      }

      if (THREAD_GETMEM(self, p_condvar_avail) == 0
	  && (THREAD_GETMEM(self, p_woken_by_cancel) == 0
	      || THREAD_GETMEM(self, p_cancelstate) != PTHREAD_CANCEL_ENABLE))
	{
	  /* Count resumes that don't belong to us. */
	  spurious_wakeup_count++;
	  continue;
	}
      break;
    }

  __pthread_set_own_extricate_if(self, 0);

  /* The remaining logic is the same as in other cancellable waits,
     such as pthread_join sem_wait or pthread_cond wait. */

  if (THREAD_GETMEM(self, p_woken_by_cancel)
      && THREAD_GETMEM(self, p_cancelstate) == PTHREAD_CANCEL_ENABLE) {
    THREAD_SETMEM(self, p_woken_by_cancel, 0);
    __pthread_mutex_lock(mutex);
    __pthread_do_exit(PTHREAD_CANCELED, CURRENT_STACK_FRAME);
  }

  /* Put back any resumes we caught that don't belong to us. */
  while (spurious_wakeup_count--)
    restart(self);

  __pthread_mutex_lock(mutex);
  return 0;
}

int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex,
                           const struct timespec * abstime)
{
  /* Indirect call through pointer! */
  return pthread_cond_timedwait_relative(cond, mutex, abstime);
}
libpthread_hidden_def(pthread_cond_timedwait)

int pthread_cond_signal(pthread_cond_t *cond)
{
  pthread_descr th;

  __pthread_lock(&cond->__c_lock, NULL);
  th = dequeue(&cond->__c_waiting);
  __pthread_unlock(&cond->__c_lock);
  if (th != NULL) {
    th->p_condvar_avail = 1;
    WRITE_MEMORY_BARRIER();
    restart(th);
  }
  return 0;
}
libpthread_hidden_def(pthread_cond_signal)

int pthread_cond_broadcast(pthread_cond_t *cond)
{
  pthread_descr tosignal, th;

  __pthread_lock(&cond->__c_lock, NULL);
  /* Copy the current state of the waiting queue and empty it */
  tosignal = cond->__c_waiting;
  cond->__c_waiting = NULL;
  __pthread_unlock(&cond->__c_lock);
  /* Now signal each process in the queue */
  while ((th = dequeue(&tosignal)) != NULL) {
    th->p_condvar_avail = 1;
    WRITE_MEMORY_BARRIER();
    restart(th);
  }
  return 0;
}
libpthread_hidden_def(pthread_cond_broadcast)

int pthread_condattr_init(pthread_condattr_t *attr attribute_unused)
{
  return 0;
}
libpthread_hidden_def(pthread_condattr_init)

int pthread_condattr_destroy(pthread_condattr_t *attr attribute_unused)
{
  return 0;
}
libpthread_hidden_def(pthread_condattr_destroy)

int pthread_condattr_getpshared (const pthread_condattr_t *attr attribute_unused, int *pshared)
{
  *pshared = PTHREAD_PROCESS_PRIVATE;
  return 0;
}

int pthread_condattr_setpshared (pthread_condattr_t *attr attribute_unused, int pshared)
{
  if (pshared != PTHREAD_PROCESS_PRIVATE && pshared != PTHREAD_PROCESS_SHARED)
    return EINVAL;

  /* For now it is not possible to shared a conditional variable.  */
  if (pshared != PTHREAD_PROCESS_PRIVATE)
    return ENOSYS;

  return 0;
}
