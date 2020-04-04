/*
 * Copyright (C) 2016-2017 Andes Technology, Inc.
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/* Machine-dependent pthreads configuration and inline functions.
   Copyright (C) 1997, 1998, 2000, 2002, 2003 Free Software Foundation, Inc.
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

extern long int testandset (int *spinlock);
extern int __compare_and_swap (long int *p, long int oldval, long int newval);

/* Spinlock implementation; required.  */
PT_EI long int
testandset (int *spinlock)
{
	unsigned int		val;
	unsigned int		temp;
	unsigned int		offset = 0;

	__asm__ __volatile__ (
		"1:\n\t"
		"llw	%[val],	[%[spinlock] + %[offset] << 0]\n\t"
		"move	%[temp], #0x1\n\t"
		"beq	%[val], %[temp], 2f\n\t"
		"scw	%[temp], [%[spinlock] + %[offset] << 0]\n\t"
		"beqz   %[temp], 1b\n\t"
		"2:\n\t"
		: [val] "=&r" (val), [temp] "=&r" (temp)
		: [spinlock] "r" (spinlock), [offset] "r" (offset)
		: "memory" ) ;
		
	return val ;
}


/* Get some notion of the current stack.  Need not be exactly the top
   of the stack, just something somewhere in the current frame.  */
#define CURRENT_STACK_FRAME  stack_pointer
register char * stack_pointer __asm__ ("$sp");

#endif /* pt-machine.h */
