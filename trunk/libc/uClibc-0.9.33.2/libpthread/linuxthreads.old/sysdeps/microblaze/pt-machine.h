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
 * Written by Miles Bader <miles@gnu.org>
 */

#ifndef _PT_MACHINE_H
#define _PT_MACHINE_H   1

#include <features.h>

#ifndef PT_EI
# define PT_EI extern inline
#endif

extern long int testandset (int *spinlock);
extern int __compare_and_swap (long *ptr, long old, long new);

/* Get some notion of the current stack.  Need not be exactly the top
   of the stack, just something somewhere in the current frame.  */
#define CURRENT_STACK_FRAME  __stack_pointer
register char *__stack_pointer __asm__ ("r1");

#define HAS_COMPARE_AND_SWAP
#define HAS_COMPARE_AND_SWAP_WITH_RELEASE_SEMANTICS
#define IMPLEMENT_TAS_WITH_CAS

/* Atomically:  If *PTR == OLD, set *PTR to NEW and return true,
   otherwise do nothing and return false.  */
PT_EI int __compare_and_swap (long *ptr, long old, long new)
{
  unsigned long psw;

  /* disable interrupts  */
  /* This is ugly ugly ugly! */
  __asm__ __volatile__ ("mfs	%0, rmsr;"
			"andi	r3, %0, ~2;"
			"mts	rmsr, r3;"
			: "=&r" (psw)
			:
			: "r3");

  if (likely (*ptr == old))
    {
      *ptr = new;
      __asm__ __volatile__ ("mts rmsr, %0;" :: "r" (psw)); /* re-enable */
      return 1;
    }
  else
    {
      __asm__ __volatile__ ("mts rmsr, %0;" :: "r" (psw)); /* re-enable */
      return 0;
    }
}

/* like above's __compare_and_swap() but it first syncs the memory
   (This is also the difference between both functions in e.g.
    ../powerpc/pt-machine.h)
   Doing this additional sync fixes a hang of __pthread_alt_unlock()
   (Falk Brettschneider <fbrettschneider@baumeroptronic.de>) */
PT_EI int
__compare_and_swap_with_release_semantics (long *p,
					   long oldval, long newval)
{
  __asm__ __volatile__ ("" : : : "memory"); /*MEMORY_BARRIER ();*/
  return __compare_and_swap (p, oldval, newval);
}


#ifndef IMPLEMENT_TAS_WITH_CAS
/* Spinlock implementation; required.  */
PT_EI long int testandset (int *spinlock)
{
	unsigned psw;

	/* disable interrupts */
	__asm__ __volatile__ ("mfs	%0, rmsr;"
			      "andi	r3, %0, ~2;"
			       "mts	rmsr, r3;"
			: "=&r" (psw)
			:
			: "r3");

	if (*spinlock)
	{
		/* Enable ints */
		__asm__ __volatile__ ("mts	rmsr, %0;" :: "r" (psw));
		return 1;
	} else {
		*spinlock=1;
		/* Enable ints */
		__asm__ __volatile__ ("mts	rmsr, %0;" :: "r" (psw));
		return 0;
	}
}

#endif
#endif /* pt-machine.h */
