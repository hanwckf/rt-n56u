/* Machine-dependent pthreads configuration and inline functions.
   Xtensa version.

   Copyright (C) 2007 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _PT_MACHINE_H
#define _PT_MACHINE_H   1

#include <sys/syscall.h>
#include <asm/unistd.h>

#ifndef PT_EI
# define PT_EI extern inline __attribute__ ((gnu_inline))
#endif

#define MEMORY_BARRIER() __asm__ ("memw" : : : "memory")
#define HAS_COMPARE_AND_SWAP

extern long int testandset (int *spinlock);
extern int __compare_and_swap (long int *p, long int oldval, long int newval);

/* Spinlock implementation; required.  */
PT_EI long int
testandset (int *spinlock)
{
	unsigned long tmp;
	__asm__ volatile (
"	movi	%0, 0			\n"
"	wsr	%0, SCOMPARE1		\n"
"	movi	%0, 1			\n"
"	s32c1i	%0, %1, 0		\n"
	: "=&a" (tmp)
	: "a" (spinlock)
	: "memory"
	);
	return tmp;
}

PT_EI int
__compare_and_swap (long int *p, long int oldval, long int newval)
{
        unsigned long tmp;
        unsigned long value;
        __asm__ volatile (
"1:     l32i    %0, %2, 0            \n"
"       bne     %0, %4, 2f           \n"
"       wsr     %0, SCOMPARE1        \n"
"       mov     %1, %0               \n"
"       mov     %0, %3               \n"
"       s32c1i  %0, %2, 0            \n"
"       bne     %1, %0, 1b           \n"
"2:                                  \n"
          : "=&a" (tmp), "=&a" (value)
          : "a" (p), "a" (newval), "a" (oldval)
          : "memory" );

        return tmp == oldval;
}

/* Get some notion of the current stack.  Need not be exactly the top
   of the stack, just something somewhere in the current frame.  */
#define CURRENT_STACK_FRAME __builtin_frame_address (0)

#endif /* _PT_MACHINE_H */
