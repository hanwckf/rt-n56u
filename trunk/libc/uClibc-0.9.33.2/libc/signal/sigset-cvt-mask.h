/* Convert between lowlevel sigmask and libc representation of sigset_t.
   Linux version.
   Copyright (C) 1998, 2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Joe Keane <jgk@jgk.org>.

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

#include <string.h>

static __inline__ void __attribute__ ((unused))
sigset_set_old_mask (sigset_t *set, int mask)
{
  if (_SIGSET_NWORDS == 2) /* typical */
    set->__val[1] = 0;
  if (_SIGSET_NWORDS > 2)
    memset(set, 0, sizeof(*set));
  set->__val[0] = (unsigned int) mask;
}

static __inline__ int __attribute__ ((unused))
sigset_get_old_mask (const sigset_t *set)
{
  return (unsigned int) set->__val[0];
}
