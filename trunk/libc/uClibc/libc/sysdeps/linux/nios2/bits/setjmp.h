/* Define the machine-dependent type `jmp_buf'.  Nios2 version.
   Copyright (C) 1992,93,95,97,2000 Free Software Foundation, Inc.
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

#ifndef _SETJMP_H
# error "Never include <bits/setjmp.h> directly; use <setjmp.h> instead."
#endif

#ifndef	_ASM
typedef struct
{
    /* Callee-saved registers r16 through r23.  */
    unsigned long __regs[8];

    /* Program counter.  */
    unsigned long __pc;

    /* Stack pointer.  */
    unsigned long __sp;

    /* The frame pointer.  */
    unsigned long __fp;

    /* The global pointer.  */
    unsigned long __gp;

	/* floating point regs, if any */
#if defined __HAVE_FPU__
    unsigned long __fpregs[64];
#endif	
} __jmp_buf[1];

#endif

#define JB_REGS		0
#define JB_PC		32
#define JB_SP		36
#define JB_FP		40
#define JB_GP		44
#define JB_FPREGS 	48

#if defined __HAVE_FPU__
# define JB_SIZE 304
#else
# define JB_SIZE 48
#endif


/* Test if longjmp to JMPBUF would unwind the frame
   containing a local variable at ADDRESS.  */
#define _JMPBUF_UNWINDS(jmpbuf, address) \
  ((void *) (address) < (void*)(jmpbuf)->__sp)
