/* Machine-dependent pthreads configuration and inline functions.
   OpenRISC version.
   Copyright (C) 2001, 2002, 2003 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   see <http://www.gnu.org/licenses/>.  */

#ifndef _PT_MACHINE_H
#define _PT_MACHINE_H   1

#include <features.h>
#include <sys/syscall.h>

#ifndef PT_EI
# define PT_EI __extern_always_inline
#endif

PT_EI long int testandset(int*);

#define OR1K_ATOMIC_XCHG 1

PT_EI long int
testandset (int *spinlock)
{
	int oldvalue = 1;

	INLINE_SYSCALL(or1k_atomic, 3, OR1K_ATOMIC_XCHG, spinlock, &oldvalue);

	return (oldvalue);
}

/* Get some notion of the current stack.  Need not be exactly the top
   of the stack, just something somewhere in the current frame.  */
#define CURRENT_STACK_FRAME  stack_pointer()
static inline char *stack_pointer(void)
{
	unsigned long ret;
	__asm__ __volatile__ ("l.ori %0, r1, 0" : "=r" (ret));
	return (char *)ret;
}
#endif /* pt-machine.h */
