/* Install given floating-point environment and raise exceptions.
   Copyright (C) 1997,99,2000,01,07 Free Software Foundation, Inc.
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

libm_hidden_proto(fesetenv)
libm_hidden_proto(feraiseexcept)

int
feupdateenv (const fenv_t *envp)
{
  fexcept_t temp;
  /* unsigned int xtemp = 0; */

  /* Save current exceptions.  */
  __asm__ ("fnstsw %0" : "=m" (*&temp));

#if 0
  /* If the CPU supports SSE we test the MXCSR as well.  */
  if ((GLRO(dl_hwcap) & HWCAP_I386_XMM) != 0)
    __asm__ ("stmxcsr %0" : "=m" (*&xtemp));

  temp = (temp | xtemp) & FE_ALL_EXCEPT;
#else
  temp &= FE_ALL_EXCEPT;
#endif

  /* Install new environment.  */
  fesetenv (envp);

  /* Raise the saved exception.  Incidently for us the implementation
     defined format of the values in objects of type fexcept_t is the
     same as the ones specified using the FE_* constants.  */
  feraiseexcept ((int) temp);

  /* Success.  */
  return 0;
}
