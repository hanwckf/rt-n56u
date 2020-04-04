/* Machine-dependent pthreads configuration and inline functions.
 *
 * Copyright (C) 2005-2007 Atmel Corporation
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License.  See the file "COPYING.LIB" in the main directory of this
 * archive for more details.
 */
#ifndef _PT_MACHINE_H
#define _PT_MACHINE_H   1

#include <features.h>

#ifndef PT_EI
# define PT_EI __extern_always_inline
#endif

static __inline__ int
_test_and_set (int *p, int v)
{
	int result;

	__asm__ __volatile__(
		"/* Inline test and set */\n"
		"	xchg	%[old], %[mem], %[new]"
		: [old] "=&r"(result)
		: [mem] "r"(p), [new] "r"(v)
		: "memory");

	return result;
}

extern long int testandset (int *spinlock);
extern int __compare_and_swap (long int *p, long int oldval, long int newval);

/* Spinlock implementation; required.  */
PT_EI long int
testandset (int *spinlock)
{
	return _test_and_set(spinlock, 1);
}


/* Get some notion of the current stack.  Need not be exactly the top
   of the stack, just something somewhere in the current frame.  */
#define CURRENT_STACK_FRAME  stack_pointer
register char * stack_pointer __asm__ ("sp");

/* Compare-and-swap for semaphores. */

#define HAS_COMPARE_AND_SWAP
PT_EI int
__compare_and_swap(long int *p, long int oldval, long int newval)
{
	long int result;

	__asm__ __volatile__(
		"/* Inline compare and swap */\n"
		"1:	ssrf	5\n"
		"	ld.w	%[result], %[mem]\n"
		"	eor	%[result], %[old]\n"
		"	brne	2f\n"
		"	stcond	%[mem], %[new]\n"
		"	brne	1b\n"
		"2:"
		: [result] "=&r"(result), [mem] "=m"(*p)
		: "m"(*p), [new] "r"(newval), [old] "r"(oldval)
		: "cc", "memory");

	return result == 0;
}

#endif /* pt-machine.h */
