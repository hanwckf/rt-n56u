/* vi: set sw=8 ts=8: */
/*
 * ldso/ldso/sh64/resolve.S
 *
 * SuperH (sh64) dynamic resolver support
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
	.globl	_dl_linux_resolver
	.globl	_dl_linux_resolve
	.type	_dl_linux_resolve, @function

	.balign	16
_dl_linux_resolve:
	addi	r15, -72, r15		! make room on the stack
	pt	_dl_linux_resolver, tr0
	st.q	r15, 0, r2		! save regs
	st.q	r15, 8, r3
	st.q	r15, 16, r4
	st.q	r15, 24, r5
	st.q	r15, 32, r6
	st.q	r15, 40, r7
	st.q	r15, 48, r8
	st.q	r15, 56, r9
	st.q	r15, 64, r18

#ifdef HAVE_FPU
	addi	r15, -48, r15		! make room for FP regs
	fst.d	r15, 0, dr0		! save FP regs
	fst.d	r15, 8, dr2
	fst.d	r15, 16, dr4
	fst.d	r15, 24, dr6
	fst.d	r15, 32, dr8
	fst.d	r15, 40, dr10
#endif

	/*
	 * Args for _dl_linux_resolver(), set in r17/r21 by PLT code
	 */

	add	r17, r63, r2		! link map address
	add	r21, r63, r3		! GOT offset
	blink	tr0, r18		! call _dl_linux_resolver()
	ptabs/l	r2, tr0			! save result = addr of function called

#ifdef HAVE_FPU
	fld.d	r15, 0, dr0		! restore FP regs
	fld.d	r15, 8, dr2
	fld.d	r15, 16, dr4
	fld.d	r15, 24, dr6
	fld.d	r15, 32, dr8
	fld.d	r15, 40, dr10
	addi	r15, 48, r15
#endif

	ld.q	r15, 0, r2		! restore regs
	ld.q	r15, 8, r3
	ld.q	r15, 16, r4
	ld.q	r15, 24, r5
	ld.q	r15, 32, r6
	ld.q	r15, 40, r7
	ld.q	r15, 48, r8
	ld.q	r15, 56, r9
	ld.q	r15, 64, r18

	addi	r15, 72, r15
	blink	tr0, r63		! jump to function address

	.size	_dl_linux_resolve, . - _dl_linux_resolve

