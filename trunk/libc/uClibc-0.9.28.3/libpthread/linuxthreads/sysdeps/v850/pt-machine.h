/*
 * sysdeps/v850/pt-machine.h -- v850-specific pthread definitions
 *
 *  Copyright (C) 2002  NEC Electronics Corporation
 *  Copyright (C) 2002  Miles Bader <miles@gnu.org>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License.  See the file COPYING.LIB in the main
 * directory of this archive for more details.
 *
 * Written by Miles Bader <miles@gnu.org>
 */

#ifndef PT_EI
# define PT_EI extern inline
#endif

/* Get some notion of the current stack.  Need not be exactly the top
   of the stack, just something somewhere in the current frame.  */
#define CURRENT_STACK_FRAME  __stack_pointer
register char *__stack_pointer __asm__ ("sp");

#define HAS_COMPARE_AND_SWAP

/* Atomically:  If *PTR == OLD, set *PTR to NEW and return true,
   otherwise do nothing and return false.  */
PT_EI int
__compare_and_swap (long *ptr, long old, long new)
{
  unsigned long psw;

  /* disable interrupts  */
  __asm__ __volatile__ ("stsr psw, %0; di" : "=&r" (psw));

  if (likely (*ptr == old))
    {
      *ptr = new;
      __asm__ __volatile__ ("ldsr %0, psw" :: "r" (psw)); /* re-enable */
      return 1;
    }
  else
    {
      __asm__ __volatile__ ("ldsr %0, psw" :: "r" (psw)); /* re-enable */
      return 0;
    }
}
