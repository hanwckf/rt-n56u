/* longjmp for PA-RISC.
   Copyright (C) 1997, 1998 Free Software Foundation, Inc.
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

/* __longjmp(jmpbuf, val) */

	.text
	.align 4
	.globl __longjmp
	.export __longjmp, code
	.proc
	.callinfo
__longjmp:	
	/* set return value */
	copy	%r25, %r28
	
	ldw	0(%r26), %r3
	ldw	8(%r26), %r4
	ldw	12(%r26), %r5
	ldw	16(%r26), %r6
	ldw	20(%r26), %r7
	ldw	24(%r26), %r8
	ldw	28(%r26), %r9
	ldw	32(%r26), %r10
	ldw	36(%r26), %r11
	ldw	40(%r26), %r12
	ldw	44(%r26), %r13
	ldw	48(%r26), %r14
	ldw	52(%r26), %r15
	ldw	56(%r26), %r16
	ldw	60(%r26), %r17
	ldw	64(%r26), %r18
	ldw	68(%r26), %r19
	ldw	72(%r26), %r27
	ldw	76(%r26), %r30
	
	ldw	80(%r26), %rp

	ldo	88(%r26),%r20
	fldds,ma 8(%r20), %fr12
	fldds,ma 8(%r20), %fr13
	fldds,ma 8(%r20), %fr14
	fldds,ma 8(%r20), %fr15
	fldds,ma 8(%r20), %fr16
	fldds,ma 8(%r20), %fr17
	fldds,ma 8(%r20), %fr18
	fldds,ma 8(%r20), %fr19
	fldds,ma 8(%r20), %fr20
	fldds	 0(%r20), %fr21

	bv,n	%r0(%r2)
	.procend

libc_hidden_def(__longjmp)
