/* Copyright (C) 2002,2003,2005,2006,2007,2009 Free Software Foundation, Inc.
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

#include <unistd.h>
#include <list.h>
#include <fork.h>
#include <dl-sysdep.h>
#include <tls.h>
#include <string.h>
#include <pthreadP.h>
#include <bits/libc-lock.h>
#include <sysdep.h>
#include <ldsodefs.h>


#ifdef TLS_MULTIPLE_THREADS_IN_TCB
void
#else
extern int __libc_multiple_threads attribute_hidden;

int *
#endif
__libc_pthread_init (
     unsigned long int *ptr,
     void (*reclaim) (void),
     const struct pthread_functions *functions)
{
  /* Remember the pointer to the generation counter in libpthread.  */
  __fork_generation_pointer = ptr;

  /* Called by a child after fork.  */
  __register_atfork (NULL, NULL, reclaim, NULL);

#ifdef SHARED
  /* We copy the content of the variable pointed to by the FUNCTIONS
     parameter to one in libc.so since this means access to the array
     can be done with one memory access instead of two.
   */
   memcpy (&__libc_pthread_functions, functions,
           sizeof (__libc_pthread_functions));
  __libc_pthread_functions_init = 1;
#endif

#ifndef TLS_MULTIPLE_THREADS_IN_TCB
  return &__libc_multiple_threads;
#endif
}

#ifdef SHARED
#if 0
void
libc_freeres_fn (freeres_libptread)
{
  if (__libc_pthread_functions_init)
    PTHFCT_CALL (ptr_freeres, ());
}
#endif
#endif
