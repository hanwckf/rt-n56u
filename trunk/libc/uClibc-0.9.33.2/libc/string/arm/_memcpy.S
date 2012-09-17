/*-
 * Copyright (c) 1997 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Neil A. Carson and Mark Brinicombe
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *        This product includes software developed by the NetBSD
 *        Foundation, Inc. and its contributors.
 * 4. Neither the name of The NetBSD Foundation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Adapted for uClibc from NetBSD _memcpy.S,v 1.6 2003/10/09
 * by Erik Andersen <andersen@codepoet.org>
 */

#include <features.h>
#include <endian.h>
#include <bits/arm_asm.h>

#if !defined(THUMB1_ONLY)
/*
 * This is one fun bit of code ...
 * Some easy listening music is suggested while trying to understand this
 * code e.g. Iron Maiden
 *
 * For anyone attempting to understand it :
 *
 * The core code is implemented here with simple stubs for memcpy()
 * memmove() and bcopy().
 *
 * All local labels are prefixed with Lmemcpy_
 * Following the prefix a label starting f is used in the forward copy code
 * while a label using b is used in the backwards copy code
 * The source and destination addresses determine whether a forward or
 * backward copy is performed.
 * Separate bits of code are used to deal with the following situations
 * for both the forward and backwards copy.
 * unaligned source address
 * unaligned destination address
 * Separate copy routines are used to produce an optimised result for each
 * of these cases.
 * The copy code will use LDM/STM instructions to copy up to 32 bytes at
 * a time where possible.
 *
 * Note: r12 (aka ip) can be trashed during the function along with
 * r0-r3 although r0-r2 have defined uses i.e. dest, src, len throughout.
 * Additional registers are preserved prior to use i.e. r4, r5 & lr
 * The return value in r0 must be the destination address.
 *
 * Apologies for the state of the comments ;-)
 */

.text
.global _memcpy
.hidden _memcpy
.type _memcpy,%function
.align 4

/* XXX: The Thumb-2 conditionals can be removed if/when we require an
   assembler that supports unified syntax.  */
.macro copy regs
#if defined(__thumb2__)
	ittt	ge
	ldmiage	r1!, \regs
	stmiage	r0!, \regs
#else
	ldmgeia	r1!, \regs
	stmgeia	r0!, \regs
#endif
.endm

.macro copydb regs
#if defined(__thumb2__)
	ittt	ge
	ldmdbge	r1!, \regs
	stmdbge	r0!, \regs
#else
	ldmgedb	r1!, \regs
	stmgedb	r0!, \regs
#endif
.endm

_memcpy:
	/* Determine copy direction */
	cmp	r1, r0
	bcc	.Lmemcpy_backwards

	IT(t, eq)			/* Quick abort for src=dst */
#if defined(__USE_BX__)
        bxeq    lr
#else
        moveq   pc, lr
#endif
	stmdb	sp!, {r0, lr}		/* memcpy() returns dest addr */
	subs	r2, r2, #4
	blt	.Lmemcpy_fl4		/* less than 4 bytes */
	ands	r12, r0, #3
	bne	.Lmemcpy_fdestul	/* oh unaligned destination addr */
	ands	r12, r1, #3
	bne	.Lmemcpy_fsrcul		/* oh unaligned source addr */

.Lmemcpy_ft8:
	/* We have aligned source and destination */
	subs	r2, r2, #8
	blt	.Lmemcpy_fl12		/* less than 12 bytes (4 from above) */
	subs	r2, r2, #0x14         
	blt	.Lmemcpy_fl32		/* less than 32 bytes (12 from above) */
	str	r4, [sp, #-4]!		/* borrow r4 */

	/* blat 32 bytes at a time */
	/* XXX for really big copies perhaps we should use more registers */
.Lmemcpy_floop32:	
	ldmia	r1!, {r3, r4, r12, lr}
	stmia	r0!, {r3, r4, r12, lr}
	ldmia	r1!, {r3, r4, r12, lr}
	stmia	r0!, {r3, r4, r12, lr}
	subs	r2, r2, #0x20         
	bge	.Lmemcpy_floop32

	cmn	r2, #0x10
	/* blat a remaining 16 bytes */
	copy	"{r3, r4, r12, lr}"
	subge	r2, r2, #0x10         
	ldr	r4, [sp], #4		/* restore r4 */

.Lmemcpy_fl32:
	adds	r2, r2, #0x14         

	/* blat 12 bytes at a time */
.Lmemcpy_floop12:
	copy	"{r3, r12, lr}"
#if defined(__thumb2__)
	subsge	r2, r2, #0x0c         
#else
	subges	r2, r2, #0x0c         
#endif
	bge	.Lmemcpy_floop12

.Lmemcpy_fl12:
	adds	r2, r2, #8
	blt	.Lmemcpy_fl4

	subs	r2, r2, #4
	IT(tt, lt)
	ldrlt	r3, [r1], #4
	strlt	r3, [r0], #4
	copy	"{r3, r12}"
	subge	r2, r2, #4

.Lmemcpy_fl4:
	/* less than 4 bytes to go */
	adds	r2, r2, #4
#if defined(__thumb2__)
	it	eq
	popeq	{r0, pc}		/* done */
#elif defined(__ARM_ARCH_4T__)
	ldmeqia	sp!, {r0, r3}		/* done */
	bxeq	r3
#else
	ldmeqia	sp!, {r0, pc}		/* done */
#endif

	/* copy the crud byte at a time */
	cmp	r2, #2
	ldrb	r3, [r1], #1
	strb	r3, [r0], #1
#if defined(__thumb2__)
	itt	ge
	ldrbge	r3, [r1], #1
	strbge	r3, [r0], #1
	itt	gt
	ldrbgt	r3, [r1], #1
	strbgt	r3, [r0], #1
#else
	ldrgeb	r3, [r1], #1
	strgeb	r3, [r0], #1
	ldrgtb	r3, [r1], #1
	strgtb	r3, [r0], #1
#endif
#if defined(__ARM_ARCH_4T__)
	ldmia	sp!, {r0, r3}
	bx	r3
#else
	ldmia	sp!, {r0, pc}
#endif

	/* erg - unaligned destination */
.Lmemcpy_fdestul:
	rsb	r12, r12, #4
	cmp	r12, #2

	/* align destination with byte copies */
	ldrb	r3, [r1], #1
	strb	r3, [r0], #1
#if defined(__thumb2__)
	itt	ge
	ldrbge	r3, [r1], #1
	strbge	r3, [r0], #1
	itt	gt
	ldrbgt	r3, [r1], #1
	strbgt	r3, [r0], #1
#else
	ldrgeb	r3, [r1], #1
	strgeb	r3, [r0], #1
	ldrgtb	r3, [r1], #1
	strgtb	r3, [r0], #1
#endif
	subs	r2, r2, r12
	blt	.Lmemcpy_fl4		/* less the 4 bytes */

	ands	r12, r1, #3
	beq	.Lmemcpy_ft8		/* we have an aligned source */

	/* erg - unaligned source */
	/* This is where it gets nasty ... */
.Lmemcpy_fsrcul:
	bic	r1, r1, #3
	ldr	lr, [r1], #4
	cmp	r12, #2
	bgt	.Lmemcpy_fsrcul3
	beq	.Lmemcpy_fsrcul2
	cmp	r2, #0x0c            
	blt	.Lmemcpy_fsrcul1loop4
	sub	r2, r2, #0x0c         
	stmdb	sp!, {r4, r5}

.Lmemcpy_fsrcul1loop16:
#if __BYTE_ORDER == __BIG_ENDIAN
	mov	r3, lr, lsl #8
	ldmia	r1!, {r4, r5, r12, lr}
	orr	r3, r3, r4, lsr #24
	mov	r4, r4, lsl #8
	orr	r4, r4, r5, lsr #24
	mov	r5, r5, lsl #8
	orr	r5, r5, r12, lsr #24
	mov	r12, r12, lsl #8
	orr	r12, r12, lr, lsr #24
#else
	mov	r3, lr, lsr #8
	ldmia	r1!, {r4, r5, r12, lr}
	orr	r3, r3, r4, lsl #24
	mov	r4, r4, lsr #8
	orr	r4, r4, r5, lsl #24
	mov	r5, r5, lsr #8
	orr	r5, r5, r12, lsl #24
	mov	r12, r12, lsr #8
	orr	r12, r12, lr, lsl #24
#endif
	stmia	r0!, {r3-r5, r12}
	subs	r2, r2, #0x10         
	bge	.Lmemcpy_fsrcul1loop16
	ldmia	sp!, {r4, r5}
	adds	r2, r2, #0x0c         
	blt	.Lmemcpy_fsrcul1l4

.Lmemcpy_fsrcul1loop4:
#if __BYTE_ORDER == __BIG_ENDIAN
	mov	r12, lr, lsl #8
	ldr	lr, [r1], #4
	orr	r12, r12, lr, lsr #24
#else
	mov	r12, lr, lsr #8
	ldr	lr, [r1], #4
	orr	r12, r12, lr, lsl #24
#endif
	str	r12, [r0], #4
	subs	r2, r2, #4
	bge	.Lmemcpy_fsrcul1loop4

.Lmemcpy_fsrcul1l4:
	sub	r1, r1, #3
	b	.Lmemcpy_fl4

.Lmemcpy_fsrcul2:
	cmp	r2, #0x0c            
	blt	.Lmemcpy_fsrcul2loop4
	sub	r2, r2, #0x0c         
	stmdb	sp!, {r4, r5}

.Lmemcpy_fsrcul2loop16:
#if __BYTE_ORDER == __BIG_ENDIAN
	mov	r3, lr, lsl #16
	ldmia	r1!, {r4, r5, r12, lr}
	orr	r3, r3, r4, lsr #16
	mov	r4, r4, lsl #16
	orr	r4, r4, r5, lsr #16
	mov	r5, r5, lsl #16
	orr	r5, r5, r12, lsr #16
	mov	r12, r12, lsl #16
	orr	r12, r12, lr, lsr #16
#else
	mov	r3, lr, lsr #16
	ldmia	r1!, {r4, r5, r12, lr}
	orr	r3, r3, r4, lsl #16
	mov	r4, r4, lsr #16
	orr	r4, r4, r5, lsl #16
	mov	r5, r5, lsr #16
	orr	r5, r5, r12, lsl #16
	mov	r12, r12, lsr #16
	orr	r12, r12, lr, lsl #16
#endif
	stmia	r0!, {r3-r5, r12}
	subs	r2, r2, #0x10         
	bge	.Lmemcpy_fsrcul2loop16
	ldmia	sp!, {r4, r5}
	adds	r2, r2, #0x0c         
	blt	.Lmemcpy_fsrcul2l4

.Lmemcpy_fsrcul2loop4:
#if __BYTE_ORDER == __BIG_ENDIAN
	mov	r12, lr, lsl #16
	ldr	lr, [r1], #4
	orr	r12, r12, lr, lsr #16
#else
	mov	r12, lr, lsr #16
	ldr	lr, [r1], #4
	orr	r12, r12, lr, lsl #16
#endif
	str	r12, [r0], #4
	subs	r2, r2, #4
	bge	.Lmemcpy_fsrcul2loop4

.Lmemcpy_fsrcul2l4:
	sub	r1, r1, #2
	b	.Lmemcpy_fl4

.Lmemcpy_fsrcul3:
	cmp	r2, #0x0c            
	blt	.Lmemcpy_fsrcul3loop4
	sub	r2, r2, #0x0c         
	stmdb	sp!, {r4, r5}

.Lmemcpy_fsrcul3loop16:
#if __BYTE_ORDER == __BIG_ENDIAN
	mov	r3, lr, lsl #24
	ldmia	r1!, {r4, r5, r12, lr}
	orr	r3, r3, r4, lsr #8
	mov	r4, r4, lsl #24
	orr	r4, r4, r5, lsr #8
	mov	r5, r5, lsl #24
	orr	r5, r5, r12, lsr #8
	mov	r12, r12, lsl #24
	orr	r12, r12, lr, lsr #8
#else
	mov	r3, lr, lsr #24
	ldmia	r1!, {r4, r5, r12, lr}
	orr	r3, r3, r4, lsl #8
	mov	r4, r4, lsr #24
	orr	r4, r4, r5, lsl #8
	mov	r5, r5, lsr #24
	orr	r5, r5, r12, lsl #8
	mov	r12, r12, lsr #24
	orr	r12, r12, lr, lsl #8
#endif
	stmia	r0!, {r3-r5, r12}
	subs	r2, r2, #0x10         
	bge	.Lmemcpy_fsrcul3loop16
	ldmia	sp!, {r4, r5}
	adds	r2, r2, #0x0c         
	blt	.Lmemcpy_fsrcul3l4

.Lmemcpy_fsrcul3loop4:
#if __BYTE_ORDER == __BIG_ENDIAN
	mov	r12, lr, lsl #24
	ldr	lr, [r1], #4
	orr	r12, r12, lr, lsr #8
#else
	mov	r12, lr, lsr #24
	ldr	lr, [r1], #4
	orr	r12, r12, lr, lsl #8
#endif
	str	r12, [r0], #4
	subs	r2, r2, #4
	bge	.Lmemcpy_fsrcul3loop4

.Lmemcpy_fsrcul3l4:
	sub	r1, r1, #1
	b	.Lmemcpy_fl4

.Lmemcpy_backwards:
	add	r1, r1, r2
	add	r0, r0, r2
	subs	r2, r2, #4
	blt	.Lmemcpy_bl4		/* less than 4 bytes */
	ands	r12, r0, #3
	bne	.Lmemcpy_bdestul	/* oh unaligned destination addr */
	ands	r12, r1, #3
	bne	.Lmemcpy_bsrcul		/* oh unaligned source addr */

.Lmemcpy_bt8:
	/* We have aligned source and destination */
	subs	r2, r2, #8
	blt	.Lmemcpy_bl12		/* less than 12 bytes (4 from above) */
	stmdb	sp!, {r4, lr}
	subs	r2, r2, #0x14		/* less than 32 bytes (12 from above) */
	blt	.Lmemcpy_bl32

	/* blat 32 bytes at a time */
	/* XXX for really big copies perhaps we should use more registers */
.Lmemcpy_bloop32:
	ldmdb	r1!, {r3, r4, r12, lr}
	stmdb	r0!, {r3, r4, r12, lr}
	ldmdb	r1!, {r3, r4, r12, lr}
	stmdb	r0!, {r3, r4, r12, lr}
	subs	r2, r2, #0x20         
	bge	.Lmemcpy_bloop32

.Lmemcpy_bl32:
	cmn	r2, #0x10            
	/* blat a remaining 16 bytes */
	copydb	"{r3, r4, r12, lr}"
	subge	r2, r2, #0x10         
	adds	r2, r2, #0x14         
	/* blat a remaining 12 bytes */
	copydb	"{r3, r12, lr}"
	subge	r2, r2, #0x0c         
	ldmia	sp!, {r4, lr}

.Lmemcpy_bl12:
	adds	r2, r2, #8
	blt	.Lmemcpy_bl4
	subs	r2, r2, #4
	IT(tt, lt)
	ldrlt	r3, [r1, #-4]!
	strlt	r3, [r0, #-4]!
	copydb	"{r3, r12}"
	subge	r2, r2, #4

.Lmemcpy_bl4:
	/* less than 4 bytes to go */
	adds	r2, r2, #4
	IT(t, eq)
#if defined(__USE_BX__)
        bxeq    lr
#else
	moveq	pc, lr			/* done */
#endif
	/* copy the crud byte at a time */
	cmp	r2, #2
	ldrb	r3, [r1, #-1]!
	strb	r3, [r0, #-1]!
#ifdef __thumb2__
	itt	ge
	ldrbge	r3, [r1, #-1]!
	strbge	r3, [r0, #-1]!
	itt	gt
	ldrbgt	r3, [r1, #-1]!
	strbgt	r3, [r0, #-1]!
#else
	ldrgeb	r3, [r1, #-1]!
	strgeb	r3, [r0, #-1]!
	ldrgtb	r3, [r1, #-1]!
	strgtb	r3, [r0, #-1]!
#endif
#if defined(__USE_BX__)
        bx      lr
#else
	mov	pc, lr
#endif
	/* erg - unaligned destination */
.Lmemcpy_bdestul:
	cmp	r12, #2

	/* align destination with byte copies */
	ldrb	r3, [r1, #-1]!
	strb	r3, [r0, #-1]!
#ifdef __thumb2__
	itt	ge
	ldrbge	r3, [r1, #-1]!
	strbge	r3, [r0, #-1]!
	itt	gt
	ldrbgt	r3, [r1, #-1]!
	strbgt	r3, [r0, #-1]!
#else
	ldrgeb	r3, [r1, #-1]!
	strgeb	r3, [r0, #-1]!
	ldrgtb	r3, [r1, #-1]!
	strgtb	r3, [r0, #-1]!
#endif
	subs	r2, r2, r12
	blt	.Lmemcpy_bl4		/* less than 4 bytes to go */
	ands	r12, r1, #3
	beq	.Lmemcpy_bt8		/* we have an aligned source */

	/* erg - unaligned source */
	/* This is where it gets nasty ... */
.Lmemcpy_bsrcul:
	bic	r1, r1, #3
	ldr	r3, [r1, #0]
	cmp	r12, #2
	blt	.Lmemcpy_bsrcul1
	beq	.Lmemcpy_bsrcul2
	cmp	r2, #0x0c            
	blt	.Lmemcpy_bsrcul3loop4
	sub	r2, r2, #0x0c         
	stmdb	sp!, {r4, r5, lr}

.Lmemcpy_bsrcul3loop16:
#if __BYTE_ORDER == __BIG_ENDIAN
	mov	lr, r3, lsr #8
	ldmdb	r1!, {r3-r5, r12}
	orr	lr, lr, r12, lsl #24
	mov	r12, r12, lsr #8
	orr	r12, r12, r5, lsl #24
	mov	r5, r5, lsr #8
	orr	r5, r5, r4, lsl #24
	mov	r4, r4, lsr #8
	orr	r4, r4, r3, lsl #24
#else
	mov	lr, r3, lsl #8
	ldmdb	r1!, {r3-r5, r12}
	orr	lr, lr, r12, lsr #24
	mov	r12, r12, lsl #8
	orr	r12, r12, r5, lsr #24
	mov	r5, r5, lsl #8
	orr	r5, r5, r4, lsr #24
	mov	r4, r4, lsl #8
	orr	r4, r4, r3, lsr #24
#endif
	stmdb	r0!, {r4, r5, r12, lr}
	subs	r2, r2, #0x10         
	bge	.Lmemcpy_bsrcul3loop16
	ldmia	sp!, {r4, r5, lr}
	adds	r2, r2, #0x0c         
	blt	.Lmemcpy_bsrcul3l4

.Lmemcpy_bsrcul3loop4:
#if __BYTE_ORDER == __BIG_ENDIAN
	mov	r12, r3, lsr #8
	ldr	r3, [r1, #-4]!
	orr	r12, r12, r3, lsl #24
#else
	mov	r12, r3, lsl #8
	ldr	r3, [r1, #-4]!
	orr	r12, r12, r3, lsr #24
#endif
	str	r12, [r0, #-4]!
	subs	r2, r2, #4
	bge	.Lmemcpy_bsrcul3loop4

.Lmemcpy_bsrcul3l4:
	add	r1, r1, #3
	b	.Lmemcpy_bl4

.Lmemcpy_bsrcul2:
	cmp	r2, #0x0c            
	blt	.Lmemcpy_bsrcul2loop4
	sub	r2, r2, #0x0c         
	stmdb	sp!, {r4, r5, lr}

.Lmemcpy_bsrcul2loop16:
#if __BYTE_ORDER == __BIG_ENDIAN
	mov	lr, r3, lsr #16
	ldmdb	r1!, {r3-r5, r12}
	orr	lr, lr, r12, lsl #16
	mov	r12, r12, lsr #16
	orr	r12, r12, r5, lsl #16
	mov	r5, r5, lsr #16
	orr	r5, r5, r4, lsl #16
	mov	r4, r4, lsr #16
	orr	r4, r4, r3, lsl #16
#else
	mov	lr, r3, lsl #16
	ldmdb	r1!, {r3-r5, r12}
	orr	lr, lr, r12, lsr #16
	mov	r12, r12, lsl #16
	orr	r12, r12, r5, lsr #16
	mov	r5, r5, lsl #16
	orr	r5, r5, r4, lsr #16
	mov	r4, r4, lsl #16
	orr	r4, r4, r3, lsr #16
#endif
	stmdb	r0!, {r4, r5, r12, lr}
	subs	r2, r2, #0x10         
	bge	.Lmemcpy_bsrcul2loop16
	ldmia	sp!, {r4, r5, lr}
	adds	r2, r2, #0x0c         
	blt	.Lmemcpy_bsrcul2l4

.Lmemcpy_bsrcul2loop4:
#if __BYTE_ORDER == __BIG_ENDIAN
	mov	r12, r3, lsr #16
	ldr	r3, [r1, #-4]!
	orr	r12, r12, r3, lsl #16
#else
	mov	r12, r3, lsl #16
	ldr	r3, [r1, #-4]!
	orr	r12, r12, r3, lsr #16
#endif
	str	r12, [r0, #-4]!
	subs	r2, r2, #4
	bge	.Lmemcpy_bsrcul2loop4

.Lmemcpy_bsrcul2l4:
	add	r1, r1, #2
	b	.Lmemcpy_bl4

.Lmemcpy_bsrcul1:
	cmp	r2, #0x0c            
	blt	.Lmemcpy_bsrcul1loop4
	sub	r2, r2, #0x0c         
	stmdb	sp!, {r4, r5, lr}

.Lmemcpy_bsrcul1loop32:
#if __BYTE_ORDER == __BIG_ENDIAN
	mov	lr, r3, lsr #24
	ldmdb	r1!, {r3-r5, r12}
	orr	lr, lr, r12, lsl #8
	mov	r12, r12, lsr #24
	orr	r12, r12, r5, lsl #8
	mov	r5, r5, lsr #24
	orr	r5, r5, r4, lsl #8
	mov	r4, r4, lsr #24
	orr	r4, r4, r3, lsl #8
#else
	mov	lr, r3, lsl #24
	ldmdb	r1!, {r3-r5, r12}
	orr	lr, lr, r12, lsr #8
	mov	r12, r12, lsl #24
	orr	r12, r12, r5, lsr #8
	mov	r5, r5, lsl #24
	orr	r5, r5, r4, lsr #8
	mov	r4, r4, lsl #24
	orr	r4, r4, r3, lsr #8
#endif
	stmdb	r0!, {r4, r5, r12, lr}
	subs	r2, r2, #0x10         
	bge	.Lmemcpy_bsrcul1loop32
	ldmia	sp!, {r4, r5, lr}
	adds	r2, r2, #0x0c         
	blt	.Lmemcpy_bsrcul1l4

.Lmemcpy_bsrcul1loop4:
#if __BYTE_ORDER == __BIG_ENDIAN
	mov	r12, r3, lsr #24
	ldr	r3, [r1, #-4]!
	orr	r12, r12, r3, lsl #8
#else
	mov	r12, r3, lsl #24
	ldr	r3, [r1, #-4]!
	orr	r12, r12, r3, lsr #8
#endif
	str	r12, [r0, #-4]!
	subs	r2, r2, #4
	bge	.Lmemcpy_bsrcul1loop4

.Lmemcpy_bsrcul1l4:
	add	r1, r1, #1
	b	.Lmemcpy_bl4

#else /* THUMB1_ONLY */

/* This is a fairly dumb implementation for when we can't use the 32-bit code
   above.  */
.text
.global _memcpy
.hidden _memcpy
.type _memcpy,%function
.align 4
.thumb
_memcpy:
	push	{r0, r4}
	cmp	r2, #0
	beq	.Lmemcpy_exit
	@ See if we have overlapping regions, and need to reverse the
	@ direction of the copy
	cmp	r0, r1
	bls	.Lmemcpy_forwards
	add	r4, r1, r2
	cmp	r0, r4
	bcc	.Lmemcpy_backwards
.Lmemcpy_forwards:
	/* Forwards.  */
	mov	r3, r0
	eor	r3, r1
	mov	r4, #3
	tst	r3, r4
	bne	.Lmemcpy_funaligned
	cmp	r2, #8
	bcc	.Lmemcpy_funaligned
1:	@ copy up to the first word boundary.
	tst	r0, r4
	beq	1f
	ldrb	r3, [r1]
	add	r1, r1, #1
	strb	r3, [r0]
	add	r0, r0, #1
	sub	r2, r2, #1
	b	1b
1:	@ Copy aligned words
	ldr	r3, [r1]
	add	r1, r1, #4
	str	r3, [r0]
	add	r0, r0, #4
	sub	r2, r2, #4
	cmp	r2, #4
	bcs	1b
	cmp	r2, #0
	beq	.Lmemcpy_exit
.Lmemcpy_funaligned:
1:
	ldrb	r3, [r1]
	add	r1, r1, #1
	strb	r3, [r0]
	add	r0, r0, #1
	sub	r2, r2, #1
	bne	1b
.Lmemcpy_exit:
	pop	{r0, r4}
	bx	lr

.Lmemcpy_backwards:
	add	r0, r0, r2
	add	r1, r1, r2
1:
	sub	r0, r0, #1
	sub	r1, r1, #1
	ldrb	r3, [r1]
	strb	r3, [r0]
	sub	r2, r2, #1
	bne	1b
	b	.Lmemcpy_exit
#endif
