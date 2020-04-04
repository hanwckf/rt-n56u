/* Copyright (C) 1999, 2000, 2003, 2004 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

/* Define the machine-dependent type `jmp_buf'.  FRV version. */

#ifndef _BITS_SETJMP_H
#define _BITS_SETJMP_H	1

#if !defined _SETJMP_H && !defined _PTHREAD_H
# error "Never include <bits/setjmp.h> directly; use <setjmp.h> instead."
#endif

#define	__SETJMP_NUM_INT	32		/* number of integer registers to save */
#define	__SETJMP_NUM_DBL	32		/* number of double registers to save */

#define	__SETJMP_INT(x)	(x)
#define	__SETJMP_DBL(x)	(__SETJMP_NUM_INT+(x))
#define	__SETJMP_LR	(__SETJMP_NUM_INT+__SETJMP_NUM_DBL)
#define __SETJMP_SP	(__SETJMP_LR+1)
#define __SETJMP_FP	(__SETJMP_SP+1)


typedef struct
/* Demand 64-bit alignment such that we can use std/ldd in
   setjmp/longjmp.  */
__attribute__((__aligned__(8)))
  {
    /* Callee-saved registers.  */
    unsigned long __ints[__SETJMP_NUM_INT];	/* integer registers */
    unsigned long __dbls[__SETJMP_NUM_DBL];	/* double registers */
    unsigned long __lr;				/* linkage register */
    unsigned long __sp;				/* stack pointer */
    unsigned long __fp;				/* frame pointer */
  } __jmp_buf[1];

#endif	/* bits/setjmp.h */
