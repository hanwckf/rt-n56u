/* sem_post -- post to a POSIX semaphore.  Powerpc version.
   Copyright (C) 2003, 2004, 2007 Free Software Foundation, Inc.
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
__new_sem_post (sem_t *sem)
{
  struct new_sem *isem = (struct new_sem *) sem;

  __asm__ __volatile__ (__lll_rel_instr ::: "memory");
  atomic_increment (&isem->value);
  __asm__ __volatile__ (__lll_acq_instr ::: "memory");
  if (isem->nwaiters > 0)
    {
      int err = lll_futex_wake (&isem->value, 1,
				isem->private ^ FUTEX_PRIVATE_FLAG);
      if (__builtin_expect (err, 0) < 0)
	{
	  __set_errno (-err);
	  return -1;
	}
    }
  return 0;
}
weak_alias(__new_sem_post, sem_post)
