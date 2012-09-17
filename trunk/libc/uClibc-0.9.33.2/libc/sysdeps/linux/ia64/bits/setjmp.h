/* Define the machine-dependent type `jmp_buf'.  Linux/IA-64 version.
   Copyright (C) 1999, 2000, 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by David Mosberger-Tang <davidm@hpl.hp.com>.

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
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef _BITS_SETJMP_H
#define _BITS_SETJMP_H  1

#if !defined _SETJMP_H && !defined _PTHREAD_H
# error "Never include <bits/setjmp.h> directly; use <setjmp.h> instead."
#endif

/* User code must not depend on the internal representation of jmp_buf. */

#define _JBLEN	70

/* the __jmp_buf element type should be __float80 per ABI... */
typedef long __jmp_buf[_JBLEN] __attribute__ ((aligned (16))); /* guarantees 128-bit alignment! */

/* Test if longjmp to JMPBUF would unwind the frame containing a local
   variable at ADDRESS.  */
#define _JMPBUF_UNWINDS(_jmpbuf, _address)		\
     ((void *)(_address) < (void *)(((long *)_jmpbuf)[0]))

#endif  /* bits/setjmp.h */
