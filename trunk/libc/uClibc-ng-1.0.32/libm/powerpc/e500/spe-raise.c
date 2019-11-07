/* Raise given exceptions.
   Copyright (C) 1997,99,2000,01,02,04 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include "fpu/fenv_libc.h"

int
__FERAISEEXCEPT_INTERNAL (int excepts)
{
  unsigned long f;

  f = fegetenv_register ();
  f |= (excepts & FE_ALL_EXCEPT);
  fesetenv_register (f);

  /* Force the operations that cause the exceptions.  */
  if ((FE_INVALID & excepts) != 0)
    {
      /* ?? Does not set sticky bit ?? */
      /* 0 / 0 */
      __asm__ __volatile__ ("efsdiv %0,%0,%1" : : "r" (0), "r" (0));
    }

  if ((FE_DIVBYZERO & excepts) != 0)
    {
      /* 1.0 / 0.0 */
      __asm__ __volatile__ ("efsdiv %0,%0,%1" : : "r" (1.0F), "r" (0));
    }

  if ((FE_OVERFLOW & excepts) != 0)
    {
      /* ?? Does not set sticky bit ?? */
      /* Largest normalized number plus itself.  */
      __asm__ __volatile__ ("efsadd %0,%0,%1" : : "r" (0x7f7fffff), "r" (0x7f7fffff));
    }

  if ((FE_UNDERFLOW & excepts) != 0)
    {
      /* ?? Does not set sticky bit ?? */
      /* Smallest normalized number times itself.  */
      __asm__ __volatile__ ("efsmul %0,%0,%1" : : "r" (0x800000), "r" (0x800000));
    }

  if ((FE_INEXACT & excepts) != 0)
    {
      /* Smallest normalized minus 1.0 raises the inexact flag.  */
      __asm__ __volatile__ ("efssub %0,%0,%1" : : "r" (0x00800000), "r" (1.0F));
    }

  /* Success.  */
  return 0;
}
