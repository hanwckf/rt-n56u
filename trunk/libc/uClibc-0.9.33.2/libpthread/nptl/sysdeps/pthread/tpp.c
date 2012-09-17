/* Thread Priority Protect helpers.
   Copyright (C) 2006, 2007 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2006.

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

#include <assert.h>
#include <atomic.h>
#include <errno.h>
#include <pthreadP.h>
#include <sched.h>
#include <stdlib.h>
#include "uClibc-glue.h"

int __sched_fifo_min_prio = -1;
int __sched_fifo_max_prio = -1;

void
__init_sched_fifo_prio (void)
{
  __sched_fifo_max_prio = sched_get_priority_max (SCHED_FIFO);
  atomic_write_barrier ();
  __sched_fifo_min_prio = sched_get_priority_min (SCHED_FIFO);
}

int
__pthread_tpp_change_priority (int previous_prio, int new_prio)
{
  struct pthread *self = THREAD_SELF;
  struct priority_protection_data *tpp = THREAD_GETMEM (self, tpp);

  if (tpp == NULL)
    {
      if (__sched_fifo_min_prio == -1)
	__init_sched_fifo_prio ();

      size_t size = sizeof *tpp;
      size += (__sched_fifo_max_prio - __sched_fifo_min_prio + 1)
	      * sizeof (tpp->priomap[0]);
      tpp = calloc (size, 1);
      if (tpp == NULL)
	return ENOMEM;
      tpp->priomax = __sched_fifo_min_prio - 1;
      THREAD_SETMEM (self, tpp, tpp);
    }

  assert (new_prio == -1
	  || (new_prio >= __sched_fifo_min_prio
	      && new_prio <= __sched_fifo_max_prio));
  assert (previous_prio == -1
	  || (previous_prio >= __sched_fifo_min_prio
	      && previous_prio <= __sched_fifo_max_prio));

  int priomax = tpp->priomax;
  int newpriomax = priomax;
  if (new_prio != -1)
    {
      if (tpp->priomap[new_prio - __sched_fifo_min_prio] + 1 == 0)
	return EAGAIN;
      ++tpp->priomap[new_prio - __sched_fifo_min_prio];
      if (new_prio > priomax)
	newpriomax = new_prio;
    }

  if (previous_prio != -1)
    {
      if (--tpp->priomap[previous_prio - __sched_fifo_min_prio] == 0
	  && priomax == previous_prio
	  && previous_prio > new_prio)
	{
	  int i;
	  for (i = previous_prio - 1; i >= __sched_fifo_min_prio; --i)
	    if (tpp->priomap[i - __sched_fifo_min_prio])
	      break;
	  newpriomax = i;
	}
    }

  if (priomax == newpriomax)
    return 0;

  lll_lock (self->lock, LLL_PRIVATE);

  tpp->priomax = newpriomax;

  int result = 0;

  if ((self->flags & ATTR_FLAG_SCHED_SET) == 0)
    {
      if (__sched_getparam (self->tid, &self->schedparam) != 0)
	result = errno;
      else
	self->flags |= ATTR_FLAG_SCHED_SET;
    }

  if ((self->flags & ATTR_FLAG_POLICY_SET) == 0)
    {
      self->schedpolicy = __sched_getscheduler (self->tid);
      if (self->schedpolicy == -1)
	result = errno;
      else
	self->flags |= ATTR_FLAG_POLICY_SET;
    }

  if (result == 0)
    {
      struct sched_param sp = self->schedparam;
      if (sp.sched_priority < newpriomax || sp.sched_priority < priomax)
	{
	  if (sp.sched_priority < newpriomax)
	    sp.sched_priority = newpriomax;

	  if (__sched_setscheduler (self->tid, self->schedpolicy, &sp) < 0)
	    result = errno;
	}
    }

  lll_unlock (self->lock, LLL_PRIVATE);

  return result;
}

int
__pthread_current_priority (void)
{
  struct pthread *self = THREAD_SELF;
  if ((self->flags & (ATTR_FLAG_POLICY_SET | ATTR_FLAG_SCHED_SET))
      == (ATTR_FLAG_POLICY_SET | ATTR_FLAG_SCHED_SET))
    return self->schedparam.sched_priority;

  int result = 0;

  lll_lock (self->lock, LLL_PRIVATE);

  if ((self->flags & ATTR_FLAG_SCHED_SET) == 0)
    {
      if (__sched_getparam (self->tid, &self->schedparam) != 0)
	result = -1;
      else
	self->flags |= ATTR_FLAG_SCHED_SET;
    }

  if ((self->flags & ATTR_FLAG_POLICY_SET) == 0)
    {
      self->schedpolicy = __sched_getscheduler (self->tid);
      if (self->schedpolicy == -1)
	result = -1;
      else
	self->flags |= ATTR_FLAG_POLICY_SET;
    }

  if (result != -1)
    result = self->schedparam.sched_priority;

  lll_unlock (self->lock, LLL_PRIVATE);

  return result;
}
