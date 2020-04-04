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
 * Adapted for uClibc from NetBSD strcmp.S, version 1.3 2003/04/05
 * by Erik Andersen <andersen@codepoet.org>
 */

#include <features.h>
#include <bits/arm_asm.h>
#include <bits/arm_bx.h>

.text
.global strcmp
.type strcmp,%function
.align 4

#if defined(__thumb__) && !defined(__thumb2__)
.thumb_func
strcmp:
1:
	ldrb	r2, [r0]
	add	r0, r0, #1
	ldrb	r3, [r1]
	add	r1, r1, #1
	cmp	r2, #0
	beq	2f
	cmp	r2, r3
	beq	1b
2:
	sub	r0, r2, r3
	bx      lr
#else
strcmp:
1:
	ldrb	r2, [r0], #1
	ldrb	r3, [r1], #1
	cmp	r2, #1
	IT(t, cs)
	cmpcs	r2, r3
	beq	1b
	sub	r0, r2, r3
	BX(lr)
#endif

.size strcmp,.-strcmp

libc_hidden_def(strcmp)
#ifndef __UCLIBC_HAS_LOCALE__
strong_alias(strcmp,strcoll)
libc_hidden_def(strcoll)
#endif
