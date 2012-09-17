/* Set current rounding direction.
   Copyright (C) 1997, 2003, 2004, 2005 Free Software Foundation, Inc.
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
fesetround (int round)
{
  unsigned short int cw;

  if ((round & ~0xc00) != 0)
    /* ROUND is no valid rounding mode.  */
    return 1;

  __asm__ ("fnstcw %0" : "=m" (*&cw));
  cw &= ~0xc00;
  cw |= round;
  __asm__ ("fldcw %0" : : "m" (*&cw));

#if 0
  /* If the CPU supports SSE we set the MXCSR as well.  */
  if ((GLRO(dl_hwcap) & HWCAP_I386_XMM) != 0)
    {
      unsigned int xcw;

      __asm__ ("stmxcsr %0" : "=m" (*&xcw));
      xcw &= ~0x6000;
      xcw |= round << 3;
      __asm__ ("ldmxcsr %0" : : "m" (*&xcw));
    }
#endif

  return 0;
}
