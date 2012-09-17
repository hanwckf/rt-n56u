/* Copyright (C) 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by David Huggins-Daines <dhd@debian.org>

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

/* Define bits representing the exception.  We use the values of the
   appropriate enable bits in the FPU status word (which,
   coincidentally, are the same as the flag bits, but shifted right by
   27 bits).  */
enum
{
  FE_INVALID   = 1<<4, /* V */
#define FE_INVALID	FE_INVALID
  FE_DIVBYZERO = 1<<3, /* Z */
#define FE_DIVBYZERO	FE_DIVBYZERO
  FE_OVERFLOW  = 1<<2, /* O */
#define FE_OVERFLOW	FE_OVERFLOW
  FE_UNDERFLOW = 1<<1, /* U */
#define FE_UNDERFLOW	FE_UNDERFLOW
  FE_INEXACT   = 1<<0, /* I */
#define FE_INEXACT	FE_INEXACT
};

#define FE_ALL_EXCEPT \
	(FE_INEXACT | FE_DIVBYZERO | FE_UNDERFLOW | FE_OVERFLOW | FE_INVALID)

/* The PA-RISC FPU supports all of the four defined rounding modes.
   We use the values of the RM field in the floating point status
   register for the appropriate macros.  */
enum
  {
    FE_TONEAREST  = 0 << 9,
#define FE_TONEAREST	FE_TONEAREST
    FE_TOWARDZERO = 1 << 9,
#define FE_TOWARDZERO	FE_TOWARDZERO
    FE_UPWARD     = 2 << 9,
#define FE_UPWARD	FE_UPWARD
    FE_DOWNWARD   = 3 << 9,
#define FE_DOWNWARD	FE_DOWNWARD
  };

/* Type representing exception flags. */
typedef unsigned int fexcept_t;

/* Type representing floating-point environment.  This structure
   corresponds to the layout of the status and exception words in the
   register file. */
typedef struct
{
  unsigned int __status_word;
  unsigned int __exception[7];
} fenv_t;

/* If the default argument is used we use this value.  */
#define FE_DFL_ENV ((fenv_t *) -1)

#ifdef __USE_GNU
/* Floating-point environment where none of the exceptions are masked.  */
# define FE_NOMASK_ENV	((fenv_t *) -2)
#endif
