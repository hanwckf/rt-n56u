/*
 * Copyright (C) 2016-2017 Andes Technology, Inc.
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/* Copyright (C) 2011-2013 Free Software Foundation, Inc.
   Contributed by Chris Metcalf <cmetcalf@tilera.com>, 2011.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ucontext.h>

/* makecontext sets up a stack and the registers for the
   user context.  The stack looks like this:

               +-----------------------+
	       | padding as required   |
               +-----------------------+
    sp ->      | parameter 7-n         |
               +-----------------------+

   The registers are set up like this:
     $r0 .. $r5: parameter 1 to 6
     $r6   : uc_link
     $sp   : stack pointer.
*/
void
__makecontext (ucontext_t *ucp, void (*func) (void), int argc, ...)
{
  extern void __startcontext (void);
  unsigned long int *sp;
  unsigned long *regptr;
  va_list ap;
  int i;

  sp = (unsigned long int *)
    ((uintptr_t) ucp->uc_stack.ss_sp + ucp->uc_stack.ss_size);

  /* Allocate stack arguments.  */
  sp -= argc < 6 ? 0 : argc - 6;

  /* Keep the stack aligned.  */
  sp = (unsigned long int *) (((uintptr_t) sp) & -8L);

  ucp->uc_mcontext.nds32_r6 = (uintptr_t) ucp->uc_link;
  ucp->uc_mcontext.nds32_sp = (uintptr_t) sp;
  ucp->uc_mcontext.nds32_ipc = (uintptr_t) func;
  ucp->uc_mcontext.nds32_lp = (uintptr_t) &__startcontext;

  va_start (ap, argc);
  regptr = &ucp->uc_mcontext.nds32_r0;
  for (i = 0; i < argc; ++i)
    if (i < 6)
      *regptr++ = va_arg (ap, unsigned long int);
    else
      sp[i - 6] = va_arg (ap, unsigned long int);

  va_end (ap);

}
weak_alias (__makecontext, makecontext)
