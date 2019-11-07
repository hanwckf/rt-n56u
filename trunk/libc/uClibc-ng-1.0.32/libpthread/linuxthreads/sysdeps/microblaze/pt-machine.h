/*
 * sysdeps/microblaze/pt-machine.h -- microblaze-specific pthread definitions
 *
 *  Copyright (C) 2003 John Williams <jwilliams@itee.uq.edu.au>
 *  Copyright (C) 2002  NEC Electronics Corporation
 *  Copyright (C) 2002  Miles Bader <miles@gnu.org>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License.  See the file COPYING.LIB in the main
 * directory of this archive for more details.
 *
 */

#ifndef _PT_MACHINE_H
#define _PT_MACHINE_H   1

#include <features.h>

#ifndef PT_EI
# define PT_EI __extern_always_inline
#endif

/* Get some notion of the current stack.  Need not be exactly the top
   of the stack, just something somewhere in the current frame.  */
#define CURRENT_STACK_FRAME  __stack_pointer
register char *__stack_pointer __asm__ ("r1");

#define HAS_COMPARE_AND_SWAP
#define HAS_COMPARE_AND_SWAP_WITH_RELEASE_SEMANTICS
#define MEMORY_BARRIER() __asm__ __volatile__("": : :"memory")

/* Atomically:  If *PTR == OLD, set *PTR to NEW and return true,
   otherwise do nothing and return false.  */
PT_EI int __compare_and_swap (long *ptr, long old, long new)
{
  long prev, cmp, retval;
  __asm__ __volatile__ ("         addi    %2, r0, 0;"
                        "1:       lwx     %0, %3, r0;"
                        "         cmp     %1, %0, %4;"
                        "         bnei    %1, 2f;"
                        "         swx     %5, %3, r0;"
                        "         addic   %1, r0, 0;"
                        "         bnei    %1, 1b;"
                        "         addi    %2, r0, 1;"
                        "2:"
                        : "=&r" (prev), "=&r" (cmp), "=&r" (retval)
                        : "r" (ptr), "r" (old), "r" (new)
                        : "cc", "memory");

  return retval;
}

PT_EI int
__compare_and_swap_with_release_semantics (long *p,
					   long oldval, long newval)
{
  MEMORY_BARRIER();
  return __compare_and_swap (p, oldval, newval);
}

/* Spinlock implementation; required.  */ 
PT_EI long int testandset (int *spinlock)
{
  long int retval;

  __asm__ __volatile__ ("1:       lwx     %0, %1, r0;"
                        "         bnei    %0, 2f;"
                        "         addik   %0, r0, 1;"
                        "         swx     %0, %1, r0;"
                        "         addic   %0, r0, 0;"
                        "         bnei    %0, 1b;"
                        "2:"
                        : "=&r" (retval)
                        : "r" (spinlock)
                        : "cc", "memory");

  return retval;
}

#endif /* pt-machine.h */
