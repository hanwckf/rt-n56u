/* Get priority protocol setting from pthread_mutexattr_t.
   Copyright (C) 2006 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2006.

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

#include <pthreadP.h>


int
pthread_mutexattr_getprotocol (
     const pthread_mutexattr_t *attr,
     int *protocol)
{
  const struct pthread_mutexattr *iattr;

  iattr = (const struct pthread_mutexattr *) attr;

  *protocol = ((iattr->mutexkind & PTHREAD_MUTEXATTR_PROTOCOL_MASK)
	       >> PTHREAD_MUTEXATTR_PROTOCOL_SHIFT);

  return 0;
}
