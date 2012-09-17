/* Machine-dependent pthreads configuration and inline functions.
   sparc version.
   Copyright (C) 1996, 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Richard Henderson <rth@tamu.edu>.

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

#ifndef PT_EI
# define PT_EI extern inline
#endif

/* Spinlock implementation; required.  */
PT_EI int
testandset (int *spinlock)
{
  int ret;

  __asm__ __volatile__("ldstub %1,%0"
	: "=r"(ret), "=m"(*spinlock)
	: "m"(*spinlock));

  return ret;
}


/* Spinlock release; default is just set to zero.  */
#define RELEASE(spinlock) \
  __asm__ __volatile__("stbar; stb %1,%0" : "=m"(*(spinlock)) : "r"(0));


/* Get some notion of the current stack.  Need not be exactly the top
   of the stack, just something somewhere in the current frame.  */
#define CURRENT_STACK_FRAME  stack_pointer
register char * stack_pointer __asm__("%sp");


/* Registers %g6 and %g7 are reserved by the ABI for "system use".  It
   happens that Solaris uses %g6 for the thread pointer -- we do the same.  */
struct _pthread_descr_struct;
register struct _pthread_descr_struct *__thread_self __asm__("%g6");

/* Return the thread descriptor for the current thread.  */
#define THREAD_SELF  __thread_self

/* Initialize the thread-unique value.  */
#define INIT_THREAD_SELF(descr, nr)  (__thread_self = (descr))

/* Access to data in the thread descriptor is easy.  */
#define THREAD_GETMEM(descr, member) __thread_self->member
#define THREAD_GETMEM_NC(descr, member) __thread_self->member
#define THREAD_SETMEM(descr, member, value) __thread_self->member = (value)
#define THREAD_SETMEM_NC(descr, member, value) __thread_self->member = (value)
