/* Copyright (C) 2002, David McCullough <davidm@snapgear.com> */
/* Copyright (C) 1997,1998,2005,2006 Free Software Foundation, Inc.
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

/* Define the machine-dependent type `jmp_buf'.  m68k version.  */
#ifndef _BITS_SETJMP_H
#define _BITS_SETJMP_H	1

#if !defined _SETJMP_H && !defined _PTHREAD_H
# error "Never include <bits/setjmp.h> directly; use <setjmp.h> instead."
#endif

#ifndef _ASM

typedef struct
  {
    /* There are eight 4-byte data registers, but D0 is not saved.  */
    long int __dregs[7];

    /* There are six 4-byte address registers, plus the FP and SP.  */
    int *__aregs[6];
    int *__fp;
    int *__sp;

#if defined __HAVE_68881__ || defined __HAVE_FPU__
    /* There are eight floating point registers which
       are saved in IEEE 96-bit extended format.  */
    char __fpregs[8 * (96 / 8)];
#endif

  } __jmp_buf[1];

#endif

#define JB_REGS   0
#define JB_DREGS  0
#define JB_AREGS  24
#define JB_PC     48
#define JB_FPREGS 52

#if defined __HAVE_68881__ || defined __HAVE_FPU__
# define JB_SIZE 76
#else
# define JB_SIZE 52
#endif


/* Test if longjmp to JMPBUF would unwind the frame
   containing a local variable at ADDRESS.  */
#define _JMPBUF_UNWINDS(jmpbuf, address) \
  ((void *) (address) < (void *) (jmpbuf)->__aregs[5])

#endif	/* bits/setjmp.h */
