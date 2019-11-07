/* Copyright (C) 2002, 2005 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <pthreadP.h>


int
pthread_mutexattr_setpshared (
     pthread_mutexattr_t *attr,
     int pshared)
{
  struct pthread_mutexattr *iattr;

  if (pshared != PTHREAD_PROCESS_PRIVATE
      && __builtin_expect (pshared != PTHREAD_PROCESS_SHARED, 0))
    return EINVAL;

  iattr = (struct pthread_mutexattr *) attr;

  if (pshared == PTHREAD_PROCESS_PRIVATE)
    iattr->mutexkind &= ~PTHREAD_MUTEXATTR_FLAG_PSHARED;
  else
    iattr->mutexkind |= PTHREAD_MUTEXATTR_FLAG_PSHARED;

  return 0;
}
