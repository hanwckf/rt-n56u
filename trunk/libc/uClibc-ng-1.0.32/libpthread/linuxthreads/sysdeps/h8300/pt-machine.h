/* Machine-dependent pthreads configuration and inline functions.
   H8/300 version.
   Copyright (C) 1996, 1998, 2000, 2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Richard Henderson <rth@tamu.edu>.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <http://www.gnu.org/licenses/>.  */

#ifndef _PT_MACHINE_H
#define _PT_MACHINE_H   1

#include <features.h>

#ifndef PT_EI
# define PT_EI __extern_always_inline
#endif

/* Spinlock implementation; required.  */
PT_EI long int
testandset (int *spinlock)
{
  char ret;

  __asm__ __volatile__(
      "sub.w %0,%0\n\t"
      "stc ccr,@-sp\n\t"
      "orc #0x80,ccr\n\t"
      "bld #0,@%2\n\t"
      "bset #0,@%2\n\t"
      "rotxl.w %0\n\t"
      "ldc @sp+,ccr\n\t"
      :"=r"(ret),"=m"(*spinlock)
      :"g"(spinlock)
      :"cc");

  return ret;
}


/* Get some notion of the current stack.  Need not be exactly the top
   of the stack, just something somewhere in the current frame.  */
#define CURRENT_STACK_FRAME  stack_pointer
register char * stack_pointer __asm__ ("%sp");

#endif /* pt-machine.h */
