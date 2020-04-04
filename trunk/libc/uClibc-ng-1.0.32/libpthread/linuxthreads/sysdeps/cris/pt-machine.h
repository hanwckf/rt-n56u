/* Machine-dependent pthreads configuration and inline functions.
   CRIS version.
   Copyright (C) 2001, 2002, 2003 Free Software Foundation, Inc.
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
   see <http://www.gnu.org/licenses/>.  */
#ifndef _PT_MACHINE_H
#define _PT_MACHINE_H   1

#include <features.h>

#ifndef PT_EI
# define PT_EI __extern_always_inline
#endif

PT_EI long int
testandset (int *spinlock)
{
  register unsigned long int ret;

  /* Note the use of a dummy output of *spinlock to expose the write.  The
     memory barrier is to stop *other* writes being moved past this code.  */

#ifdef __arch_v32
  __asm__ __volatile__("clearf p\n"
		       "0:\n\t"
		       "movu.b [%2],%0\n\t"
		       "ax\n\t"
		       "move.b %3,[%2]\n\t"
		       "bcs 0b\n\t"
		       "clearf p"
		       : "=&r" (ret), "=m" (*spinlock)
		       : "r" (spinlock), "r" ((int) 1), "m" (*spinlock)
		       : "memory");
#else  /* not __arch_v32 */
  __asm__ __volatile__("clearf\n"
		       "0:\n\t"
		       "movu.b [%2],%0\n\t"
		       "ax\n\t"
		       "move.b %3,[%2]\n\t"
		       "bwf 0b\n\t"
		       "clearf"
		       : "=&r" (ret), "=m" (*spinlock)
		       : "r" (spinlock), "r" ((int) 1), "m" (*spinlock)
		       : "memory");
#endif /* __arch_v32 */
  return ret;
}


/* Get some notion of the current stack.  Need not be exactly the top
   of the stack, just something somewhere in the current frame.
   I don't trust register variables, so let's do this the safe way.  */
#define CURRENT_STACK_FRAME \
 ({ char *sp; __asm__ ("move.d $sp,%0" : "=rm" (sp)); sp; })

#endif /* pt-machine.h */
