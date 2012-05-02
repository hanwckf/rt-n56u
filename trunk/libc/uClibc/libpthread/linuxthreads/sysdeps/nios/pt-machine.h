/* Machine-dependent pthreads configuration and inline functions.
   ARM version.
   Copyright (C) 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Philip Blundell <philb@gnu.org>.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#ifndef _PT_MACHINE_H
#define _PT_MACHINE_H   1

#ifndef PT_EI
# define PT_EI extern inline
#endif

extern long int testandset (int *spinlock);
/* Spinlock implementation; required.  */
/* it is weird and dangerous to disable interrupt in userspace, but for nios
   what else we can do before we have a swap like instruction?  This is better
   than nothing
 */
PT_EI long int
testandset (int *spinlock)
{
  unsigned int ret;

  __asm__ __volatile__("pfx 8\n\t"
  			"wrctl %1	; disable interrupt\n\t"
			"nop\n\t"
			"nop\n\t"
  			"ld %0, [%2]\n\t"
  			"st [%2], %1\n\t"
  			"pfx 9\n\t"
  			"wrctl %1	; enable interrupt\n\t"
			"nop\n\t"
			"nop\n\t"
 		       : "=&r"(ret)
		       : "r"(1), "r"(spinlock)
		       : "memory");

  return ret;
}


/* Get some notion of the current stack.  Need not be exactly the top
   of the stack, just something somewhere in the current frame.  */
#define CURRENT_STACK_FRAME  stack_pointer
register char * stack_pointer __asm__ ("%sp");

/* nios needs more because of reg windows */
#define THREAD_MANAGER_STACK_SIZE (32*1024)
#define STACK_SIZE	(32*1024)

#endif /* pt-machine.h */
