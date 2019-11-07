/*
 * Copyright (c) 2002 ARM Ltd
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the company may not be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ARM LTD ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ARM LTD BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Adapted for uClibc from NetBSD memcmp.S, version 1.2 2003/04/05
 * by Erik Andersen <andersen@codepoet.org>
 */

#include <features.h>
#include <bits/arm_asm.h>
#include <bits/arm_bx.h>

.text
.global memcmp
.type memcmp,%function
.align 4

#if defined(THUMB1_ONLY)
.thumb_func
memcmp:
	cmp	r2, #0
	bne	1f
	mov	r0, #0
	bx	lr
1:
	push	{r4}
	add	r4, r0, r2
2:
	ldrb	r2, [r0]
	add	r0, r0, #1
	ldrb	r3, [r1]
	add	r1, r1, #1
	cmp	r4, r0
	beq	3f
	cmp	r2, r3
	beq	2b
3:
	sub	r0, r2, r3
        pop	{r4}
	bx	lr
#else
memcmp:
	/* if ((len - 1) < 0) return 0 */
	subs	r2, r2, #1
	IT(tt, mi)
	movmi	r0, #0
	BXC(mi, lr)
	/* ip == last src address to compare */
	add	ip, r0, r2
1:
	ldrb	r2, [r0], #1
	ldrb	r3, [r1], #1
	cmp	ip, r0
	IT(t, cs)
	cmpcs	r2, r3
	beq	1b
	sub	r0, r2, r3
	BX(lr)
#endif

.size memcmp,.-memcmp

libc_hidden_def(memcmp)
#ifdef __UCLIBC_SUSV3_LEGACY__
strong_alias(memcmp,bcmp)
#endif
