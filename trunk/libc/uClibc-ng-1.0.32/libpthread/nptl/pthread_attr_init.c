/* Copyright (C) 2002, 2003, 2004, 2007 Free Software Foundation, Inc.
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
#include <string.h>
#include <unistd.h>
#include "pthreadP.h"


struct pthread_attr *__attr_list;
int __attr_list_lock = LLL_LOCK_INITIALIZER;


int
attribute_protected
pthread_attr_init (
     pthread_attr_t *attr)
{
  struct pthread_attr *iattr;

  /* Many elements are initialized to zero so let us do it all at
     once.  This also takes care of clearing the bytes which are not
     internally used.  */
  memset (attr, '\0', __SIZEOF_PTHREAD_ATTR_T);

  assert (sizeof (*attr) >= sizeof (struct pthread_attr));
  iattr = (struct pthread_attr *) attr;

  /* Default guard size specified by the standard.  */
  iattr->guardsize = getpagesize ();

  return 0;
}
