/* vi: set sw=8 ts=8: */
/*
 * libc/string/sh64/strlen.S
 *
 * Simplistic strlen() implementation for SHmedia.
 *
 * Copyright (C) 2003  Paul Mundt <lethal@linux-sh.org>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. The name of the above contributors may not be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

	.section .text..SHmedia32,"ax"
	.globl	strlen
	.type	strlen,@function

	.balign 16
strlen:
	ptabs	r18, tr4

	/*
	 * Note: We could easily deal with the NULL case here with a simple
	 * sanity check, though it seems that the behavior we want is to fault
	 * in the event that r2 == NULL, so we don't bother.
	 */
/*	beqi    r2, 0, tr4 */	! Sanity check

	movi	-1, r0
	pta/l	loop, tr0
loop:
	ld.b	r2, 0, r1
	addi	r2, 1, r2
	addi	r0, 1, r0
	bnei/l	r1, 0, tr0

	or	r0, r63, r2
	blink	tr4, r63

	.size	strlen, . - strlen

