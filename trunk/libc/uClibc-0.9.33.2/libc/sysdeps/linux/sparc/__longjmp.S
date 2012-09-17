/* Copyright (C) 1991, 93, 96, 97, 98, 99, 2000 Free Software Foundation, Inc.
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

#include <features.h>
#include <sys/syscall.h>

#define _ASM 1
#define _SETJMP_H
#include <bits/setjmp.h>
#define ENV(base,reg) [%base + (reg * 4)]
#define ST_FLUSH_WINDOWS 3
#define RW_FP [%fp + 0x48]

.global __longjmp
.type   __longjmp,%function
.align 4
__longjmp:
	.register %g2, #scratch
	.register %g3, #scratch
	/* Store our arguments in global registers so we can still
	 * use them while unwinding frames and their register windows.  */

	ld ENV(o0,JB_FP), %g3	/* Cache target FP in register %g3.  */
	mov %o0, %g1		/* ENV in %g1 */
	orcc %o1, %g0, %g2	/* VAL in %g2 */
	be,a 0f			/* Branch if zero; else skip delay slot.  */
	mov 1, %g2		/* Delay slot only hit if zero: VAL = 1.  */
0:
	xor %fp, %g3, %o0
	add %fp, 512, %o1
	andncc %o0, 4095, %o0
	bne .Lthread
	cmp %o1, %g3
	bl .Lthread

	/* Now we will loop, unwinding the register windows up the stack
	 * until the restored %fp value matches the target value in %g3.  */

.Lloop:
	cmp %fp, %g3		/* Have we reached the target frame? */
	bl,a .Lloop		/* Loop while current fp is below target.  */
	 restore		/* Unwind register window in delay slot.  */
	be,a .Lfound		/* Better have hit it exactly.  */
	 ld ENV(g1,JB_SP), %o0	/* Delay slot: extract target SP.  */

.Lthread:
	/*
	 * Do a "flush register windows trap".  The trap handler in the
	 * kernel writes all the register windows to their stack slots, and
	 * marks them all as invalid (needing to be sucked up from the
	 * stack when used).  This ensures that all information needed to
	 * unwind to these callers is in memory, not in the register
	 * windows.
	 */
	ta	ST_FLUSH_WINDOWS
	ld	ENV(g1,JB_PC), %o7 /* Set return PC. */
	ld	ENV(g1,JB_SP), %fp /* Set saved SP on restore below. */
	sub	%fp, 64, %sp	/* Allocate a register frame. */
	st	%g3, RW_FP	/* Set saved FP on restore below. */
	retl
	restore %g2, 0, %o0	/* Restore values from above register frame. */

.Lfound:
	/* We have unwound register windows so %fp matches the target.  */
	mov %o0, %sp		/* OK, install new SP.  */

.Lsp_ok:
	ld ENV(g1,JB_PC), %o0	/* Extract target return PC.  */
	jmp %o0 + 8		/* Return there.  */
	 mov %g2, %o0		/* Delay slot: set return value.  */

.size __longjmp,.-__longjmp

libc_hidden_def(__longjmp)
