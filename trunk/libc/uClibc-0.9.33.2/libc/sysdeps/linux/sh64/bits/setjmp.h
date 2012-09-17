/* Copyright (C) 1999, 2000 Free Software Foundation, Inc.
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

/* Define the machine-dependent type `jmp_buf'.  SH-5 version. */
#ifndef _BITS_SETJMP_H
#define _BITS_SETJMP_H	1

#if !defined _SETJMP_H && !defined _PTHREAD_H
# error "Never include <bits/setjmp.h> directly; use <setjmp.h> instead."
#endif

#define	__SETJMP_NUM_INT	31		/* number of integer registers to save */
#define	__SETJMP_NUM_DBL	0 /* 16 */	/* number of double registers to save */
#define	__SETJMP_NUM_TRG	3		/* number of traget registers to save */

#define	__SETJMP_INT(x)	(x)
#define	__SETJMP_DBL(x)	(__SETJMP_NUM_INT+(x))
#define	__SETJMP_TRG(x)	(__SETJMP_NUM_INT+__SETJMP_NUM_DBL+(x))
#define	__SETJMP_LR	(__SETJMP_NUM_INT+__SETJMP_NUM_DBL+__SETJMP_NUM_TRG)


#ifndef _ASM
typedef struct
  {
	    /* Callee-saved registers.  */
    unsigned long long __ints[__SETJMP_NUM_INT];	/* integer registers */
#if __SETJMP_NUM_DBL > 0
    unsigned long long __dbls[__SETJMP_NUM_DBL];	/* double registers */
#endif
    unsigned long long __trgs[__SETJMP_NUM_TRG];	/* traget registers */
    unsigned long long __lr;				/* linkage register */
  } __jmp_buf[1];
#endif

#endif	/* bits/setjmp.h */
