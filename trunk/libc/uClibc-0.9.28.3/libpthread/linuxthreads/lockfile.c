/* lockfile - Handle locking and unlocking of stream.
   Copyright (C) 1996, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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

#include <stdio.h>
#include <pthread.h>

/* Note: glibc puts flockfile, funlockfile, and ftrylockfile in both
 * libc and libpthread.  In uClibc, they are now in libc only.  */

void
__fresetlockfiles (void)
{
  FILE *fp;
  pthread_mutexattr_t attr;

  pthread_mutexattr_init(&attr);
  pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);

  for (fp = _stdio_openlist; fp != NULL; fp = fp->__nextopen)
    pthread_mutex_init(&fp->__lock, &attr);

  pthread_mutexattr_destroy(&attr);
}
