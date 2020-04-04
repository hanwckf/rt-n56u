/* Copyright (C) 1997, 1998, 1999 Free Software Foundation, Inc.
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

#ifndef _FENV_H
# error "Never use <bits/fenv.h> directly; include <fenv.h> instead."
#endif

#ifdef __MAVERICK__

/* Define bits representing exceptions in the FPU status word.  */
enum
  {
    FE_INVALID = 1,
#define FE_INVALID FE_INVALID
    FE_OVERFLOW = 4,
#define FE_OVERFLOW FE_OVERFLOW
    FE_UNDERFLOW = 8,
#define FE_UNDERFLOW FE_UNDERFLOW
    FE_INEXACT = 16,
#define FE_INEXACT FE_INEXACT
  };

/* Amount to shift by to convert an exception to a mask bit.  */
#define FE_EXCEPT_SHIFT    5

/* All supported exceptions.  */
#define FE_ALL_EXCEPT  \
	(FE_INVALID | FE_OVERFLOW | FE_UNDERFLOW | FE_INEXACT)

/* IEEE rounding modes.  */
enum
  {
    FE_TONEAREST = 0,
#define FE_TONEAREST    FE_TONEAREST
    FE_TOWARDZERO = 0x400,
#define FE_TOWARDZERO   FE_TOWARDZERO
    FE_DOWNWARD = 0x800,
#define FE_DOWNWARD     FE_DOWNWARD
    FE_UPWARD = 0xc00,
#define FE_UPWARD       FE_UPWARD
  };

#define FE_ROUND_MASK (FE_UPWARD)

#else /* !__MAVERICK__ */

/* Define bits representing exceptions in the FPU status word.  */
enum
  {
    FE_INVALID = 1,
#define FE_INVALID FE_INVALID
    FE_DIVBYZERO = 2,
#define FE_DIVBYZERO FE_DIVBYZERO
    FE_OVERFLOW = 4,
#define FE_OVERFLOW FE_OVERFLOW
    FE_UNDERFLOW = 8,
#define FE_UNDERFLOW FE_UNDERFLOW
  };

/* Amount to shift by to convert an exception to a mask bit.  */
#define FE_EXCEPT_SHIFT	16

/* All supported exceptions.  */
#define FE_ALL_EXCEPT	\
	(FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW | FE_UNDERFLOW)

/* The ARM FPU basically only supports round-to-nearest.  Other rounding
   modes exist, but you have to encode them in the actual instruction.  */
#define FE_TONEAREST	0

#endif /* __MAVERICK__ */

/* Type representing exception flags. */
typedef unsigned long int fexcept_t;

/* Type representing floating-point environment.  */
typedef struct
  {
    unsigned long int __cw;
  }
fenv_t;

/* If the default argument is used we use this value.  */
#define FE_DFL_ENV	((fenv_t *) -1l)
