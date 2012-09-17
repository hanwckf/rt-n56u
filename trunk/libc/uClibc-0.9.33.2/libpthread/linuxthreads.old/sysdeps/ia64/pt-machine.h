/* Machine-dependent pthreads configuration and inline functions.
   IA-64 version.
   Copyright (C) 1999, 2000, 2002, 2003, 2004 Free Software Foundation, Inc.
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
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#ifndef _PT_MACHINE_H
#define _PT_MACHINE_H   1

#include <features.h>
#include <ia64intrin.h>

#include <sys/types.h>
extern int __clone2 (int (*__fn) (void *__arg), void *__child_stack_base,
                     size_t __child_stack_size, int __flags, void *__arg, ...);

#ifndef PT_EI
# define PT_EI __extern_always_inline
#endif

/* Make sure gcc doesn't try to be clever and move things around on
   us. We need to use _exactly_ the address the user gave us, not some
   alias that contains the same information.  */
#define __atomic_fool_gcc(x) (*(volatile struct { int a[100]; } *)x)

#ifndef ELF_MACHINE_NAME

#define NEED_SEPARATE_REGISTER_STACK

/* We want the OS to assign stack addresses.  */
#define FLOATING_STACKS 1

/* Maximum size of the stack if the rlimit is unlimited.  */
#define ARCH_STACK_MAX_SIZE     32*1024*1024

/* Get some notion of the current stack.  Need not be exactly the top
   of the stack, just something somewhere in the current frame.
   r12 (sp) is the stack pointer. */
#define CURRENT_STACK_FRAME  stack_pointer
register char *stack_pointer __asm__ ("sp");


/* Register r13 (tp) is reserved by the ABI as "thread pointer". */
struct _pthread_descr_struct;
register struct _pthread_descr_struct *__thread_self __asm__("r13");

/* Return the thread descriptor for the current thread.  */
#define THREAD_SELF  __thread_self

/* Initialize the thread-unique value.  */
#define INIT_THREAD_SELF(descr, nr)  (__thread_self = (descr))


/* Access to data in the thread descriptor is easy.  */
#define THREAD_GETMEM(descr, member) \
  ((void) sizeof (descr), THREAD_SELF->member)
#define THREAD_GETMEM_NC(descr, member) \
  ((void) sizeof (descr), THREAD_SELF->member)
#define THREAD_SETMEM(descr, member, value) \
  ((void) sizeof (descr), THREAD_SELF->member = (value))
#define THREAD_SETMEM_NC(descr, member, value) \
  ((void) sizeof (descr), THREAD_SELF->member = (value))


/* Memory barrier */
#define MEMORY_BARRIER() __sync_synchronize ()


#define HAS_COMPARE_AND_SWAP_WITH_RELEASE_SEMANTICS

PT_EI int
__compare_and_swap (long int *p, long int oldval, long int newval)
{
  long int readval;

  __asm__ __volatile__
       ("mov ar.ccv=%4;;\n\t"
	"cmpxchg8.acq %0=%1,%2,ar.ccv"
	: "=r" (readval), "=m" (__atomic_fool_gcc (p))
	: "r"(newval), "m" (__atomic_fool_gcc (p)), "r" (oldval)
	: "memory");
  return readval == oldval;
}

PT_EI int
__compare_and_swap_with_release_semantics (long int *p,
					   long int oldval,
					   long int newval)
{
  long int readval;

  __asm__ __volatile__
       ("mov ar.ccv=%4;;\n\t"
	"cmpxchg8.rel %0=%1,%2,ar.ccv"
	: "=r" (readval), "=m" (__atomic_fool_gcc (p))
	: "r"(newval), "m" (__atomic_fool_gcc (p)), "r" (oldval)
	: "memory");
  return readval == oldval;
}

#endif /* ELF_MACHINE_NAME */

/* Spinlock implementation; required.  */
PT_EI long int
testandset (int *spinlock)
{
  long int ret;

  __asm__ __volatile__(
       "xchg4 %0=%1,%2"
       : "=r"(ret), "=m"(__atomic_fool_gcc (spinlock))
       : "r"(1), "m"(__atomic_fool_gcc (spinlock))
       : "memory");

  return ret;
}

/* Indicate that we are looping.  */
#define BUSY_WAIT_NOP	__asm__ ("hint @pause")

#endif /* pt-machine.h */
