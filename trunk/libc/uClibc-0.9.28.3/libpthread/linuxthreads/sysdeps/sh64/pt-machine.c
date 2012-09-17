/* Cloned for uClibc by Paul Mundt, December 2003 */
/* Modified by SuperH, Inc. September 2003 */

/* Machine-dependent pthreads configuration and inline functions.
   SH5 version.
   Copyright (C) 1999, 2000, 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Niibe Yutaka <gniibe@m17n.org>.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include "pt-machine.h"

/* Spinlock implementation; required.  */

/* The SH5 does not have a suitable test-and-set instruction (SWAP only 
   operates on an aligned quad word). So we use the SH4 version instead.
   This must be seperately compiled in SHcompact mode, so it cannot be
   inline. */

long int testandset (int *spinlock)
{
  int ret;

  __asm__ __volatile__(
       "tas.b	@%1\n\t"
       "movt	%0"
       : "=r" (ret)
       : "r" (spinlock)
       : "memory", "cc");

  return (ret == 0);
}

