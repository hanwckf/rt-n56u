/* POSIX barrier implementation for LinuxThreads.
   Copyright (C) 2000, 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Kaz Kylheku <kaz@ashi.footprints.net>, 2000.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <errno.h>
#include "pthread.h"
#include "internals.h"
#include "spinlock.h"
#include "queue.h"
#include "restart.h"

int
pthread_barrier_wait(pthread_barrier_t *barrier)
{
  pthread_descr self = thread_self();
  pthread_descr temp_wake_queue, th;
  int result = 0;

  __pthread_lock(&barrier->__ba_lock, self);

  /* If the required number of threads have achieved rendezvous... */
  if (barrier->__ba_present >= barrier->__ba_required - 1)
    {
      /* ... then this last caller shall be the serial thread */
      result = PTHREAD_BARRIER_SERIAL_THREAD;
      /* Copy and clear wait queue and reset barrier. */
      temp_wake_queue = barrier->__ba_waiting;
      barrier->__ba_waiting = NULL;
      barrier->__ba_present = 0;
    }
  else
    {
      result = 0;
      barrier->__ba_present++;
      enqueue(&barrier->__ba_waiting, self);
    }

  __pthread_unlock(&barrier->__ba_lock);

  if (result == 0)
    {
      /* Non-serial threads have to suspend */
      suspend(self);
      /* We don't bother dealing with cancellation because the POSIX
         spec for barriers doesn't mention that pthread_barrier_wait
         is a cancellation point. */
    }
  else
    {
      /* Serial thread wakes up all others. */
      while ((th = dequeue(&temp_wake_queue)) != NULL)
	restart(th);
    }

  return result;
}

int
pthread_barrier_init(pthread_barrier_t *barrier,
				const pthread_barrierattr_t *attr,
				unsigned int count)
{
  if (count == 0)
     return EINVAL;

  __pthread_init_lock(&barrier->__ba_lock);
  barrier->__ba_required = count;
  barrier->__ba_present = 0;
  barrier->__ba_waiting = NULL;
  return 0;
}

int
pthread_barrier_destroy(pthread_barrier_t *barrier)
{
  if (barrier->__ba_waiting != NULL) return EBUSY;
  return 0;
}

int
pthread_barrierattr_init(pthread_barrierattr_t *attr)
{
  attr->__pshared = PTHREAD_PROCESS_PRIVATE;
  return 0;
}

int
pthread_barrierattr_destroy(pthread_barrierattr_t *attr)
{
  return 0;
}

int
__pthread_barrierattr_getpshared(const pthread_barrierattr_t *attr,
				 int *pshared)
{
  *pshared = PTHREAD_PROCESS_PRIVATE;
  return 0;
}

int
pthread_barrierattr_setpshared(pthread_barrierattr_t *attr, int pshared)
{
  if (pshared != PTHREAD_PROCESS_PRIVATE && pshared != PTHREAD_PROCESS_SHARED)
    return EINVAL;

  /* For now it is not possible to shared a conditional variable.  */
  if (pshared != PTHREAD_PROCESS_PRIVATE)
    return ENOSYS;

  return 0;
}
