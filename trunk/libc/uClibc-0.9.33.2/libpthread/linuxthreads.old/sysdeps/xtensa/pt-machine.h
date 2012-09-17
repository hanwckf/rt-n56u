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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street - Fifth Floor,
   Boston, MA 02110-1301, USA.  */

#ifndef _PT_MACHINE_H
#define _PT_MACHINE_H   1

#include <sys/syscall.h>
#include <asm/unistd.h>

#ifndef PT_EI
# define PT_EI __extern_always_inline
#endif

/* Memory barrier.  */
#define MEMORY_BARRIER() __asm__ ("memw" : : : "memory")

/* Spinlock implementation; required.  */
PT_EI long int
testandset (int *spinlock)
{
  int unused = 0;
  return INTERNAL_SYSCALL (xtensa, , 4, SYS_XTENSA_ATOMIC_SET,
			   spinlock, 1, unused);
}

/* Get some notion of the current stack.  Need not be exactly the top
   of the stack, just something somewhere in the current frame.  */
#define CURRENT_STACK_FRAME __builtin_frame_address (0)

#endif /* _PT_MACHINE_H */
