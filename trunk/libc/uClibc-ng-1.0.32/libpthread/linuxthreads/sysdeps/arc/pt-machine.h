/*
 * Copyright (C) 2013 Synopsys, Inc. (www.synopsys.com)
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

#ifndef _PT_MACHINE_H
#define _PT_MACHINE_H   1
#include <features.h>

#ifndef PT_EI
# define PT_EI __extern_always_inline
#endif

extern long int testandset (int *spinlock);
extern int __compare_and_swap (long int *p, long int oldval, long int newval);

PT_EI long int
testandset (int *spinlock)
{
	unsigned int old = 1;

	/* Atomically exchange @spinlock with 1 */
	__asm__ __volatile__(
	"ex %0, [%1]"
	: "+r" (old)
	: "r" (spinlock)
	: "memory");

  return old;

}

/* Get some notion of the current stack.  Need not be exactly the top
   of the stack, just something somewhere in the current frame.
   I don't trust register variables, so let's do this the safe way.  */
#define CURRENT_STACK_FRAME \
__extension__ ({ char *__sp; __asm__ ("mov %0,sp" : "=r" (__sp)); __sp; })

#else
#error PT_MACHINE already defined
#endif /* pt-machine.h */
