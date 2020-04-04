/* Copyright (C) 1997, 1998 Free Software Foundation, Inc.
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

/* Define the machine-dependent type `jmp_buf'.  bfin version.  Lineo, Inc. 2001*/
#ifndef _BITS_SETJMP_H
#define _BITS_SETJMP_H	1

#if !defined _SETJMP_H && !defined _PTHREAD_H
# error "Never include <bits/setjmp.h> directly; use <setjmp.h> instead."
#endif

/* Jump buffer contains r7-r4, p5-p3, fp, sp and pc.  Other registers are not saved.  */
typedef struct
{
	unsigned long __pregs[6];
	unsigned long fp;
	unsigned long sp;
	unsigned long __rregs[8];
	unsigned long astat;
	unsigned long __lcregs[2];
	unsigned long a0w;
	unsigned long a0x;
	unsigned long a1w;
	unsigned long a1x;
	unsigned long __iregs[4];
	unsigned long __mregs[4];
	unsigned long __lregs[4];
	unsigned long __bregs[4];
	unsigned long pc;
}__jmp_buf[1];

#endif	/* bits/setjmp.h */
