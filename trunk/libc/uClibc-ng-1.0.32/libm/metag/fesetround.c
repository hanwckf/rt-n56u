/* Set current rounding direction.
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
#include <unistd.h>

int
fesetround (int round)
{
  unsigned int txmode;

  if ((round & ~0x3) != 0)
    /* ROUND is no valid rounding mode.  */
    return 1;

  __asm__ ("MOV %0,TXMODE" : "=r" (txmode));

  txmode &= ~(0x3 << 16);
  /* Write rounding mode and guard bit.  */
  txmode |= (0x1 << 18 ) | (round << 16);

  __asm__ ("MOV TXMODE,%0" : : "r" (txmode));

  return 0;
}
