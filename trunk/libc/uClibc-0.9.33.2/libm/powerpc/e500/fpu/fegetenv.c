/* Store current floating-point environment.
   Copyright (C) 2004 Free Software Foundation, Inc.
   Contributed by Aldy Hernandez <aldyh@redhat.com>, 2004
   This file is part of the GNU C Library.

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
__fegetenv (fenv_t *envp)
{
  fenv_union_t u;
  INTERNAL_SYSCALL_DECL (err);

  INTERNAL_SYSCALL (prctl, err, 2, PR_GET_FPEXC, &u.l[0]);
  u.l[1] = fegetenv_register ();
  *envp = u.fenv;

  /* Success.  */
  return 0;
}

