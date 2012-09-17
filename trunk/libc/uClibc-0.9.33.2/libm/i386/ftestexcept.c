/* Test exception in current environment.
   Copyright (C) 1997, 2003, 2004 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <fenv.h>
#include <unistd.h>

int
fetestexcept (int excepts)
{
  short temp;
  int xtemp = 0;

  /* Get current exceptions.  */
  __asm__ ("fnstsw %0" : "=a" (temp));

#if 0
  /* If the CPU supports SSE we test the MXCSR as well.  */
  if ((GLRO(dl_hwcap) & HWCAP_I386_XMM) != 0)
    __asm__ ("stmxcsr %0" : "=m" (*&xtemp));
#endif

  return (temp | xtemp) & excepts & FE_ALL_EXCEPT;
}
