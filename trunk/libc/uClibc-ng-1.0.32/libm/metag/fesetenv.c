/* Install given floating-point environment.
   Copyright (C) 2013 Imagination Technologies Ltd.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <http://www.gnu.org/licenses/>.  */

#include <fenv.h>
#include <assert.h>

#include "internal.h"

libm_hidden_proto(fesetenv)

int
fesetenv (const fenv_t *envp)
{
  unsigned int exc;
  unsigned int txmode;

  __asm__ ("MOV %0,TXMODE" : "=r" (txmode));

  /* Clear rounding mode bits (round to nearest).  */
  txmode &= ~(0x3 << 16);

  if (envp == FE_DFL_ENV)
    {
      exc = 0;
    }
  else if (envp == FE_NOMASK_ENV)
    {
      exc = 0x1f;
    }
  else
    {
      exc = envp->txdefr & (FE_ALL_EXCEPT | (FE_ALL_EXCEPT << 16));
      /* Write rounding mode and guard bit.  */
      txmode |= (0x1 << 18 ) | (envp->txmode & (0x3 << 16));
    }

  __asm__ ("MOV TXMODE,%0" : : "r" (txmode));

  metag_set_fpu_flags(exc);

  /* Success.  */
  return 0;
}
libm_hidden_def(fesetenv)
