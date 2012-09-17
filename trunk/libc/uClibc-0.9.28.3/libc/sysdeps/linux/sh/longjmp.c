/* Copyright (C) 1991, 92, 94, 95, 97, 98, 2000 Free Software Foundation, Inc.
   Copyright (C) 2001 Hewlett-Packard Australia

 This program is free software; you can redistribute it and/or modify it under
 the terms of the GNU Library General Public License as published by the Free
 Software Foundation; either version 2 of the License, or (at your option) any
 later version.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE. See the GNU Library General Public License for more
 details.

 You should have received a copy of the GNU Library General Public License
 along with this program; if not, write to the Free Software Foundation, Inc.,
 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

 Derived in part from the Linux-8086 C library, the GNU C Library, and several
 other sundry sources.  Files within this library are copyright by their
 respective copyright holders.
*/

#include <stddef.h>
#include <setjmp.h>
#include <signal.h>

extern int __longjmp(char *env, int val);

/* Set the signal mask to the one specified in ENV, and jump
   to the position specified in ENV, causing the setjmp
   call there to return VAL, or 1 if VAL is 0.  */
void __libc_siglongjmp (sigjmp_buf env, int val)
{
  if (env[0].__mask_was_saved)
    /* Restore the saved signal mask.  */
    (void) sigprocmask (SIG_SETMASK, &env[0].__saved_mask,
			  (sigset_t *) NULL);

  /* Call the machine-dependent function to restore machine state.  */
  __longjmp ((char *) env[0].__jmpbuf, val ?: 1);
}

__asm__(".weak longjmp; longjmp = __libc_siglongjmp");
__asm__(".weak _longjmp; _longjmp = __libc_siglongjmp");
__asm__(".weak siglongjmp; siglongjmp = __libc_siglongjmp");
