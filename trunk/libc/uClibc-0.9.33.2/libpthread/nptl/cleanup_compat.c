/* Copyright (C) 2002, 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

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

#include <stdlib.h>
#include "pthreadP.h"


void
_pthread_cleanup_push (
     struct _pthread_cleanup_buffer *buffer,
     void (*routine) (void *),
     void *arg)
{
  struct pthread *self = THREAD_SELF;

  buffer->__routine = routine;
  buffer->__arg = arg;
  buffer->__prev = THREAD_GETMEM (self, cleanup);

  THREAD_SETMEM (self, cleanup, buffer);
}
strong_alias (_pthread_cleanup_push, __pthread_cleanup_push)


void
_pthread_cleanup_pop (
     struct _pthread_cleanup_buffer *buffer,
     int execute)
{
  struct pthread *self __attribute ((unused)) = THREAD_SELF;

  THREAD_SETMEM (self, cleanup, buffer->__prev);

  /* If necessary call the cleanup routine after we removed the
     current cleanup block from the list.  */
  if (execute)
    buffer->__routine (buffer->__arg);
}
strong_alias (_pthread_cleanup_pop, __pthread_cleanup_pop)
