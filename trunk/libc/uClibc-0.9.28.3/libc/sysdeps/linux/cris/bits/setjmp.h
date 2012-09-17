/* Copyright (C) 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* Define the machine-dependent type `jmp_buf'.  CRIS version.  */

#ifndef _SETJMP_H
# error "Never include <bits/setjmp.h> directly; use <setjmp.h> instead."
#endif

/*
   Note that we save and restore CCR to be able to
   correctly handle DI/EI.  Note also that the "move x,ccr" does NOT affect
   the DMA enable bits (E and D).

   jmp_buf[0] - PC
   jmp_buf[1] - SP (R14)
   jmp_buf[2] - R13
   jmp_buf[3] - R12
   jmp_buf[4] - R11
   jmp_buf[5] - R10
   jmp_buf[6] - R9
   jmp_buf[7] - R8
   jmp_buf[8] - R7
   jmp_buf[9] - R6
   jmp_buf[10] - R5
   jmp_buf[11] - R4
   jmp_buf[12] - R3
   jmp_buf[13] - R2
   jmp_buf[14] - R1
   jmp_buf[15] - R0
   jmp_buf[16] - SRP
   jmp_buf[17] - CCR
   */

#define _JBLEN 18
#if     defined (__USE_MISC) || defined (_ASM)
#define JB_SP 1
#endif

#ifndef	_ASM
typedef int __jmp_buf[_JBLEN];
#endif

/* Test if longjmp to JMPBUF would unwind the frame
   containing a local variable at ADDRESS.  */
#define _JMPBUF_UNWINDS(jmpbuf, address) \
  ((void *) (address) < (void *) (jmpbuf)[JB_SP])


