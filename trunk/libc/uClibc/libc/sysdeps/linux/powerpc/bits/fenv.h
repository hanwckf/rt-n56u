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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef _FENV_H
# error "Never use <bits/fenv.h> directly; include <fenv.h> instead."
#endif


/* Define bits representing the exception.  We use the bit positions of
   the appropriate bits in the FPSCR...  */
enum
  {
    FE_INEXACT = 1 << (31 - 6),
#define FE_INEXACT	FE_INEXACT
    FE_DIVBYZERO = 1 << (31 - 5),
#define FE_DIVBYZERO	FE_DIVBYZERO
    FE_UNDERFLOW = 1 << (31 - 4),
#define FE_UNDERFLOW	FE_UNDERFLOW
    FE_OVERFLOW = 1 << (31 - 3),
#define FE_OVERFLOW	FE_OVERFLOW

    /* ... except for FE_INVALID, for which we use bit 31. FE_INVALID
       actually corresponds to bits 7 through 12 and 21 through 23
       in the FPSCR, but we can't use that because the current draft
       says that it must be a power of 2.  Instead we use bit 2 which
       is the summary bit for all the FE_INVALID exceptions, which
       kind of makes sense.  */
    FE_INVALID = 1 << (31 - 2),
#define FE_INVALID	FE_INVALID

#ifdef __USE_GNU
    /* Breakdown of the FE_INVALID bits. Setting FE_INVALID on an
       input to a routine is equivalent to setting all of these bits;
       FE_INVALID will be set on output from a routine iff one of
       these bits is set.  Note, though, that you can't disable or
       enable these exceptions individually.  */

    /* Operation with SNaN. */
    FE_INVALID_SNAN = 1 << (31 - 7),
# define FE_INVALID_SNAN	FE_INVALID_SNAN

    /* Inf - Inf */
    FE_INVALID_ISI = 1 << (31 - 8),
# define FE_INVALID_ISI		FE_INVALID_ISI

    /* Inf / Inf */
    FE_INVALID_IDI = 1 << (31 - 9),
# define FE_INVALID_IDI		FE_INVALID_IDI

    /* 0 / 0 */
    FE_INVALID_ZDZ = 1 << (31 - 10),
# define FE_INVALID_ZDZ		FE_INVALID_ZDZ

    /* Inf * 0 */
    FE_INVALID_IMZ = 1 << (31 - 11),
# define FE_INVALID_IMZ		FE_INVALID_IMZ

    /* Comparison with NaN or SNaN.  */
    FE_INVALID_COMPARE = 1 << (31 - 12),
# define FE_INVALID_COMPARE	FE_INVALID_COMPARE

    /* Invalid operation flag for software (not set by hardware).  */
    /* Note that some chips don't have this implemented, presumably
       because no-one expected anyone to write software for them %-).  */
    FE_INVALID_SOFTWARE = 1 << (31 - 21),
# define FE_INVALID_SOFTWARE	FE_INVALID_SOFTWARE

    /* Square root of negative number (including -Inf).  */
    /* Note that some chips don't have this implemented.  */
    FE_INVALID_SQRT = 1 << (31 - 22),
# define FE_INVALID_SQRT	FE_INVALID_SQRT

    /* Conversion-to-integer of a NaN or a number too large or too small.  */
    FE_INVALID_INTEGER_CONVERSION = 1 << (31 - 23)
# define FE_INVALID_INTEGER_CONVERSION	FE_INVALID_INTEGER_CONVERSION

# define FE_ALL_INVALID \
        (FE_INVALID_SNAN | FE_INVALID_ISI | FE_INVALID_IDI | FE_INVALID_ZDZ \
	 | FE_INVALID_IMZ | FE_INVALID_COMPARE | FE_INVALID_SOFTWARE \
	 | FE_INVALID_SQRT | FE_INVALID_INTEGER_CONVERSION)
#endif
  };

#define FE_ALL_EXCEPT \
	(FE_INEXACT | FE_DIVBYZERO | FE_UNDERFLOW | FE_OVERFLOW | FE_INVALID)

/* PowerPC chips support all of the four defined rounding modes.  We
   use the bit pattern in the FPSCR as the values for the
   appropriate macros.  */
enum
  {
    FE_TONEAREST = 0,
#define FE_TONEAREST	FE_TONEAREST
    FE_TOWARDZERO = 1,
#define FE_TOWARDZERO	FE_TOWARDZERO
    FE_UPWARD = 2,
#define FE_UPWARD	FE_UPWARD
    FE_DOWNWARD = 3
#define FE_DOWNWARD	FE_DOWNWARD
  };

/* Type representing exception flags.  */
typedef unsigned int fexcept_t;

/* Type representing floating-point environment.  We leave it as 'double'
   for efficiency reasons (rather than writing it to a 32-bit integer). */
typedef double fenv_t;

/* If the default argument is used we use this value.  */
extern const fenv_t __fe_dfl_env;
#define FE_DFL_ENV	(&__fe_dfl_env)

#ifdef __USE_GNU
/* Floating-point environment where all exceptions are enabled.  Note that
   this is not sufficient to give you SIGFPE.  */
extern const fenv_t __fe_enabled_env;
# define FE_ENABLED_ENV	(&__fe_enabled_env)

/* Floating-point environment with (processor-dependent) non-IEEE floating
   point.  */
extern const fenv_t __fe_nonieee_env;
# define FE_NONIEEE_ENV	(&__fe_nonieee_env)

/* Floating-point environment with all exceptions enabled.  Note that
   just evaluating this value will set the processor into 'FPU
   exceptions imprecise recoverable' mode, which may cause a significant
   performance penalty (but have no other visible effect).  */
extern const fenv_t *__fe_nomask_env (void);
# define FE_NOMASK_ENV	(__fe_nomask_env ())
#endif
