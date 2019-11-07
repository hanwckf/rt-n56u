/* Store current floating-point environment and clear exceptions.
   Copyright (C) 2001-2017 Free Software Foundation, Inc.

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

#include <fenv.h>

int
feholdexcept (fenv_t *envp)
{
  unsigned int mxcsr;

  /* Store the environment.  Recall that fnstenv has a side effect of
     masking all exceptions.  Then clear all exceptions.  */
  __asm__ ("fnstenv %0\n\t"
	   "stmxcsr %1\n\t"
	   "fnclex"
	   : "=m" (*envp), "=m" (envp->__mxcsr));

  /* Set the SSE MXCSR register.  */
  mxcsr = (envp->__mxcsr | 0x1f80) & ~0x3f;
  __asm__ ("ldmxcsr %0" : : "m" (*&mxcsr));

  return 0;
}
