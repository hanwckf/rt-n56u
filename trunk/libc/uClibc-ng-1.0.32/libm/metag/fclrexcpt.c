/* Clear given exceptions in current floating-point environment.
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

#include "internal.h"

int
feclearexcept (int excepts)
{
  unsigned int temp;

  /* Get the current exceptions.  */
  __asm__ ("MOV %0,TXDEFR" : "=r" (temp));

  /* Mask out unsupported bits/exceptions.  */
  excepts &= FE_ALL_EXCEPT;

  excepts <<= 16;

  temp &= ~excepts;

  metag_set_fpu_flags(temp);

  /* Success.  */
  return 0;
}
