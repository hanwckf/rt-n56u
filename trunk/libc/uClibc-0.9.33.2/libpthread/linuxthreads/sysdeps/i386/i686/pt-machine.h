/* Machine-dependent pthreads configuration and inline functions.
   i686 version.
   Copyright (C) 1996-2001, 2002, 2003 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#ifndef _PT_MACHINE_H
#define _PT_MACHINE_H	1

#ifndef PT_EI
# define PT_EI __extern_always_inline
#endif
#include <bits/kernel-features.h>

#ifndef __ASSEMBLER__
extern long int testandset (int *spinlock);
extern int __compare_and_swap (long int *p, long int oldval, long int newval);

/* Get some notion of the current stack.  Need not be exactly the top
   of the stack, just something somewhere in the current frame.  */
#define CURRENT_STACK_FRAME  __builtin_frame_address (0)


/* Spinlock implementation; required.  */
PT_EI long int
testandset (int *spinlock)
{
  long int ret;

  __asm__ __volatile__ (
	"xchgl %0, %1"
	: "=r" (ret), "=m" (*spinlock)
	: "0" (1), "m" (*spinlock)
	: "memory");

  return ret;
}


/* Compare-and-swap for semaphores.  It's always available on i686.  */
#define HAS_COMPARE_AND_SWAP

PT_EI int
__compare_and_swap (long int *p, long int oldval, long int newval)
{
  char ret;
  long int readval;

  __asm__ __volatile__ ("lock; cmpxchgl %3, %1; sete %0"
			: "=q" (ret), "=m" (*p), "=a" (readval)
			: "r" (newval), "m" (*p), "a" (oldval)
			: "memory");
  return ret;
}
#endif

#if __ASSUME_LDT_WORKS > 0
#include "../useldt.h"
#endif

/* The P4 and above really want some help to prevent overheating.  */
#define BUSY_WAIT_NOP	__asm__ ("rep; nop")

#endif /* pt-machine.h */
