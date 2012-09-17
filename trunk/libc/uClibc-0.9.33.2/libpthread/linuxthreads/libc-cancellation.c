/* Copyright (C) 2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2002.

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

#include <errno.h>
#include <rpc/rpc.h>
#include "pthread.h"
#include "internals.h"
#include "spinlock.h"
#include "restart.h"
#include <bits/libc-lock.h>

#if !defined NOT_IN_libc

# ifndef SHARED
weak_extern (__pthread_do_exit)
# endif

/* The next two functions are similar to pthread_setcanceltype() but
   more specialized for the use in the cancelable functions like write().
   They do not need to check parameters etc.  */
int
attribute_hidden
__libc_enable_asynccancel (void)
{
  pthread_descr self = thread_self();
  int oldtype = LIBC_THREAD_GETMEM(self, p_canceltype);
  LIBC_THREAD_SETMEM(self, p_canceltype, PTHREAD_CANCEL_ASYNCHRONOUS);
  if (__builtin_expect (LIBC_THREAD_GETMEM(self, p_canceled), 0) &&
      LIBC_THREAD_GETMEM(self, p_cancelstate) == PTHREAD_CANCEL_ENABLE)
    __libc_maybe_call2 (pthread_do_exit,
			(PTHREAD_CANCELED, CURRENT_STACK_FRAME), 0);
  return oldtype;
}
strong_alias (__libc_enable_asynccancel, __librt_enable_asynccancel)

void
internal_function attribute_hidden
__libc_disable_asynccancel (int oldtype)
{
  pthread_descr self = thread_self();
  LIBC_THREAD_SETMEM(self, p_canceltype, oldtype);
}
strong_alias (__libc_disable_asynccancel, __librt_disable_asynccancel)

#endif
