/* Copyright (C) 1999, 2000, 2003, 2005 Free Software Foundation, Inc.
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
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

/* Define the machine-dependent type `jmp_buf'.  SH version. */
#ifndef _BITS_SETJMP_H
#define _BITS_SETJMP_H	1

#if !defined _SETJMP_H && !defined _PTHREAD_H
# error "Never include <bits/setjmp.h> directly; use <setjmp.h> instead."
#endif

#ifndef _ASM
typedef struct
  {
    /* Callee-saved registers r8 through r15.  */
    int __regs[8];

    /* Program counter.  */
    void * __pc;

    /* The global pointer.  */
    void * __gbr;

    /* Floating point status register.  */
    int __fpscr;

    /* Callee-saved floating point registers fr12 through fr15.  */
    int __fpregs[4];
  } __jmp_buf[1];
#endif

#if defined __USE_MISC || defined _ASM
# define JB_SIZE		(4 * 15)
#endif

/* Test if longjmp to JMPBUF would unwind the frame
   containing a local variable at ADDRESS.  */
#define _JMPBUF_UNWINDS(jmpbuf, address) \
  ((void *) (address) < (void *) (jmpbuf)[0].__regs[7])

#endif	/* bits/setjmp.h */
