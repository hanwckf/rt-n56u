/* Raise given exceptions.
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
#include <math.h>

libm_hidden_proto(feraiseexcept)

int
feraiseexcept (int excepts)
{
  /* Raise exceptions represented by EXPECTS.  But we must raise only
     one signal at a time.  It is important that if the overflow/underflow
     exception and the inexact exception are given at the same time,
     the overflow/underflow exception follows the inexact exception.  */

  /* First: invalid exception.  */
  if ((FE_INVALID & excepts) != 0)
    {
      /* Reciprocal square root of a negative number is invalid. */
      __asm__ volatile(
		   "F MOV FX.0,#0xc000 ! -2\n"
		   "F RSQ FX.1,FX.0\n"
		   );
    }

  /* Next: division by zero.  */
  if ((FE_DIVBYZERO & excepts) != 0)
    {
      __asm__ volatile(
		   "F MOV FX.0,#0\n"
		   "F RCP FX.1,FX.0\n"
		   );
    }

  /* Next: overflow.  */
  if ((FE_OVERFLOW & excepts) != 0)
    {
      /* Adding a large number in single precision can cause overflow. */
      __asm__ volatile(
		   "  MOVT D0.0,#0x7f7f\n"
		   "  ADD  D0.0,D0.0,#0xffff\n"
		   "F MOV  FX.0,D0.0\n"
		   "F ADD  FX.1,FX.0,FX.0\n"
		   );
    }

  /* Next: underflow.  */
  if ((FE_UNDERFLOW & excepts) != 0)
    {
      /* Multiplying a small value by 0.5 will cause an underflow. */
      __asm__ volatile(
		   "  MOV  D0.0,#1\n"
		   "F MOV  FX.0,D0.0\n"
		   "  MOVT D0.0,#0x3f00\n"
		   "F MOV  FX.1,D0.0\n"
		   "F MUL  FX.2,FX.1,FX.0\n"
		   );
    }

  /* Last: inexact.  */
  if ((FE_INEXACT & excepts) != 0)
    {
      /* Converting a small single precision value to half precision
	 can cause an inexact exception. */
      __asm__ volatile(
		   "  MOV  D0.0,#0x0001\n"
		   "F MOV  FX.0,D0.0\n"
		   "F FTOH FX.1,FX.0\n"
		   );
    }

  /* Success.  */
  return 0;
}
libm_hidden_def(feraiseexcept)
