/* Copyright (C) 1996, 1997 Free Software Foundation, Inc.
   Contributed by David Mosberger (davidm@cs.arizona.edu).
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

/* The current Alpha chips don't provide hardware for integer
   division.  The C compiler expects the functions

	__divqu: 64-bit unsigned long divide
	__remqu: 64-bit unsigned long remainder
	__divqs/__remqs: signed 64-bit
	__divlu/__remlu: unsigned 32-bit
	__divls/__remls: signed 32-bit

   These are not normal C functions: instead of the normal calling
   sequence, these expect their arguments in registers t10 and t11, and
   return the result in t12 (aka pv).  Register AT may be clobbered
   (assembly temporary), anything else must be saved.  */

#include <features.h>

#include <sys/regdef.h>
#ifdef __linux__
# include <asm/gentrap.h>
# include <asm/pal.h>
#else
# include <machine/pal.h>
#endif

#define mask			v0
#define divisor			t0
#define compare			AT
#define tmp1			t2
#define tmp2			t3
#define retaddr			t9
#define arg1			t10
#define arg2			t11
#define result			t12


#if IS_REM
# define DIV_ONLY(x,y...)
# define REM_ONLY(x,y...)	x,##y
# define modulus		result
# define quotient		t1
# define GETSIGN(x)		mov arg1, x
# define STACK			32
#else
# define DIV_ONLY(x,y...)	x,##y
# define REM_ONLY(x,y...)
# define modulus		t1
# define quotient		result
# define GETSIGN(x)		xor arg1, arg2, x
# define STACK			48
#endif

#if SIZE == 8
# define LONGIFY(x,y)		mov x,y
# define SLONGIFY(x,y)		mov x,y
# define _SLONGIFY(x)
# define NEG(x,y)		negq x,y
#else
# define LONGIFY(x,y)		zapnot x,15,y
# define SLONGIFY(x,y)		sextl x,y
# define _SLONGIFY(x)		sextl x,x
# define NEG(x,y)		negl x,y
#endif

	.set noreorder
	.set noat

	.ent UFUNC_NAME
	.globl UFUNC_NAME
#ifndef IS_IN_rtld
	.hidden UFUNC_NAME
#endif

	.align 3
UFUNC_NAME:
	lda	sp, -STACK(sp)
	.frame	sp, STACK, retaddr, 0
	.prologue 0

$udiv:
	stq	t0, 0(sp)
	LONGIFY	(arg2, divisor)
	stq	t1, 8(sp)
	LONGIFY	(arg1, modulus)
	stq	v0, 16(sp)
	clr	quotient
	stq	tmp1, 24(sp)
	ldiq	mask, 1
	DIV_ONLY(stq tmp2,32(sp))

	beq	divisor, $divbyzero

	.align 3
#if SIZE == 8
	/* Shift divisor left.  */
1:	cmpult	divisor, modulus, compare
	blt	divisor, 2f
	addq	divisor, divisor, divisor
	addq	mask, mask, mask
	bne	compare, 1b
	unop
2:
#else
	/* Shift divisor left using 3-bit shifts as we can't overflow.
	   This results in looping three times less here, but up to
	   two more times later.  Thus using a large shift isn't worth it.  */
1:	cmpult	divisor, modulus, compare
	s8addq	divisor, zero, divisor
	s8addq	mask, zero, mask
	bne	compare, 1b
#endif

	/* Now go back to the right.  */
3:	DIV_ONLY(addq quotient, mask, tmp2)
	srl	mask, 1, mask
	cmpule	divisor, modulus, compare
	subq	modulus, divisor, tmp1
	DIV_ONLY(cmovne compare, tmp2, quotient)
	srl	divisor, 1, divisor
	cmovne	compare, tmp1, modulus
	bne	mask, 3b

$done:	ldq	t0, 0(sp)
	ldq	t1, 8(sp)
	ldq	v0, 16(sp)
	ldq	tmp1, 24(sp)
	DIV_ONLY(ldq tmp2, 32(sp))
	lda	sp, STACK(sp)
	ret	zero, (retaddr), 1

$divbyzero:
	mov	a0, tmp1
	ldiq	a0, GEN_INTDIV
	call_pal PAL_gentrap
	mov	tmp1, a0
	clr	result			/* If trap returns, return zero.  */
	br	$done

	.end UFUNC_NAME

	.ent SFUNC_NAME
	.globl SFUNC_NAME

	.align 3
SFUNC_NAME:
	lda	sp, -STACK(sp)
	.frame	sp, STACK, retaddr, 0
	.prologue 0

	or	arg1, arg2, AT
	_SLONGIFY(AT)
	bge	AT, $udiv		/* don't need to mess with signs */

	/* Save originals and find absolute values.  */
	stq	arg1, 0(sp)
	NEG	(arg1, AT)
	stq	arg2, 8(sp)
	cmovge	AT, AT, arg1
	stq	retaddr, 16(sp)
	NEG	(arg2, AT)
	stq	tmp1, 24(sp)
	cmovge	AT, AT, arg2

	/* Do the unsigned division.  */
	bsr	retaddr, UFUNC_NAME

	/* Restore originals and adjust the sign of the result.  */
	ldq	arg1, 0(sp)
	ldq	arg2, 8(sp)
	GETSIGN	(AT)
	NEG	(result, tmp1)
	_SLONGIFY(AT)
	ldq	retaddr, 16(sp)
	cmovlt	AT, tmp1, result
	ldq	tmp1, 24(sp)

	lda	sp, STACK(sp)
	ret	zero, (retaddr), 1

	.end	SFUNC_NAME
