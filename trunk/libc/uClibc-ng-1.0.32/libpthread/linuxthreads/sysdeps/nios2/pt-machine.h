/* Machine-dependent pthreads configuration and inline functions.
   nios2 version.
   Copyright (C) 1996, 1998, 2000, 2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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
  unsigned int scratch;
  long int ret=-2;

  __asm__ __volatile__(
         "rdctl %0, status\n\t"
         "and	%0, %0, %1\n\t"
         "wrctl status, %0   #disable interrupts\n\t"
         "ldw %1, 0(%4)\n\t"
         "stw %3, 0(%4)\n\t"
         "ori	%0, %0, 1\n\t"
         "wrctl status, %0   #enable interrupts\n\t"
       : "=&r"(scratch), "=r"(ret)
       : "1"(ret), "r"(1), "r"(spinlock)
       : "memory");

  return ret;
}


/* Get some notion of the current stack.  Need not be exactly the top
   of the stack, just something somewhere in the current frame.  */
#define CURRENT_STACK_FRAME  stack_pointer
register char * stack_pointer __asm__ ("%sp");

#endif /* pt-machine.h */
