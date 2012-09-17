/* Copyright (C) 1999, 2000 Free Software Foundation, Inc.
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


/* Define bits representing the exception.  We use the bit positions of
   the appropriate bits in the FPSR...  (Tahoe EAS 2.4 5-4)*/

enum
  {
    FE_INEXACT =	1UL << 5,
#define FE_INEXACT	FE_INEXACT

    FE_UNDERFLOW =	1UL << 4,
#define FE_UNDERFLOW	FE_UNDERFLOW

    FE_OVERFLOW =	1UL << 3,
#define FE_OVERFLOW	FE_OVERFLOW

    FE_DIVBYZERO =	1UL << 2,
#define FE_DIVBYZERO	FE_DIVBYZERO

    FE_UNNORMAL =	1UL << 1,
#define FE_UNNORMAL	FE_UNNORMAL

    FE_INVALID =	1UL << 0,
#define FE_INVALID	FE_INVALID

    FE_ALL_EXCEPT =
	(FE_INEXACT | FE_UNDERFLOW | FE_OVERFLOW | FE_DIVBYZERO | FE_UNNORMAL | FE_INVALID)
#define FE_ALL_EXCEPT	FE_ALL_EXCEPT
  };


enum
  {
    FE_TOWARDZERO =	3,
#define FE_TOWARDZERO	FE_TOWARDZERO

    FE_UPWARD =		2,
#define FE_UPWARD	FE_UPWARD

    FE_DOWNWARD = 	1,
#define FE_DOWNWARD	FE_DOWNWARD

    FE_TONEAREST =	0,
#define FE_TONEAREST	FE_TONEAREST
  };


/* Type representing exception flags.  */
typedef unsigned long int fexcept_t;

/* Type representing floating-point environment.  */
typedef unsigned long int fenv_t;

/* If the default argument is used we use this value.  */
#define FE_DFL_ENV	((__const fenv_t *) 0xc009804c0270033fUL)

#ifdef __USE_GNU
/* Floating-point environment where only FE_UNNORMAL is masked since this
   exception is not generally supported by glibc.  */
# define FE_NOMASK_ENV	((__const fenv_t *) 0xc009804c02700302UL)

/* Floating-point environment with (processor-dependent) non-IEEE
   floating point.  In this case, turning on flush-to-zero mode for
   s0, s2, and s3.  */
# define FE_NONIEEE_ENV ((__const fenv_t *) 0xc009a04d0270037fUL)
#endif
