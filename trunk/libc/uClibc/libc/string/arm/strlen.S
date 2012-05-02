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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <endian.h>
#include <sys/syscall.h>

/* size_t strlen(const char *S)
 * entry: r0 -> string
 * exit: r0 = len
 */

		.text
		.global strlen;
		.type strlen,%function
		.align 4;                                                               \

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
	orrgt   r2, r2, $0x00ff0000     @ if so, set this byte
	subs    r3, r3, $1              @ more?
	orrgt   r2, r2, $0x0000ff00     @ then set.
#else
	orr     r2, r2, $0x000000ff     @ set this byte to non-zero
	subs    r3, r3, $1              @ any more to do?
	orrgt   r2, r2, $0x0000ff00     @ if so, set this byte
	subs    r3, r3, $1              @ more?
	orrgt   r2, r2, $0x00ff0000     @ then set.
#endif
Laligned:				@ here, we have a word in r2.  Does it
	tst     r2, $0x000000ff         @ contain any zeroes?
	tstne   r2, $0x0000ff00         @
	tstne   r2, $0x00ff0000         @
	tstne   r2, $0xff000000         @
	addne   r0, r0, $4              @ if not, the string is 4 bytes longer
	ldrne   r2, [r1], $4            @ and we continue to the next word
	bne     Laligned                @
Llastword:				@ drop through to here once we find a
#if __BYTE_ORDER == __BIG_ENDIAN
	tst     r2, $0xff000000         @ word that has a zero byte in it
	addne   r0, r0, $1              @
	tstne   r2, $0x00ff0000         @ and add up to 3 bytes on to it
	addne   r0, r0, $1              @
	tstne   r2, $0x0000ff00         @ (if first three all non-zero, 4th
	addne   r0, r0, $1              @  must be zero)
#else
	tst     r2, $0x000000ff         @
	addne   r0, r0, $1              @
	tstne   r2, $0x0000ff00         @ and add up to 3 bytes on to it
	addne   r0, r0, $1              @
	tstne   r2, $0x00ff0000         @ (if first three all non-zero, 4th
	addne   r0, r0, $1              @  must be zero)
#endif
	mov	pc,lr

.size strlen,.-strlen;

