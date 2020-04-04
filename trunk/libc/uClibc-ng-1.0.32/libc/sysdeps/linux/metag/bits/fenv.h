/* Copyright (C) 2013 Imagination Technologies Ltd.
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

#ifndef _FENV_H
# error "Never use <bits/fenv.h> directly; include <fenv.h> instead."
#endif


/* Define bits representing the exception.  We use the bit positions
   of the appropriate bits in TXDEFR.  */
enum
  {
    FE_INEXACT = 0x1,
#define FE_INEXACT	FE_INEXACT
    FE_UNDERFLOW = 0x2,
#define FE_UNDERFLOW	FE_UNDERFLOW
    FE_OVERFLOW = 0x4,
#define FE_OVERFLOW	FE_OVERFLOW
    FE_DIVBYZERO = 0x8,
#define FE_DIVBYZERO	FE_DIVBYZERO
    FE_INVALID = 0x10,
#define FE_INVALID	FE_INVALID
  };

#define FE_ALL_EXCEPT \
	(FE_INEXACT | FE_DIVBYZERO | FE_UNDERFLOW | FE_OVERFLOW | FE_INVALID)

/* The Meta FPU supports all of the four defined rounding modes.  We
   use the values of the rounding mode bits in TXMODE as the values
   for the appropriate macros.  */
enum
  {
    FE_TONEAREST = 0x0,
#define FE_TONEAREST	FE_TONEAREST
    FE_TOWARDZERO = 0x1,
#define FE_TOWARDZERO	FE_TOWARDZERO
    FE_UPWARD = 0x2,
#define FE_UPWARD	FE_UPWARD
    FE_DOWNWARD = 0x3
#define FE_DOWNWARD	FE_DOWNWARD
  };


/* Type representing exception flags.  */
typedef unsigned int fexcept_t;


/* Type representing floating-point environment.  */
typedef struct
  {
    unsigned int txdefr;
    unsigned int txmode;
  }
fenv_t;

/* If the default argument is used we use this value.  */
#define FE_DFL_ENV	((__const fenv_t *) -1)

#ifdef __USE_GNU
/* Floating-point environment where none of the exception is masked.  */
# define FE_NOMASK_ENV  ((__const fenv_t *) -2)
#endif
