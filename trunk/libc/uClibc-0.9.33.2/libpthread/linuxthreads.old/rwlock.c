/* Read-write lock implementation.
   Copyright (C) 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Xavier Leroy <Xavier.Leroy@inria.fr>
   and Ulrich Drepper <drepper@cygnus.com>, 1998.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include "internals.h"
#include "queue.h"
#include "spinlock.h"
#include "restart.h"

/*
 * Check whether the calling thread already owns one or more read locks on the
 * specified lock. If so, return a pointer to the read lock info structure
 * corresponding to that lock.
 */

static pthread_readlock_info *
rwlock_is_in_list(pthread_descr self, pthread_rwlock_t *rwlock)
{
  pthread_readlock_info *info;

  for (info = self->p_readlock_list; info != NULL; info = info->pr_next)
    {
      if (info->pr_lock == rwlock)
	return info;
    }

  return NULL;
}

/*
 * Add a new lock to the thread's list of locks for which it has a read lock.
 * A new info node must be allocated for this, which is taken from the thread's
 * free list, or by calling malloc. If malloc fails, a null pointer is
 * returned. Otherwise the lock info structure is initialized and pushed
 * onto the thread's list.
 */

static pthread_readlock_info *
rwlock_add_to_list(pthread_descr self, pthread_rwlock_t *rwlock)
{
  pthread_readlock_info *info = self->p_readlock_free;

  if (info != NULL)
    self->p_readlock_free = info->pr_next;
  else
    info = malloc(sizeof *info);

  if (info == NULL)
    return NULL;

  info->pr_lock_count = 1;
  info->pr_lock = rwlock;
  info->pr_next = self->p_readlock_list;
  self->p_readlock_list = info;

  return info;
}

/*
 * If the thread owns a read lock over the given pthread_rwlock_t,
 * and this read lock is tracked in the thread's lock list,
 * this function returns a pointer to the info node in that list.
 * It also decrements the lock count within that node, and if
 * it reaches zero, it removes the node from the list.
 * If nothing is found, it returns a null pointer.
 */

static pthread_readlock_info *
rwlock_remove_from_list(pthread_descr self, pthread_rwlock_t *rwlock)
{
  pthread_readlock_info **pinfo;

  for (pinfo = &self->p_readlock_list; *pinfo != NULL; pinfo = &(*pinfo)->pr_next)
    {
      if ((*pinfo)->pr_lock == rwlock)
	{
	  pthread_readlock_info *info = *pinfo;
	  if (--info->pr_lock_count == 0)
	    *pinfo = info->pr_next;
	  return info;
	}
    }

  return NULL;
}

/*
 * This function checks whether the conditions are right to place a read lock.
 * It returns 1 if so, otherwise zero. The rwlock's internal lock must be
 * locked upon entry.
 */

static int
rwlock_can_rdlock(pthread_rwlock_t *rwlock, int have_lock_already)
{
  /* Can't readlock; it is write locked. */
  if (rwlock->__rw_writer != NULL)
    return 0;

  /* Lock prefers readers; get it. */
  if (rwlock->__rw_kind == PTHREAD_RWLOCK_PREFER_READER_NP)
    return 1;

  /* Lock prefers writers, but none are waiting. */
  if (queue_is_empty(&rwlock->__rw_write_waiting))
    return 1;

  /* Writers are waiting, but this thread already has a read lock */
  if (have_lock_already)
    return 1;

  /* Writers are waiting, and this is a new lock */
  return 0;
}

/*
 * This function helps support brain-damaged recursive read locking
 * semantics required by Unix 98, while maintaining write priority.
 * This basically determines whether this thread already holds a read lock
 * already. It returns 1 if so, otherwise it returns 0.
 *
 * If the thread has any ``untracked read locks'' then it just assumes
 * that this lock is among them, just to be safe, and returns 1.
 *
 * Also, if it finds the thread's lock in the list, it sets the pointer
 * referenced by pexisting to refer to the list entry.
 *
 * If the thread has no untracked locks, and the lock is not found
 * in its list, then it is added to the list. If this fails,
 * then *pout_of_mem is set to 1.
 */

static int
rwlock_have_already(pthread_descr *pself, pthread_rwlock_t *rwlock,
    pthread_readlock_info **pexisting, int *pout_of_mem)
{
  pthread_readlock_info *existing = NULL;
  int out_of_mem = 0, have_lock_already = 0;
  pthread_descr self = *pself;

  if (rwlock->__rw_kind == PTHREAD_RWLOCK_PREFER_WRITER_NP)
    {
      if (!self)
	self = thread_self();

      existing = rwlock_is_in_list(self, rwlock);

      if (existing != NULL || self->p_untracked_readlock_count > 0)
	have_lock_already = 1;
      else
	{
	  existing = rwlock_add_to_list(self, rwlock);
	  if (existing == NULL)
	    out_of_mem = 1;
	}
    }

  *pout_of_mem = out_of_mem;
  *pexisting = existing;
  *pself = self;

  return have_lock_already;
}

int
pthread_rwlock_init (pthread_rwlock_t *rwlock,
		     const pthread_rwlockattr_t *attr)
{
  __pthread_init_lock(&rwlock->__rw_lock);
  rwlock->__rw_readers = 0;
  rwlock->__rw_writer = NULL;
  rwlock->__rw_read_waiting = NULL;
  rwlock->__rw_write_waiting = NULL;

  if (attr == NULL)
    {
      rwlock->__rw_kind = PTHREAD_RWLOCK_DEFAULT_NP;
      rwlock->__rw_pshared = PTHREAD_PROCESS_PRIVATE;
    }
  else
    {
      rwlock->__rw_kind = attr->__lockkind;
      rwlock->__rw_pshared = attr->__pshared;
    }

  return 0;
}


int
pthread_rwlock_destroy (pthread_rwlock_t *rwlock)
{
  int readers;
  _pthread_descr writer;

  __pthread_lock (&rwlock->__rw_lock, NULL);
  readers = rwlock->__rw_readers;
  writer = rwlock->__rw_writer;
  __pthread_unlock (&rwlock->__rw_lock);

  if (readers > 0 || writer != NULL)
    return EBUSY;

  return 0;
}

int
pthread_rwlock_rdlock (pthread_rwlock_t *rwlock)
{
  pthread_descr self = NULL;
  pthread_readlock_info *existing;
  int out_of_mem, have_lock_already;

  have_lock_already = rwlock_have_already(&self, rwlock,
      &existing, &out_of_mem);

  for (;;)
    {
      if (self == NULL)
	self = thread_self ();

      __pthread_lock (&rwlock->__rw_lock, self);

      if (rwlock_can_rdlock(rwlock, have_lock_already))
	break;

      enqueue (&rwlock->__rw_read_waiting, self);
      __pthread_unlock (&rwlock->__rw_lock);
      suspend (self); /* This is not a cancellation point */
    }

  ++rwlock->__rw_readers;
  __pthread_unlock (&rwlock->__rw_lock);

  if (have_lock_already || out_of_mem)
    {
      if (existing != NULL)
	existing->pr_lock_count++;
      else
	self->p_untracked_readlock_count++;
    }

  return 0;
}

int
pthread_rwlock_tryrdlock (pthread_rwlock_t *rwlock)
{
  pthread_descr self = thread_self();
  pthread_readlock_info *existing;
  int out_of_mem, have_lock_already;
  int retval = EBUSY;

  have_lock_already = rwlock_have_already(&self, rwlock,
      &existing, &out_of_mem);

  __pthread_lock (&rwlock->__rw_lock, self);

  /* 0 is passed to here instead of have_lock_already.
     This is to meet Single Unix Spec requirements:
     if writers are waiting, pthread_rwlock_tryrdlock
     does not acquire a read lock, even if the caller has
     one or more read locks already. */

  if (rwlock_can_rdlock(rwlock, 0))
    {
      ++rwlock->__rw_readers;
      retval = 0;
    }

  __pthread_unlock (&rwlock->__rw_lock);

  if (retval == 0)
    {
      if (have_lock_already || out_of_mem)
	{
	  if (existing != NULL)
	    existing->pr_lock_count++;
	  else
	    self->p_untracked_readlock_count++;
	}
    }

  return retval;
}


int
pthread_rwlock_wrlock (pthread_rwlock_t *rwlock)
{
  pthread_descr self = thread_self ();

  while(1)
    {
      __pthread_lock (&rwlock->__rw_lock, self);
      if (rwlock->__rw_readers == 0 && rwlock->__rw_writer == NULL)
	{
	  rwlock->__rw_writer = self;
	  __pthread_unlock (&rwlock->__rw_lock);
	  return 0;
	}

      /* Suspend ourselves, then try again */
      enqueue (&rwlock->__rw_write_waiting, self);
      __pthread_unlock (&rwlock->__rw_lock);
      suspend (self); /* This is not a cancellation point */
    }
}


int
pthread_rwlock_trywrlock (pthread_rwlock_t *rwlock)
{
  int result = EBUSY;

  __pthread_lock (&rwlock->__rw_lock, NULL);
  if (rwlock->__rw_readers == 0 && rwlock->__rw_writer == NULL)
    {
      rwlock->__rw_writer = thread_self ();
      result = 0;
    }
  __pthread_unlock (&rwlock->__rw_lock);

  return result;
}


int
pthread_rwlock_unlock (pthread_rwlock_t *rwlock)
{
  pthread_descr torestart;
  pthread_descr th;

  __pthread_lock (&rwlock->__rw_lock, NULL);
  if (rwlock->__rw_writer != NULL)
    {
      /* Unlocking a write lock.  */
      if (rwlock->__rw_writer != thread_self ())
	{
	  __pthread_unlock (&rwlock->__rw_lock);
	  return EPERM;
	}
      rwlock->__rw_writer = NULL;

      if (rwlock->__rw_kind == PTHREAD_RWLOCK_PREFER_READER_NP
	  || (th = dequeue (&rwlock->__rw_write_waiting)) == NULL)
	{
	  /* Restart all waiting readers.  */
	  torestart = rwlock->__rw_read_waiting;
	  rwlock->__rw_read_waiting = NULL;
	  __pthread_unlock (&rwlock->__rw_lock);
	  while ((th = dequeue (&torestart)) != NULL)
	    restart (th);
	}
      else
	{
	  /* Restart one waiting writer.  */
	  __pthread_unlock (&rwlock->__rw_lock);
	  restart (th);
	}
    }
  else
    {
      /* Unlocking a read lock.  */
      if (rwlock->__rw_readers == 0)
	{
	  __pthread_unlock (&rwlock->__rw_lock);
	  return EPERM;
	}

      --rwlock->__rw_readers;
      if (rwlock->__rw_readers == 0)
	/* Restart one waiting writer, if any.  */
	th = dequeue (&rwlock->__rw_write_waiting);
      else
	th = NULL;

      __pthread_unlock (&rwlock->__rw_lock);
      if (th != NULL)
	restart (th);

      /* Recursive lock fixup */

      if (rwlock->__rw_kind == PTHREAD_RWLOCK_PREFER_WRITER_NP)
	{
	  pthread_descr self = thread_self();
	  pthread_readlock_info *victim = rwlock_remove_from_list(self, rwlock);

	  if (victim != NULL)
	    {
	      if (victim->pr_lock_count == 0)
		{
		  victim->pr_next = self->p_readlock_free;
		  self->p_readlock_free = victim;
		}
	    }
	  else
	    {
	      if (self->p_untracked_readlock_count > 0)
		self->p_untracked_readlock_count--;
	    }
	}
    }

  return 0;
}



int
pthread_rwlockattr_init (pthread_rwlockattr_t *attr)
{
  attr->__lockkind = 0;
  attr->__pshared = 0;

  return 0;
}


int
pthread_rwlockattr_destroy (pthread_rwlockattr_t *attr attribute_unused)
{
  return 0;
}


int
pthread_rwlockattr_getpshared (const pthread_rwlockattr_t *attr, int *pshared)
{
  *pshared = attr->__pshared;
  return 0;
}


int
pthread_rwlockattr_setpshared (pthread_rwlockattr_t *attr, int pshared)
{
  if (pshared != PTHREAD_PROCESS_PRIVATE && pshared != PTHREAD_PROCESS_SHARED)
    return EINVAL;

  attr->__pshared = pshared;

  return 0;
}


int
pthread_rwlockattr_getkind_np (const pthread_rwlockattr_t *attr, int *pref)
{
  *pref = attr->__lockkind;
  return 0;
}


int
pthread_rwlockattr_setkind_np (pthread_rwlockattr_t *attr, int pref)
{
  if (pref != PTHREAD_RWLOCK_PREFER_READER_NP
      && pref != PTHREAD_RWLOCK_PREFER_WRITER_NP
      && pref != PTHREAD_RWLOCK_DEFAULT_NP)
    return EINVAL;

  attr->__lockkind = pref;

  return 0;
}
