/* Machine-dependent pthreads configuration and inline functions.
   C6x version.
   Copyright (C) 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Aurelien Jacquiot <aurelien.jacquiot@jaluna.com>.

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

#ifndef PT_EI
# define PT_EI extern inline
#endif

extern int __compare_and_swap (long int *p, long int oldval, long int newval);

/* Spinlock implementation; required.  */
static inline long int
testandset (int *spinlock)
{
	register unsigned int ret = 1;
	int dummy;
	__asm__ __volatile__ ("mvc .s2 CSR, %0\n\tand .s2 -2, %0, %0\n\tmvc .s2 %0, CSR\n"
			      : "=b" (dummy));
	
	if (*spinlock == 0) {
		*spinlock = 1;
		ret = 0;
	}
	__asm__ __volatile__ ("mvc .s2 CSR, %0\n\tor .s2 1, %0, %0\n\tmvc .s2 %0, CSR\n"
			      : "=b" (dummy));
	return ret;
}

#define WRITE_MEMORY_BARRIER()
#define READ_MEMORY_BARRIER()

/* Get some notion of the current stack.  Need not be exactly the top
   of the stack, just something somewhere in the current frame.  */
#define CURRENT_STACK_FRAME  get_stack_pointer()
static inline char * get_stack_pointer(void)
{
	char *sp;
  	__asm__ __volatile__ ("mv .d2 B15, %0" : "=b" (sp));
	return sp;
}

#define THREAD_STACK_OFFSET 8

#endif /* pt-machine.h */
