/* Copyright (C) 1997, 1998, 1999, 2000 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef _FENV_H
# error "Never use <bits/fenv.h> directly; include <fenv.h> instead."
#endif

#include <bits/wordsize.h>


/* Define bits representing the exception.  We use the bit positions
   of the appropriate accrued exception bits from the FSR.  */
enum
  {
    FE_INVALID = 	(1 << 9),
#define FE_INVALID	FE_INVALID
    FE_OVERFLOW = 	(1 << 8),
#define FE_OVERFLOW	FE_OVERFLOW
    FE_UNDERFLOW = 	(1 << 7),
#define FE_UNDERFLOW	FE_UNDERFLOW
    FE_DIVBYZERO = 	(1 << 6),
#define FE_DIVBYZERO	FE_DIVBYZERO
    FE_INEXACT = 	(1 << 5)
#define FE_INEXACT	FE_INEXACT
  };

#define FE_ALL_EXCEPT \
	(FE_INEXACT | FE_DIVBYZERO | FE_UNDERFLOW | FE_OVERFLOW | FE_INVALID)

/* The Sparc FPU supports all of the four defined rounding modes.  We
   use again the bit positions in the FPU control word as the values
   for the appropriate macros.  */
enum
  {
    FE_TONEAREST = 	(0U << 30),
#define FE_TONEAREST	FE_TONEAREST
    FE_TOWARDZERO = 	(1U << 30),
#define FE_TOWARDZERO	FE_TOWARDZERO
    FE_UPWARD = 	(2U << 30),
#define FE_UPWARD	FE_UPWARD
    FE_DOWNWARD = 	(3U << 30)
#define FE_DOWNWARD	FE_DOWNWARD
  };

#define __FE_ROUND_MASK	(3U << 30)


/* Type representing exception flags.  */
typedef unsigned long int fexcept_t;


/* Type representing floating-point environment.  */
typedef unsigned long int fenv_t;

/* If the default argument is used we use this value.  */
#define FE_DFL_ENV	((__const fenv_t *) -1)

#ifdef __USE_GNU
/* Floating-point environment where none of the exception is masked.  */
# define FE_NOMASK_ENV	((__const fenv_t *) -2)
#endif

/* For internal use only: access the fp state register.  */
#if __WORDSIZE == 64
# define __fenv_stfsr(X)   __asm__ ("stx %%fsr,%0" : "=m" (X))
# define __fenv_ldfsr(X)   __asm__ __volatile__ ("ldx %0,%%fsr" : : "m" (X))
#else
# define __fenv_stfsr(X)   __asm__ ("st %%fsr,%0" : "=m" (X))
# define __fenv_ldfsr(X)   __asm__ __volatile__ ("ld %0,%%fsr" : : "m" (X))
#endif
