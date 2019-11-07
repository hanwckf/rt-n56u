/* Machine-dependent pthreads configuration and inline functions.
   FR-V version.
   Copyright (C) 2004  Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Alexandre Oliva <aoliva@redhat.com>

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

#include <features.h>

#ifndef __ASSEMBLER__

#ifndef PT_EI
# define PT_EI __extern_always_inline
#endif

/* Spinlock implementation; required.  */
PT_EI long int
testandset (int *spinlock)
{
  int i = 1;
  __asm__ ("swap%I0 %M0, %1" : "+m"(*(volatile int *)spinlock), "+r"(i));
  return i;
}

/* We want the OS to assign stack addresses.  */
#define FLOATING_STACKS 1

/* This symbol is defined by the ABI as the stack size requested by
   the main program.  */
extern char __stacksize;
#define ARCH_STACK_MAX_SIZE ((unsigned long)&__stacksize)

/* Memory barrier; default is to do nothing */
#define MEMORY_BARRIER() __asm__ __volatile__("membar" : : : "memory")
/* Write barrier.  */
#define WRITE_MEMORY_BARRIER() __asm__ __volatile__("membar" : : : "memory")

/* Return the thread descriptor for the current thread.  */
register struct _pthread_descr_struct *THREAD_SELF __asm__ ("gr29");
#define THREAD_SELF THREAD_SELF

/* Initialize the thread-unique value.  */
#define INIT_THREAD_SELF(descr, nr) \
  (THREAD_SELF = descr)

/* Get some notion of the current stack.  Need not be exactly the top
   of the stack, just something somewhere in the current frame.  */
#define CURRENT_STACK_FRAME  stack_pointer
register char * stack_pointer __asm__ ("sp");

#endif

#endif /* pt-machine.h */
