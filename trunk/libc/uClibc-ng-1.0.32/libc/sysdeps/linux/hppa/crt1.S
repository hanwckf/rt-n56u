/* ELF startup code for HPPA.
   Copyright (C) 2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   In addition to the permissions in the GNU Lesser General Public
   License, the Free Software Foundation gives you unlimited
   permission to link the compiled version of this file with other
   programs, and to distribute those programs without any restriction
   coming from the use of this file. (The GNU Lesser General Public
   License restrictions do apply in other respects; for example, they
   cover modification of the file, and distribution when not linked
   into another program.)

   Note that people who make modified versions of this file are not
   obligated to grant this special exception for their modified
   versions; it is their choice whether to do so. The GNU Lesser
   General Public License gives permission to release a modified
   version without this exception; this exception also makes it
   possible to release a modified version which carries forward this
   exception.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

	.import main, code
	.import $global$, data
	.import __uClibc_main, code
#if defined(__UCLIBC_CTOR_DTOR__)
	.import _fini, code
	.import _init, code
#else
	.weak _fini
	.weak _init
#endif

	/* Have the linker create plabel words
           so we get PLABEL32 relocs and not 21/14 */
	.section	.rodata
	.align 4
.Lpmain:
	.word P%main
.Lp__uClibc_main:
	.word P%__uClibc_main
.Lp_fini:
	.word P%_fini
.Lp_init:
	.word P%_init

	.text
	.align 4
	.globl _start
	.export _start, ENTRY
	.type _start,@function
_start:

	.proc
	.callinfo

	/* Expand the stack to store the 5th through 7th args */
	ldo	64(%sp), %sp
	/* TODO: Follow ABI? Place more things on the stack here... */

#ifdef __PIC__
	/* load main (1st argument) */
	addil	LR'.Lpmain, %r19
	ldw	RR'.Lpmain(%r1), %r26
	ldw	0(%r26),%r26
	/* argc and argv should be in 25 and 24 (2nd and 3rd argument) */
	/* void (*init) (void) (4th argument) */
	addil	LR'.Lp_init, %r19
	ldw	RR'.Lp_init(%r1), %r23
	ldw	0(%r23), %r23
	/* void (*fini) (void) (5th argument) */
	addil	LR'.Lp_fini, %r19
	ldw	RR'.Lp_fini(%r1), %r22
	ldw	0(%r22), %r22
#else
	/* load main (1st argument) */
	ldil	LR'.Lpmain, %r26
	ldw	RR'.Lpmain(%r26), %r26
	/* argc and argv should be in 25 and 24 (2nd and 3rd argument) */
	/* void (*init) (void) (4th argument) */
	ldil	LR'.Lp_init, %r23
	ldw	RR'.Lp_init(%r23), %r23
	/* void (*fini) (void) (5th argument) */
	ldil	LR'.Lp_fini, %r22
	ldw	RR'.Lp_fini(%r22), %r22
#endif
	/* Store 5th argument */
	stw	%r22, -52(%sp)
	/* void (*rtld_fini) (void) (6th argument) */
	stw	%r23, -56(%sp)
	/* void *stack_end (7th argument) */
	stw	%sp, -60(%sp)

	/* load global */
	ldil	L%$global$, %dp
	ldo	R%$global$(%dp), %dp

	bl	__uClibc_main,%r2
	nop
	/* die horribly if it returned (it shouldn't) */
	iitlbp %r0,(%sr0,%r0)
	nop

	.procend

/* Define a symbol for the first piece of initialized data.  */
	.data
	.globl __data_start
__data_start:
	.long 0
	.weak data_start
	data_start = __data_start
