/* Machine-dependent pthreads configuration and inline functions.
   ARM version.
   Copyright (C) 1997, 1998, 2000, 2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Philip Blundell <philb@gnu.org>.

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
#define _PT_MACHINE_H   1

#include <features.h>

#ifndef PT_EI
# define PT_EI __extern_always_inline
#endif

/* This will not work on ARM1 or ARM2 because SWP is lacking on those
   machines.  Unfortunately we have no way to detect this at compile
   time; let's hope nobody tries to use one.  */

/* Spinlock implementation; required.  */
PT_EI long int testandset (int *spinlock);
PT_EI long int testandset (int *spinlock)
{
  register unsigned int ret;

#if defined(__thumb__)
  void *pc;
  __asm__ __volatile__(
	".align 0\n"
	"\tbx pc\n"
	"\tnop\n"
	"\t.arm\n"
	"\tswp %0, %2, [%3]\n"
	"\torr %1, pc, #1\n"
	"\tbx %1\n"
	"\t.force_thumb"
	: "=r"(ret), "=r"(pc)
	: "0"(1), "r"(spinlock));
#else
  __asm__ __volatile__("swp %0, %1, [%2]"
		       : "=r"(ret)
		       : "0"(1), "r"(spinlock));
#endif

  return ret;
}


/* Get some notion of the current stack.  Need not be exactly the top
   of the stack, just something somewhere in the current frame.  */
#define CURRENT_STACK_FRAME  stack_pointer
register char * stack_pointer __asm__ ("sp");

#endif /* pt-machine.h */
