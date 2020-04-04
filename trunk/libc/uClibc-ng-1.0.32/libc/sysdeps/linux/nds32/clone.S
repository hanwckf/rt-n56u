/*
 * Copyright (C) 2016-2017 Andes Technology, Inc.
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/* Copyright (C) 2010-2014 Free Software Foundation, Inc.
   Contributed by Pat Beirne <patb@corelcomputer.com>

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

/* clone() is even more special than fork() as it mucks with stacks
   and invokes a function in the right context after its all over.  */

#include <sysdep.h>
#define _ERRNO_H	1
#include <bits/errno.h>

/* int clone(int (*fn)(void *), void *child_stack, int flags, void *arg);
   _syscall2(int, clone, int, flags, void *, child_stack)  */

ENTRY(__clone)
#ifdef __NDS32_ABI_2FP_PLUS__
	lwi	$r4, [$sp]
	lwi	$r5, [$sp+4]
#endif
#ifdef PIC
	/* set GP register to parent only, cause child's $SP will be $r1. */
	pushm	$fp, $gp
	cfi_adjust_cfa_offset(8)
	cfi_rel_offset(fp, 0)
	cfi_rel_offset(gp, 4)
	mfusr	$r15, $pc
	sethi	$gp, hi20(_GLOBAL_OFFSET_TABLE_+4)
	ori	$gp, $gp, lo12(_GLOBAL_OFFSET_TABLE_+8)
	add	$gp, $gp, $r15
#endif /* PIC  */

	/* sanity check arguments.  */
	beqz	$r0, 1f
	bnez	$r1, 2f

1:
	movi	$r0, -EINVAL

5:
#ifdef PIC
	/* restore GP register, only in parent's stack  */
	la	$r15, C_SYMBOL_NAME(__syscall_error@PLT)
	push	$lp
	cfi_adjust_cfa_offset(4)
	cfi_rel_offset(lp, 0)
	addi	$sp, $sp, -4
	cfi_adjust_cfa_offset(4)
	jral	$r15
	addi	$sp, $sp, 4
	cfi_adjust_cfa_offset(-4)
	pop	$lp
	cfi_adjust_cfa_offset(-4)
	cfi_restore(lp)
	popm	$fp, $gp
	cfi_adjust_cfa_offset(-8)
	cfi_restore(fp)
	cfi_restore(gp)
	ret
#else /* ! PIC  */
	la	$r15, C_SYMBOL_NAME(__syscall_error)
	jr	$r15
#endif /* ! PIC  */

2:
	/* Child's $sp will be $r1, make $sp 8-byte alignment */
	bitci	$r1, $r1, 7
	/* push to child's stack only.  */
	addi	$r1, $r1, -4
	swi.p	$r3, [$r1], -4			! arg
	swi	$r0, [$r1]			! fn

	/* do the system call  */
	or	$r0, $r2, $r2			! move $r0, $r2

	move    $r3, $r5
	move    $r5, $r2                        ! Use $r5 to backup $r2
						! The pt_regs is placed in $r5 in kerenl (sys_clone_wrapper)
	move    $r2, $r4

#ifdef __NDS32_ABI_2FP_PLUS__
# ifdef PIC
       lwi     $r4, [$sp+#0x10]
# else
       lwi     $r4, [$sp+#0x8]
# endif
#else
# ifdef PIC
       lwi     $r4, [$sp+#0x8]
# else
       lwi     $r4, [$sp]
# endif
#endif

	__do_syscall(clone)
	beqz    $r0, 4f
	bltz    $r0, 5b


10:
#ifdef PIC
	/* restore GP register, only in parent's stack  */
	popm	$fp, $gp
	cfi_adjust_cfa_offset(-8)
	cfi_restore(gp)
	cfi_restore(fp)
#endif /* PIC  */
	ret
4:
	/* Only in child's stack.  */
	pop	$r1				! fn
	pop	$r0				! arg


#if !defined(__NDS32_ABI_2__) && !defined(__NDS32_ABI_2FP_PLUS__)
	addi	$sp, $sp, -24
#endif /* !defined(__NDS32_ABI_2__) && !defined(__NDS32_ABI_2FP_PLUS__)  */

	! use $r15 in case _exit is PIC
	bral	$r1

#if !defined(__NDS32_ABI_2__) && !defined(__NDS32_ABI_2FP_PLUS__)
	addi	$sp, $sp, 24
#endif /* !defined(__NDS32_ABI_2__) && !defined(__NDS32_ABI_2FP_PLUS__)  */

	! use $r15 in case _exit is PIC
#ifdef PIC
	la	$r15, C_SYMBOL_NAME(_exit@PLT)
#else /* ! PIC  */
	la	$r15, C_SYMBOL_NAME(_exit)
#endif /* ! PIC  */
	jr	$r15


PSEUDO_END (__clone)
weak_alias (__clone, clone)
