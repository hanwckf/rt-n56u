/* Store current floating-point environment and clear exceptions.
   Copyright (C) 1997, 1999, 2003, 2004, 2005, 2007
   Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

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
#include <unistd.h>

int
feholdexcept (fenv_t *envp)
{
  fenv_t temp;

  /* Store the environment.  */
  __asm__ ("fnstenv %0" : "=m" (temp));
  *envp = temp;

  /* Now set all exceptions to non-stop.  */
  temp.__control_word |= 0x3f;

  /* And clear all exceptions.  */
  temp.__status_word &= ~0x3f;

  __asm__ ("fldenv %0" : : "m" (temp));

#if 0
  /* If the CPU supports SSE we set the MXCSR as well.  */
  if ((GLRO(dl_hwcap) & HWCAP_I386_XMM) != 0)
    {
      unsigned int xwork;

      /* Get the current control word.  */
      __asm__ ("stmxcsr %0" : "=m" (*&xwork));

      /* Set all exceptions to non-stop and clear them.  */
      xwork = (xwork | 0x1f80) & ~0x3f;

      __asm__ ("ldmxcsr %0" : : "m" (*&xwork));
    }
#endif

  return 0;
}
