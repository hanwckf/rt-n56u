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
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <http://www.gnu.org/licenses/>.  */

#ifndef _PT_MACHINE_H
#define _PT_MACHINE_H   1

#include <sys/syscall.h>
#include <unistd.h>

#ifndef PT_EI
# define PT_EI __extern_always_inline
#endif

#if defined(__thumb2__)
PT_EI long int ldrex(int *spinlock)
{
	long int ret;
	__asm__ __volatile__(
		"ldrex %0, [%1]\n"
		: "=r"(ret)
		: "r"(spinlock) : "memory");
	return ret;
}

PT_EI long int strex(int val, int *spinlock)
{
	long int ret;
	__asm__ __volatile__(
		"strex %0, %1, [%2]\n"
		: "=r"(ret)
		: "r" (val), "r"(spinlock) : "memory");
	return ret;
}

/* Spinlock implementation; required.  */
PT_EI long int
testandset (int *spinlock)
{
  register unsigned int ret;

  do {
	  ret = ldrex(spinlock);
  } while (strex(1, spinlock));

  return ret;
}

#elif defined(__thumb__)

/* This will not work on ARM1 or ARM2 because SWP is lacking on those
   machines.  Unfortunately we have no way to detect this at compile
   time; let's hope nobody tries to use one.  */

/* Spinlock implementation; required.  */
PT_EI long int testandset (int *spinlock);
PT_EI long int testandset (int *spinlock)
{
  register unsigned int ret;
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
  return ret;
}

#else /* __thumb__ */

PT_EI long int testandset (int *spinlock);
PT_EI long int testandset (int *spinlock)
{
  register unsigned int ret;
  __asm__ __volatile__("swp %0, %1, [%2]"
		       : "=r"(ret)
		       : "0"(1), "r"(spinlock));
  return ret;
}
#endif

/* Get some notion of the current stack.  Need not be exactly the top
   of the stack, just something somewhere in the current frame.  */
#define CURRENT_STACK_FRAME  stack_pointer
register char * stack_pointer __asm__ ("sp");

#endif /* pt-machine.h */
