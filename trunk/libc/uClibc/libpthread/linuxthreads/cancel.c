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

/* Thread cancellation */

#define __FORCE_GLIBC
#include <features.h>
#include <errno.h>
#include "pthread.h"
#include "internals.h"
#include "spinlock.h"
#include "restart.h"
#ifdef __UCLIBC_HAS_RPC__
#include <rpc/rpc.h>
extern void __rpc_thread_destroy(void);
#endif


int pthread_setcancelstate(int state, int * oldstate)
{
  pthread_descr self = thread_self();
  if (state < PTHREAD_CANCEL_ENABLE || state > PTHREAD_CANCEL_DISABLE)
    return EINVAL;
  if (oldstate != NULL) *oldstate = THREAD_GETMEM(self, p_cancelstate);
  THREAD_SETMEM(self, p_cancelstate, state);
  if (THREAD_GETMEM(self, p_canceled) &&
      THREAD_GETMEM(self, p_cancelstate) == PTHREAD_CANCEL_ENABLE &&
      THREAD_GETMEM(self, p_canceltype) == PTHREAD_CANCEL_ASYNCHRONOUS)
    pthread_exit(PTHREAD_CANCELED);
  return 0;
}

int pthread_setcanceltype(int type, int * oldtype)
{
  pthread_descr self = thread_self();
  if (type < PTHREAD_CANCEL_DEFERRED || type > PTHREAD_CANCEL_ASYNCHRONOUS)
    return EINVAL;
  if (oldtype != NULL) *oldtype = THREAD_GETMEM(self, p_canceltype);
  THREAD_SETMEM(self, p_canceltype, type);
  if (THREAD_GETMEM(self, p_canceled) &&
      THREAD_GETMEM(self, p_cancelstate) == PTHREAD_CANCEL_ENABLE &&
      THREAD_GETMEM(self, p_canceltype) == PTHREAD_CANCEL_ASYNCHRONOUS)
    pthread_exit(PTHREAD_CANCELED);
  return 0;
}

int pthread_cancel(pthread_t thread)
{
  pthread_handle handle = thread_handle(thread);
  int pid;
  int dorestart = 0;
  pthread_descr th;
  pthread_extricate_if *pextricate;

  __pthread_lock(&handle->h_lock, NULL);
  if (invalid_handle(handle, thread)) {
    __pthread_unlock(&handle->h_lock);
    return ESRCH;
  }

  th = handle->h_descr;

  if (th->p_canceled) {
    __pthread_unlock(&handle->h_lock);
    return 0;
  }

  pextricate = th->p_extricate;
  th->p_canceled = 1;
  pid = th->p_pid;

  /* If the thread has registered an extrication interface, then
     invoke the interface. If it returns 1, then we succeeded in
     dequeuing the thread from whatever waiting object it was enqueued
     with. In that case, it is our responsibility to wake it up. 
     And also to set the p_woken_by_cancel flag so the woken thread
     can tell that it was woken by cancellation. */

  if (pextricate != NULL) {
    dorestart = pextricate->pu_extricate_func(pextricate->pu_object, th);
    th->p_woken_by_cancel = dorestart;
  }

  __pthread_unlock(&handle->h_lock);

  /* If the thread has suspended or is about to, then we unblock it by
     issuing a restart, instead of a cancel signal. Otherwise we send
     the cancel signal to unblock the thread from a cancellation point,
     or to initiate asynchronous cancellation. The restart is needed so
     we have proper accounting of restarts; suspend decrements the thread's
     resume count, and restart() increments it.  This also means that suspend's
     handling of the cancel signal is obsolete. */

  if (dorestart)
    restart(th);
  else 
    kill(pid, __pthread_sig_cancel);

  return 0;
}

void pthread_testcancel(void)
{
  pthread_descr self = thread_self();
  if (THREAD_GETMEM(self, p_canceled)
      && THREAD_GETMEM(self, p_cancelstate) == PTHREAD_CANCEL_ENABLE)
    pthread_exit(PTHREAD_CANCELED);
}

void _pthread_cleanup_push(struct _pthread_cleanup_buffer * buffer,
			   void (*routine)(void *), void * arg)
{
  pthread_descr self = thread_self();
  buffer->__routine = routine;
  buffer->__arg = arg;
  buffer->__prev = THREAD_GETMEM(self, p_cleanup);
  THREAD_SETMEM(self, p_cleanup, buffer);
}

void _pthread_cleanup_pop(struct _pthread_cleanup_buffer * buffer,
			  int execute)
{
  pthread_descr self = thread_self();
  if (execute) buffer->__routine(buffer->__arg);
  THREAD_SETMEM(self, p_cleanup, buffer->__prev);
}

void _pthread_cleanup_push_defer(struct _pthread_cleanup_buffer * buffer,
				 void (*routine)(void *), void * arg)
{
  pthread_descr self = thread_self();
  buffer->__routine = routine;
  buffer->__arg = arg;
  buffer->__canceltype = THREAD_GETMEM(self, p_canceltype);
  buffer->__prev = THREAD_GETMEM(self, p_cleanup);
  THREAD_SETMEM(self, p_canceltype, PTHREAD_CANCEL_DEFERRED);
  THREAD_SETMEM(self, p_cleanup, buffer);
}

void _pthread_cleanup_pop_restore(struct _pthread_cleanup_buffer * buffer,
				  int execute)
{
  pthread_descr self = thread_self();
  if (execute) buffer->__routine(buffer->__arg);
  THREAD_SETMEM(self, p_cleanup, buffer->__prev);
  THREAD_SETMEM(self, p_canceltype, buffer->__canceltype);
  if (THREAD_GETMEM(self, p_canceled) &&
      THREAD_GETMEM(self, p_cancelstate) == PTHREAD_CANCEL_ENABLE &&
      THREAD_GETMEM(self, p_canceltype) == PTHREAD_CANCEL_ASYNCHRONOUS)
    pthread_exit(PTHREAD_CANCELED);
}

void __pthread_perform_cleanup(void)
{
  pthread_descr self = thread_self();
  struct _pthread_cleanup_buffer * c;
  for (c = THREAD_GETMEM(self, p_cleanup); c != NULL; c = c->__prev)
    c->__routine(c->__arg);

#ifdef __UCLIBC_HAS_RPC__
  /* And the TSD which needs special help.  */
  if (THREAD_GETMEM(self, p_libc_specific[_LIBC_TSD_KEY_RPC_VARS]) != NULL)
      __rpc_thread_destroy ();
#endif
}

#ifndef __PIC__
/* We need a hook to force the cancelation wrappers to be linked in when
   static libpthread is used.  */
extern const int __pthread_provide_wrappers;
static const int * const __pthread_require_wrappers =
  &__pthread_provide_wrappers;
#endif
