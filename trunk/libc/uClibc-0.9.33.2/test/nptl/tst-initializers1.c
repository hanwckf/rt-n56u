/* Copyright (C) 2005 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2005.

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

#include <pthread.h>

pthread_mutex_t mtx_normal = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mtx_recursive = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
pthread_mutex_t mtx_errorchk = PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP;
pthread_mutex_t mtx_adaptive = PTHREAD_ADAPTIVE_MUTEX_INITIALIZER_NP;
pthread_rwlock_t rwl_normal = PTHREAD_RWLOCK_INITIALIZER;
pthread_rwlock_t rwl_writer
  = PTHREAD_RWLOCK_WRITER_NONRECURSIVE_INITIALIZER_NP;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int
main (void)
{
  if (mtx_normal.__data.__kind != PTHREAD_MUTEX_TIMED_NP)
    return 1;
  if (mtx_recursive.__data.__kind != PTHREAD_MUTEX_RECURSIVE_NP)
    return 1;
  if (mtx_errorchk.__data.__kind != PTHREAD_MUTEX_ERRORCHECK_NP)
    return 1;
  if (mtx_adaptive.__data.__kind != PTHREAD_MUTEX_ADAPTIVE_NP)
    return 1;
  if (rwl_normal.__data.__flags != PTHREAD_RWLOCK_PREFER_READER_NP)
    return 1;
  if (rwl_writer.__data.__flags
      != PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP)
    return 1;
  return 0;
}
