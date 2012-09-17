/* Copyright (C) 1997, 1998, 2000, 2003, 2004 Free Software Foundation, Inc.
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

/* Define the machine-dependent type `jmp_buf'.  PowerPC version.  */
#ifndef _BITS_SETJMP_H
#define _BITS_SETJMP_H	1

#if !defined _SETJMP_H && !defined _PTHREAD_H
# error "Never include <bits/setjmp.h> directly; use <setjmp.h> instead."
#endif

/* The previous bits/setjmp.h had __jmp_buf defined as a structure.
   We use an array of 'long int' instead, to make writing the
   assembler easier. Naturally, user code should not depend on
   either representation. */

#include <bits/wordsize.h>

#if defined __USE_MISC || defined _ASM
# define JB_GPR1   0  /* Also known as the stack pointer */
# define JB_GPR2   1
# define JB_LR     2  /* The address we will return to */
# if __WORDSIZE == 64
#  define JB_GPRS   3  /* GPRs 14 through 31 are saved, 18*2 words total.  */
#  define JB_CR     21 /* Condition code registers with the VRSAVE at */
                       /* offset 172 (low half of the double word.  */
#  define JB_FPRS   22 /* FPRs 14 through 31 are saved, 18*2 words total.  */
#  define JB_SIZE   (64 * 8) /* As per PPC64-VMX ABI.  */
#  define JB_VRSAVE 21 /* VRSAVE shares a double word with the CR at offset */
                       /* 168 (high half of the double word).  */
#  define JB_VRS    40 /* VRs 20 through 31 are saved, 12*4 words total.  */
# else
#  define JB_GPRS   3  /* GPRs 14 through 31 are saved, 18 in total.  */
#  define JB_CR     21 /* Condition code registers.  */
#  define JB_FPRS   22 /* FPRs 14 through 31 are saved, 18*2 words total.  */
#  define JB_SIZE   ((64 + (12 * 4)) * 4)
#  define JB_VRSAVE 62
#  define JB_VRS    64
# endif
#endif


/* The current powerpc 32-bit Altivec ABI specifies for SVR4 ABI and EABI
   the vrsave must be at byte 248 & v20 at byte 256.  So we must pad this
   correctly on 32 bit.  It also insists that vecregs are only gauranteed
   4 byte alignment so we need to use vperm in the setjmp/longjmp routines.
   We have to version the code because members like  int __mask_was_saved
   in the jmp_buf will move as jmp_buf is now larger than 248 bytes.  We
   cannot keep the altivec jmp_buf backward compatible with the jmp_buf.  */
#ifndef	_ASM
# if __WORDSIZE == 64
typedef long int __jmp_buf[64] __attribute__ ((__aligned__ (16)));
# else
/* The alignment is not essential, i.e.the buffer can be copied to a 4 byte
   aligned buffer as per the ABI it is just added for performance reasons.  */
typedef long int __jmp_buf[64 + (12 * 4)] __attribute__ ((__aligned__ (16)));
# endif
#endif

/* Test if longjmp to JMPBUF would unwind the frame
   containing a local variable at ADDRESS.  */
#define _JMPBUF_UNWINDS(jmpbuf, address) \
  ((void *) (address) < (void *) (jmpbuf)[JB_GPR1])

#endif	/* bits/setjmp.h */
