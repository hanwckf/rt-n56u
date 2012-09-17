/* sem_trywait -- wait on a semaphore.  SPARC version.
   Copyright (C) 2003, 2006, 2007 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Paul Mackerras <paulus@au.ibm.com>, 2003.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <errno.h>
#include <sysdep.h>
#include <lowlevellock.h>
#include <internaltypes.h>
#include <semaphore.h>


int
__new_sem_trywait (sem_t *sem)
{
  struct sparc_old_sem *isem = (struct sparc_old_sem *) sem;
  int val;

  if (isem->value > 0)
    {
      if (__atomic_is_v9)
	val = atomic_decrement_if_positive (&isem->value);
      else
	{
	  __sparc32_atomic_do_lock24 (&isem->lock);
	  val = isem->value;
	  if (val > 0)
	    isem->value = val - 1;
	  __sparc32_atomic_do_unlock24 (&isem->lock);
	}
      if (val > 0)
	return 0;
    }

  __set_errno (EAGAIN);
  return -1;
}
weak_alias(__new_sem_trywait, sem_trywait)

