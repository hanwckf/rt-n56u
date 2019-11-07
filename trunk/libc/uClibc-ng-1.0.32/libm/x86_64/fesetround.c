/* Set current rounding direction.
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
fesetround (int round)
{
  unsigned short int cw;
  int mxcsr;

  if ((round & ~0xc00) != 0)
    /* ROUND is no valid rounding mode.  */
    return 1;

  /* First set the x87 FPU.  */
  __asm__ ("fnstcw %0" : "=m" (*&cw));
  cw &= ~0xc00;
  cw |= round;
  __asm__ ("fldcw %0" : : "m" (*&cw));

  /* And now the MSCSR register for SSE, the precision is at different bit
     positions in the different units, we need to shift it 3 bits.  */
  __asm__ ("stmxcsr %0" : "=m" (*&mxcsr));
  mxcsr &= ~ 0x6000;
  mxcsr |= round << 3;
  __asm__ ("ldmxcsr %0" : : "m" (*&mxcsr));

  return 0;
}
