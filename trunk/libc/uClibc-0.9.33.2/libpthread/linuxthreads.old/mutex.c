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

/* Mutexes */

#include <errno.h>
#include <sched.h>
#include <stddef.h>
#include <limits.h>
#include "pthread.h"
#include "internals.h"
#include "spinlock.h"
#include "queue.h"
#include "restart.h"

int attribute_hidden __pthread_mutex_init(pthread_mutex_t * mutex,
                       const pthread_mutexattr_t * mutex_attr)
{
  __pthread_init_lock(&mutex->__m_lock);
  mutex->__m_kind =
    mutex_attr == NULL ? PTHREAD_MUTEX_TIMED_NP : mutex_attr->__mutexkind;
  mutex->__m_count = 0;
  mutex->__m_owner = NULL;
  return 0;
}
strong_alias (__pthread_mutex_init, pthread_mutex_init)

int attribute_hidden __pthread_mutex_destroy(pthread_mutex_t * mutex)
{
  switch (mutex->__m_kind) {
  case PTHREAD_MUTEX_ADAPTIVE_NP:
  case PTHREAD_MUTEX_RECURSIVE_NP:
    if ((mutex->__m_lock.__status & 1) != 0)
      return EBUSY;
    return 0;
  case PTHREAD_MUTEX_ERRORCHECK_NP:
  case PTHREAD_MUTEX_TIMED_NP:
    if (mutex->__m_lock.__status != 0)
      return EBUSY;
    return 0;
  default:
    return EINVAL;
  }
}
strong_alias (__pthread_mutex_destroy, pthread_mutex_destroy)

int attribute_hidden __pthread_mutex_trylock(pthread_mutex_t * mutex)
{
  pthread_descr self;
  int retcode;

  switch(mutex->__m_kind) {
  case PTHREAD_MUTEX_ADAPTIVE_NP:
    retcode = __pthread_trylock(&mutex->__m_lock);
    return retcode;
  case PTHREAD_MUTEX_RECURSIVE_NP:
    self = thread_self();
    if (mutex->__m_owner == self) {
      mutex->__m_count++;
      return 0;
    }
    retcode = __pthread_trylock(&mutex->__m_lock);
    if (retcode == 0) {
      mutex->__m_owner = self;
      mutex->__m_count = 0;
    }
    return retcode;
  case PTHREAD_MUTEX_ERRORCHECK_NP:
    retcode = __pthread_alt_trylock(&mutex->__m_lock);
    if (retcode == 0) {
      mutex->__m_owner = thread_self();
    }
    return retcode;
  case PTHREAD_MUTEX_TIMED_NP:
    retcode = __pthread_alt_trylock(&mutex->__m_lock);
    return retcode;
  default:
    return EINVAL;
  }
}
strong_alias (__pthread_mutex_trylock, pthread_mutex_trylock)

int attribute_hidden __pthread_mutex_lock(pthread_mutex_t * mutex)
{
  pthread_descr self;

  switch(mutex->__m_kind) {
  case PTHREAD_MUTEX_ADAPTIVE_NP:
    __pthread_lock(&mutex->__m_lock, NULL);
    return 0;
  case PTHREAD_MUTEX_RECURSIVE_NP:
    self = thread_self();
    if (mutex->__m_owner == self) {
      mutex->__m_count++;
      return 0;
    }
    __pthread_lock(&mutex->__m_lock, self);
    mutex->__m_owner = self;
    mutex->__m_count = 0;
    return 0;
  case PTHREAD_MUTEX_ERRORCHECK_NP:
    self = thread_self();
    if (mutex->__m_owner == self) return EDEADLK;
    __pthread_alt_lock(&mutex->__m_lock, self);
    mutex->__m_owner = self;
    return 0;
  case PTHREAD_MUTEX_TIMED_NP:
    __pthread_alt_lock(&mutex->__m_lock, NULL);
    return 0;
  default:
    return EINVAL;
  }
}
strong_alias (__pthread_mutex_lock, pthread_mutex_lock)

int pthread_mutex_timedlock (pthread_mutex_t *mutex,
			       const struct timespec *abstime)
{
  pthread_descr self;
  int res;

  if (__builtin_expect (abstime->tv_nsec, 0) < 0
      || __builtin_expect (abstime->tv_nsec, 0) >= 1000000000)
    return EINVAL;

  switch(mutex->__m_kind) {
  case PTHREAD_MUTEX_ADAPTIVE_NP:
    __pthread_lock(&mutex->__m_lock, NULL);
    return 0;
  case PTHREAD_MUTEX_RECURSIVE_NP:
    self = thread_self();
    if (mutex->__m_owner == self) {
      mutex->__m_count++;
      return 0;
    }
    __pthread_lock(&mutex->__m_lock, self);
    mutex->__m_owner = self;
    mutex->__m_count = 0;
    return 0;
  case PTHREAD_MUTEX_ERRORCHECK_NP:
    self = thread_self();
    if (mutex->__m_owner == self) return EDEADLK;
    res = __pthread_alt_timedlock(&mutex->__m_lock, self, abstime);
    if (res != 0)
      {
	mutex->__m_owner = self;
	return 0;
      }
    return ETIMEDOUT;
  case PTHREAD_MUTEX_TIMED_NP:
    /* Only this type supports timed out lock. */
    return (__pthread_alt_timedlock(&mutex->__m_lock, NULL, abstime)
	    ? 0 : ETIMEDOUT);
  default:
    return EINVAL;
  }
}

int attribute_hidden __pthread_mutex_unlock(pthread_mutex_t * mutex)
{
  switch (mutex->__m_kind) {
  case PTHREAD_MUTEX_ADAPTIVE_NP:
    return __pthread_unlock(&mutex->__m_lock);
  case PTHREAD_MUTEX_RECURSIVE_NP:
    if (mutex->__m_owner != thread_self())
      return EPERM;
    if (mutex->__m_count > 0) {
      mutex->__m_count--;
      return 0;
    }
    mutex->__m_owner = NULL;
    return __pthread_unlock(&mutex->__m_lock);
  case PTHREAD_MUTEX_ERRORCHECK_NP:
    if (mutex->__m_owner != thread_self() || mutex->__m_lock.__status == 0)
      return EPERM;
    mutex->__m_owner = NULL;
    __pthread_alt_unlock(&mutex->__m_lock);
    return 0;
  case PTHREAD_MUTEX_TIMED_NP:
    __pthread_alt_unlock(&mutex->__m_lock);
    return 0;
  default:
    return EINVAL;
  }
}
strong_alias (__pthread_mutex_unlock, pthread_mutex_unlock)

int attribute_hidden __pthread_mutexattr_init(pthread_mutexattr_t *attr)
{
  attr->__mutexkind = PTHREAD_MUTEX_TIMED_NP;
  return 0;
}
strong_alias(__pthread_mutexattr_init,pthread_mutexattr_init)

int attribute_hidden __pthread_mutexattr_destroy(pthread_mutexattr_t *attr attribute_unused)
{
  return 0;
}
strong_alias(__pthread_mutexattr_destroy,pthread_mutexattr_destroy)

int attribute_hidden __pthread_mutexattr_settype(pthread_mutexattr_t *attr, int kind)
{
  if (kind != PTHREAD_MUTEX_ADAPTIVE_NP
      && kind != PTHREAD_MUTEX_RECURSIVE_NP
      && kind != PTHREAD_MUTEX_ERRORCHECK_NP
      && kind != PTHREAD_MUTEX_TIMED_NP)
    return EINVAL;
  attr->__mutexkind = kind;
  return 0;
}
strong_alias(__pthread_mutexattr_settype,pthread_mutexattr_settype)
strong_alias (__pthread_mutexattr_settype, __pthread_mutexattr_setkind_np)
weak_alias (__pthread_mutexattr_setkind_np, pthread_mutexattr_setkind_np)

int __pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int *kind) attribute_hidden;
int __pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int *kind)
{
  *kind = attr->__mutexkind;
  return 0;
}
weak_alias (__pthread_mutexattr_gettype, pthread_mutexattr_gettype)
strong_alias (__pthread_mutexattr_gettype, __pthread_mutexattr_getkind_np)
weak_alias (__pthread_mutexattr_getkind_np, pthread_mutexattr_getkind_np)

int __pthread_mutexattr_getpshared (const pthread_mutexattr_t *attr attribute_unused,
				   int *pshared) attribute_hidden;
int __pthread_mutexattr_getpshared (const pthread_mutexattr_t *attr attribute_unused,
				   int *pshared)
{
  *pshared = PTHREAD_PROCESS_PRIVATE;
  return 0;
}
weak_alias (__pthread_mutexattr_getpshared, pthread_mutexattr_getpshared)

int __pthread_mutexattr_setpshared (pthread_mutexattr_t *attr attribute_unused, int pshared) attribute_hidden;
int __pthread_mutexattr_setpshared (pthread_mutexattr_t *attr attribute_unused, int pshared)
{
  if (pshared != PTHREAD_PROCESS_PRIVATE && pshared != PTHREAD_PROCESS_SHARED)
    return EINVAL;

  /* For now it is not possible to shared a conditional variable.  */
  if (pshared != PTHREAD_PROCESS_PRIVATE)
    return ENOSYS;

  return 0;
}
weak_alias (__pthread_mutexattr_setpshared, pthread_mutexattr_setpshared)

/* Once-only execution */

static pthread_mutex_t once_masterlock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t once_finished = PTHREAD_COND_INITIALIZER;
static int fork_generation = 0;	/* Child process increments this after fork. */

enum { NEVER = 0, IN_PROGRESS = 1, DONE = 2 };

/* If a thread is canceled while calling the init_routine out of
   pthread once, this handler will reset the once_control variable
   to the NEVER state. */

static void pthread_once_cancelhandler(void *arg)
{
    pthread_once_t *once_control = arg;

    __pthread_mutex_lock(&once_masterlock);
    *once_control = NEVER;
    __pthread_mutex_unlock(&once_masterlock);
    pthread_cond_broadcast(&once_finished);
}

int __pthread_once(pthread_once_t * once_control, void (*init_routine)(void))
{
  /* flag for doing the condition broadcast outside of mutex */
  int state_changed;

  /* Test without locking first for speed */
  if (*once_control == DONE) {
    READ_MEMORY_BARRIER();
    return 0;
  }
  /* Lock and test again */

  state_changed = 0;

  __pthread_mutex_lock(&once_masterlock);

  /* If this object was left in an IN_PROGRESS state in a parent
     process (indicated by stale generation field), reset it to NEVER. */
  if ((*once_control & 3) == IN_PROGRESS && (*once_control & ~3) != fork_generation)
    *once_control = NEVER;

  /* If init_routine is being called from another routine, wait until
     it completes. */
  while ((*once_control & 3) == IN_PROGRESS) {
    pthread_cond_wait(&once_finished, &once_masterlock);
  }
  /* Here *once_control is stable and either NEVER or DONE. */
  if (*once_control == NEVER) {
    *once_control = IN_PROGRESS | fork_generation;
    __pthread_mutex_unlock(&once_masterlock);
    pthread_cleanup_push(pthread_once_cancelhandler, once_control);
    init_routine();
    pthread_cleanup_pop(0);
    __pthread_mutex_lock(&once_masterlock);
    WRITE_MEMORY_BARRIER();
    *once_control = DONE;
    state_changed = 1;
  }
  __pthread_mutex_unlock(&once_masterlock);

  if (state_changed)
    pthread_cond_broadcast(&once_finished);

  return 0;
}
strong_alias (__pthread_once, pthread_once)

/*
 * Handle the state of the pthread_once mechanism across forks.  The
 * once_masterlock is acquired in the parent process prior to a fork to ensure
 * that no thread is in the critical region protected by the lock.  After the
 * fork, the lock is released. In the child, the lock and the condition
 * variable are simply reset.  The child also increments its generation
 * counter which lets pthread_once calls detect stale IN_PROGRESS states
 * and reset them back to NEVER.
 */

void __pthread_once_fork_prepare(void);
void __pthread_once_fork_prepare(void)
{
  __pthread_mutex_lock(&once_masterlock);
}

void __pthread_once_fork_parent(void);
void __pthread_once_fork_parent(void)
{
  __pthread_mutex_unlock(&once_masterlock);
}

void __pthread_once_fork_child(void);
void __pthread_once_fork_child(void)
{
  __pthread_mutex_init(&once_masterlock, NULL);
  pthread_cond_init(&once_finished, NULL);
  if (fork_generation <= INT_MAX - 4)
    fork_generation += 4;	/* leave least significant two bits zero */
  else
    fork_generation = 0;
}
