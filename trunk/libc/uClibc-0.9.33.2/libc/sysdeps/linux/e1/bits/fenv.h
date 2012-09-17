
/*  Copyright (C) 2002-2003,    George Thanos <george.thanos@gdt.gr>
                                Yannis Mitsos <yannis.mitsos@gdt.gr>

   Copyright (C) 1997, 1998, 1999, 2000 Free Software Foundation, Inc.

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
   the appropriate bits in the SR.  */
enum
  {
    FE_INEXACT = (1 << 8),
#define FE_INEXACT	FE_INEXACT
    FE_UNDERFLOW = (1 << 9),
#define FE_UNDERFLOW	FE_UNDERFLOW
    FE_OVERFLOW = (1 << 10),
#define FE_OVERFLOW	FE_OVERFLOW
    FE_DIVBYZERO = (1 << 11),
#define FE_DIVBYZERO	FE_DIVBYZERO
    FE_INVALID = (1 << 12)
#define FE_INVALID	FE_INVALID
  };

#define FE_ALL_EXCEPT \
	(FE_INEXACT | FE_DIVBYZERO | FE_UNDERFLOW | FE_OVERFLOW | FE_INVALID)

/* We support all of the four defined rounding modes.  We use
   the bit positions in the FPCR Mode Control Byte as the values for the
   appropriate macros.  */
enum
  {
    FE_TONEAREST = 0,
#define FE_TONEAREST	FE_TONEAREST
    FE_TOWARDZERO = 1 << 13 ,
#define FE_TOWARDZERO	FE_TOWARDZERO
    FE_DOWNWARD = 2 << 13,
#define FE_DOWNWARD	FE_DOWNWARD
    FE_UPWARD = 3 << 13
#define FE_UPWARD	FE_UPWARD
  };


/* Type representing exception flags.  */
typedef unsigned int fexcept_t;


/* Type representing floating-point environment.*/
typedef struct
{
    unsigned int round_mode;
    unsigned int trap_enabled;
    unsigned int accrued_except;
    unsigned int actual_except;
} fenv_t;

#if 0
/* If the default argument is used we use this value.  */
const fenv FE_DFL_ENV_OBJ = {0, 0x1C00, 0}
#define FE_DFL_ENV	(&FE_DFL_ENV_OBJ)

#ifdef __USE_GNU
/* Floating-point environment where none of the exceptions are masked.  */
const fenv_t FE_NOMASK_ENV_OBJ = { 0, 0x1F00, 0 };
# define FE_NOMASK_ENV	(&FE_NOMASK_ENV_OBJ)
#endif

#endif

#include <bits/fenvinline.h>
