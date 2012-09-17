/* Copyright (C) 2002, 2003, 2006 Free Software Foundation, Inc.
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

#include <errno.h>
#include "pthreadP.h"
#include <atomic.h>


int
attribute_protected
__pthread_key_create (
     pthread_key_t *key,
     void (*destr) (void *))
{
  /* Find a slot in __pthread_kyes which is unused.  */
  size_t cnt;
  for (cnt = 0; cnt < PTHREAD_KEYS_MAX; ++cnt)
    {
      uintptr_t seq = __pthread_keys[cnt].seq;

      if (KEY_UNUSED (seq) && KEY_USABLE (seq)
	  /* We found an unused slot.  Try to allocate it.  */
	  && ! atomic_compare_and_exchange_bool_acq (&__pthread_keys[cnt].seq,
						     seq + 1, seq))
	{
	  /* Remember the destructor.  */
	  __pthread_keys[cnt].destr = destr;

	  /* Return the key to the caller.  */
	  *key = cnt;

	  /* The call succeeded.  */
	  return 0;
	}
    }

  return EAGAIN;
}
strong_alias (__pthread_key_create, pthread_key_create)
strong_alias (__pthread_key_create, __pthread_key_create_internal)
