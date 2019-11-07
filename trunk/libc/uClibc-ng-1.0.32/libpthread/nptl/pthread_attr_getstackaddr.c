/* Copyright (C) 2002, 2004 Free Software Foundation, Inc.
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

#include <assert.h>
#include <errno.h>
#include "pthreadP.h"


int
__pthread_attr_getstackaddr (
     const pthread_attr_t *attr,
     void **stackaddr)
{
  struct pthread_attr *iattr;

  assert (sizeof (*attr) >= sizeof (struct pthread_attr));
  iattr = (struct pthread_attr *) attr;

  /* Some code assumes this function to work even if no stack address
     has been set.  Let them figure it out for themselves what the
     value means.  Simply store the result.  */
  *stackaddr = iattr->stackaddr;

  return 0;
}
strong_alias (__pthread_attr_getstackaddr, pthread_attr_getstackaddr)
