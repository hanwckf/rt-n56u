/* Install given floating-point environment and raise exceptions.
   Copyright (C) 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Aldy Hernandez <aldyh@redhat.com>, 2004.

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

#include "fenv_libc.h"
#include <syscall.h>
#include <sys/prctl.h>

int
__feupdateenv (const fenv_t *envp)
{
  unsigned long fpescr, old, new, pflags;
  fenv_union_t u;
  INTERNAL_SYSCALL_DECL (err);

  /* Save the currently set exceptions.  */
  u.fenv = *envp;
  new = u.l[1];
  old = fegetenv_register ();
  new |= (old & FE_ALL_EXCEPT);

  INTERNAL_SYSCALL (prctl, err, 2, PR_GET_FPEXC, &pflags);
  pflags |= u.l[0];
  INTERNAL_SYSCALL (prctl, err, 2, PR_SET_FPEXC, pflags);

  /* Enable and raise (if appropriate) exceptions set in `new'. */
  fesetenv_register (new);
  feraiseexcept (new & FE_ALL_EXCEPT);

  /* Success.  */
  return 0;
}

