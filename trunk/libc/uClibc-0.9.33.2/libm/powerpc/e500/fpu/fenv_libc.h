/* Internal libc stuff for floating point environment routines.
   Copyright (C) 2004 Free Software Foundation, Inc.
   Contributed by Aldy Hernandez <aldyh@redhat.com>, 2004
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

#ifndef _FENV_LIBC_H
#define _FENV_LIBC_H	1

#include <fenv.h>

extern int __feraiseexcept_internal (int __excepts);

/* Equivalent to fegetenv, but returns a fenv_t instead of taking a
   pointer.  */
#define fegetenv_register() \
        ({ unsigned fscr; __asm__ __volatile__ ("mfspefscr %0" : "=r" (fscr)); fscr; })

/* Equivalent to fesetenv, but takes a fenv_t instead of a pointer.  */
#define fesetenv_register(fscr) \
	({ __asm__ __volatile__ ("mtspefscr %0" : : "r" (fscr)); })

typedef union
{
  fenv_t fenv;
  unsigned int l[2];
} fenv_union_t;

/* Definitions of all the SPEFSCR bit numbers.  */
enum {
  SPEFSCR_SOVH          = 0x80000000,
  SPEFSCR_OVH           = 0x40000000,
  SPEFSCR_FGH           = 0x20000000,
  SPEFSCR_FXH           = 0x10000000,
  SPEFSCR_FINVH         = 0x08000000,
  SPEFSCR_FDBZH         = 0x04000000,
  SPEFSCR_FUNFH         = 0x02000000,
  SPEFSCR_FOVFH         = 0x01000000,
  /* 2 unused bits.  */
  SPEFSCR_FINXS         = 0x00200000,
  SPEFSCR_FINVS         = 0x00100000,
  SPEFSCR_FDBZS         = 0x00080000,
  SPEFSCR_FUNFS         = 0x00040000,
  SPEFSCR_FOVFS         = 0x00020000,
  SPEFSCR_MODE          = 0x00010000,
  SPEFSCR_SOV           = 0x00008000,
  SPEFSCR_OV            = 0x00004000,
  SPEFSCR_FG            = 0x00002000,
  SPEFSCR_FX            = 0x00001000,
  SPEFSCR_FINV          = 0x00000800,
  SPEFSCR_FDBZ          = 0x00000400,
  SPEFSCR_FUNF          = 0x00000200,
  SPEFSCR_FOVF          = 0x00000100,
  /* 1 unused bit.  */
  SPEFSCR_FINXE         = 0x00000040,
  SPEFSCR_FINVE         = 0x00000020,
  SPEFSCR_FDBZE         = 0x00000010,
  SPEFSCR_FUNFE         = 0x00000008,
  SPEFSCR_FOVFE         = 0x00000004,
  SPEFSCR_FRMC          = 0x00000003
};

#endif /* fenv_libc.h */
