/* Store current representation for exceptions.
   Copyright (C) 1997,99,2000,01 Free Software Foundation, Inc.
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

int
fegetexceptflag (fexcept_t *flagp, int excepts)
{
  fexcept_t temp;

  /* Get the current exceptions.  */
  __asm__ ("fnstsw %0" : "=m" (*&temp));

  *flagp = temp & excepts & FE_ALL_EXCEPT;

  /* Success.  */
  return 0;
}
