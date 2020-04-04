/* Copyright (C) 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Code contributed by Matthew Wilcox <willy@odie.barnet.ac.uk>

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

#include <features.h>
#include <endian.h>
#include <bits/arm_asm.h>
#include <bits/arm_bx.h>

/* size_t strlen(const char *S)
 * entry: r0 -> string
 * exit: r0 = len
 */

.text
.global strlen
.type strlen,%function
.align 4

#if defined(THUMB1_ONLY)
/* A simple implementation for when the ARM implementation can't be used.  */
.thumb_func
strlen:
	mov r2, #0
1:
	ldrb	r1, [r0, r2]
	add	r2, r2, #1
	cmp	r1, #0
	bne	1b
	sub	r0, r2, #1
	bx lr
#else
strlen:
	bic     r1, r0, $3              @ addr of word containing first byte
	ldr     r2, [r1], $4            @ get the first word
	ands    r3, r0, $3              @ how many bytes are duff?
	rsb     r0, r3, $0              @ get - that number into counter.
	beq     Laligned                @ skip into main check routine if no
					@ more
#if __BYTE_ORDER == __BIG_ENDIAN
	orr     r2, r2, $0xff000000     @ set this byte to non-zero
	subs    r3, r3, $1              @ any more to do?
	IT(t, gt)
	orrgt   r2, r2, $0x00ff0000     @ if so, set this byte
	subs    r3, r3, $1              @ more?
	IT(t, gt)
	orrgt   r2, r2, $0x0000ff00     @ then set.
#else
	orr     r2, r2, $0x000000ff     @ set this byte to non-zero
	subs    r3, r3, $1              @ any more to do?
	IT(t, gt)
	orrgt   r2, r2, $0x0000ff00     @ if so, set this byte
	subs    r3, r3, $1              @ more?
	IT(t, gt)
	orrgt   r2, r2, $0x00ff0000     @ then set.
#endif
Laligned:				@ here, we have a word in r2.  Does it
	tst     r2, $0x000000ff         @ contain any zeroes?
	IT(tttt, ne)
	tstne   r2, $0x0000ff00         @
	tstne   r2, $0x00ff0000         @
	tstne   r2, $0xff000000         @
	addne   r0, r0, $4              @ if not, the string is 4 bytes longer
	IT(t, ne)
	ldrne   r2, [r1], $4            @ and we continue to the next word
	bne     Laligned                @
Llastword:				@ drop through to here once we find a
#if __BYTE_ORDER == __BIG_ENDIAN
	tst     r2, $0xff000000         @ word that has a zero byte in it
	IT(tttt, ne)
	addne   r0, r0, $1              @
	tstne   r2, $0x00ff0000         @ and add up to 3 bytes on to it
	addne   r0, r0, $1              @
	tstne   r2, $0x0000ff00         @ (if first three all non-zero, 4th
	IT(t, ne)
	addne   r0, r0, $1              @  must be zero)
#else
	tst     r2, $0x000000ff         @
	IT(tttt, ne)
	addne   r0, r0, $1              @
	tstne   r2, $0x0000ff00         @ and add up to 3 bytes on to it
	addne   r0, r0, $1              @
	tstne   r2, $0x00ff0000         @ (if first three all non-zero, 4th
	IT(t, ne)
	addne   r0, r0, $1              @  must be zero)
#endif
	BX(lr)
#endif

.size strlen,.-strlen

libc_hidden_def(strlen)
