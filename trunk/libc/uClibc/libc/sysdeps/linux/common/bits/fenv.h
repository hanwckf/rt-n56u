/* Copyright (C) 1997, 1998, 1999, 2000, 2001 Free Software Foundation, Inc.
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


/* Here should be the exception be defined:
    FE_INVALID
    FE_DIVBYZERO
    FE_OVERFLOW
    FE_UNDERFLOW
    FE_INEXACT
   We define no macro which signals no exception is supported.  */

#define FE_ALL_EXCEPT 0


/* Here should the rounding modes be defined:
    FE_TONEAREST
    FE_DOWNWARD
    FE_UPWARD
    FE_TOWARDZERO
   We define no macro which signals no rounding mode is selectable.  */


/* Type representing exception flags.  */
typedef unsigned int fexcept_t;


/* Type representing floating-point environment.  */
typedef struct
  {
    fexcept_t __excepts;
    /* XXX I don't know what else we should save.  */
  }
fenv_t;

/* If the default argument is used we use this value.  */
#define FE_DFL_ENV	((__const fenv_t *) -1l)
