/*
 * Copyright (C) 2003 by Erik Andersen
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.LIB.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <features.h>
/* Integer registers.  */
#define r0      0
#define r1      1
#define r2      2
#define r3      3
#define r4      4
#define r5      5
#define r6      6
#define r7      7
#define r8      8
#define r9      9
#define r10     10
#define r13     13
#define r31     31

.text
	.globl	_start
	.type	_start,%function
	.type	_init,%function
	.type	_fini,%function
#ifndef __UCLIBC_CTOR_DTOR__
	.weak _init
	.weak _fini
#endif
	.type	main,%function
	.type	__uClibc_main,%function

_start:
	mr	r9,r1 	/* Save the stack pointer and pass it to __uClibc_main */
	clrrwi	r1,r1,4	/* Align stack ptr to 16 bytes */
#ifdef __PIC__
# ifdef HAVE_ASM_PPC_REL16
	bcl	20,31,1f
1:	mflr	r31
	addis	r31,r31,_GLOBAL_OFFSET_TABLE_-1b@ha
	addi	r31,r31,_GLOBAL_OFFSET_TABLE_-1b@l
# else
	bl	_GLOBAL_OFFSET_TABLE_-4@local
	mflr	r31
# endif
#endif
	/* Set up the small data pointer in r13.  */
#ifdef __PIC__
	lwz	r13,_SDA_BASE_@got(r31)
#else
	lis	r13,_SDA_BASE_@ha
	addi	r13,r13,_SDA_BASE_@l
#endif
	/* Set up an initial stack frame, and clear the LR.  */
	li	r0,0
	stwu	r1,-16(r1)
	mtlr	r0
	stw	r0,0(r1)
	/* find argc from the stack pointer */
	lwz	r4,0(r9)
	/* find argv one word offset from the stack pointer */
	addi	r5,r9,4
	mr	r8,r3 /* Pass _dl_fini from ldso or NULL if statically linked */
		      /* Note: PPC depends on the kernel to zero r3 before */
		      /* handing over to user space, otherwise static apps */
		      /* will SEGV during exit() */

	/* Ok, now run uClibc's main() -- shouldn't return */
#ifdef __PIC__
	lwz	r6,_init@got(r31)
	lwz	r7,_fini@got(r31)
	lwz	r3,main@got(r31)
	b	__uClibc_main@plt
#else
	lis     r6,_init@ha	# load top 16 bits
	addi    r6,r6,_init@l	# load bottom 16 bits
	lis     r7,_fini@ha	# load top 16 bits
	addi    r7,r7,_fini@l	# load bottom 16 bits
	lis     r3,main@ha	# load top 16 bits
	addi    r3,r3,main@l	# load bottom 16 bits
	b	__uClibc_main
#endif

.size _start,.-_start

/* Define a symbol for the first piece of initialized data.  */
	.data
	.globl __data_start
__data_start:
	.long 0
	.weak data_start
	data_start = __data_start

