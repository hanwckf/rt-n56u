/* Machine-dependent pthreads configuration and inline functions.
   m68k version.
   Copyright (C) 1996, 1998, 2000, 2002, 2003 Free Software Foundation, Inc.
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
   not, write to the Free Software Foundation, Inc.,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

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

  __asm__ __volatile__("tas %1; sne %0"
       : "=dm"(ret), "=m"(*spinlock)
       : "m"(*spinlock)
       : "cc");

  return ret;
}


/* Get some notion of the current stack.  Need not be exactly the top
   of the stack, just something somewhere in the current frame.  */
#define CURRENT_STACK_FRAME  stack_pointer
register char * stack_pointer __asm__ ("%sp");


/* Compare-and-swap for semaphores. */

#define HAS_COMPARE_AND_SWAP
PT_EI int
__compare_and_swap (long int *p, long int oldval, long int newval)
{
  char ret;
  long int readval;

  __asm__ __volatile__ ("casl %2, %3, %1; seq %0"
			: "=dm" (ret), "=m" (*p), "=d" (readval)
			: "d" (newval), "m" (*p), "2" (oldval));

  return ret;
}

#endif /* pt-machine.h */
