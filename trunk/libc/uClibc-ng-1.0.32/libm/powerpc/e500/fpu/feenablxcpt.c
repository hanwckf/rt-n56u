/* Enable floating-point exceptions.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include "fenv_libc.h"
#include <syscall.h>
#include <sys/prctl.h>

int
feenableexcept (int excepts)
{
  unsigned int result = 0, pflags, r;
  INTERNAL_SYSCALL_DECL (err);

  INTERNAL_SYSCALL (prctl, err, 2, PR_GET_FPEXC, &pflags);

  /* Save old enable bits.  */
  if (pflags & PR_FP_EXC_OVF)
    result |= FE_OVERFLOW;
  if (pflags & PR_FP_EXC_UND)
    result |= FE_UNDERFLOW;
  if (pflags & PR_FP_EXC_INV)
    result |= FE_INVALID;
  if (pflags & PR_FP_EXC_DIV)
    result |= FE_DIVBYZERO;
  if (pflags & PR_FP_EXC_RES)
    result |= FE_INEXACT;

  if (excepts & FE_INEXACT)
    pflags |= PR_FP_EXC_RES;
  if (excepts & FE_DIVBYZERO)
    pflags |= PR_FP_EXC_DIV;
  if (excepts & FE_UNDERFLOW)
    pflags |= PR_FP_EXC_UND;
  if (excepts & FE_OVERFLOW)
    pflags |= PR_FP_EXC_OVF;
  if (excepts & FE_INVALID)
    pflags |= PR_FP_EXC_INV;
  r = INTERNAL_SYSCALL (prctl, err, 2, PR_SET_FPEXC, pflags);
  if (INTERNAL_SYSCALL_ERROR_P (r, err))
    return -1;

  return result;
}
