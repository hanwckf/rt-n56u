/* Optimized strlen for Xtensa.
   Copyright (C) 2001, 2007 Free Software Foundation, Inc.
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
   Software Foundation, Inc., 51 Franklin Street - Fifth Floor,
   Boston, MA 02110-1301, USA.  */

#include "../../sysdeps/linux/xtensa/sysdep.h"
#include <bits/xtensa-config.h>

#ifdef __XTENSA_EB__
#define	MASK0 0xff000000
#define	MASK1 0x00ff0000
#define	MASK2 0x0000ff00
#define	MASK3 0x000000ff
#else
#define	MASK0 0x000000ff
#define	MASK1 0x0000ff00
#define	MASK2 0x00ff0000
#define	MASK3 0xff000000
#endif

	.text
ENTRY (strlen)
	/* a2 = s */

	addi	a3, a2, -4	/* because we overincrement at the end */
	movi	a4, MASK0
	movi	a5, MASK1
	movi	a6, MASK2
	movi	a7, MASK3
	bbsi.l	a2, 0, .L1mod2
	bbsi.l	a2, 1, .L2mod4
	j	.Laligned

.L1mod2: /* address is odd */
	l8ui	a8, a3, 4	/* get byte 0 */
	addi	a3, a3, 1	/* advance string pointer */
	beqz	a8, .Lz3	/* if byte 0 is zero */
	bbci.l	a3, 1, .Laligned /* if string pointer is now word-aligned */

.L2mod4: /* address is 2 mod 4 */
	addi	a3, a3, 2	/* advance ptr for aligned access */
	l32i	a8, a3, 0	/* get word with first two bytes of string */
	bnone	a8, a6, .Lz2	/* if byte 2 (of word, not string) is zero */
	bany	a8, a7, .Laligned /* if byte 3 (of word, not string) is nonzero */

	/* Byte 3 is zero.  */
	addi	a3, a3, 3	/* point to zero byte */
	sub	a2, a3, a2	/* subtract to get length */
	retw


/* String is word-aligned.  */

	.align	4
	/* (2 mod 4) alignment for loop instruction */
.Laligned:
#if XCHAL_HAVE_LOOPS
	_movi.n	a8, 0		/* set up for the maximum loop count */
	loop	a8, .Lz3	/* loop forever (almost anyway) */
#endif
1:	l32i	a8, a3, 4	/* get next word of string */
	addi	a3, a3, 4	/* advance string pointer */
	bnone	a8, a4, .Lz0	/* if byte 0 is zero */
	bnone	a8, a5, .Lz1	/* if byte 1 is zero */
	bnone	a8, a6, .Lz2	/* if byte 2 is zero */
#if XCHAL_HAVE_LOOPS
	bnone	a8, a7, .Lz3	/* if byte 3 is zero */
#else
	bany	a8, a7, 1b	/* repeat if byte 3 is non-zero */
#endif

.Lz3:	/* Byte 3 is zero.  */
	addi	a3, a3, 3	/* point to zero byte */
	/* Fall through....  */

.Lz0:	/* Byte 0 is zero.  */
	sub	a2, a3, a2	/* subtract to get length */
	retw

.Lz1:	/* Byte 1 is zero.  */
	addi	a3, a3, 1	/* point to zero byte */
	sub	a2, a3, a2	/* subtract to get length */
	retw

.Lz2:	/* Byte 2 is zero.  */
	addi	a3, a3, 2	/* point to zero byte */
	sub	a2, a3, a2	/* subtract to get length */
	retw

libc_hidden_def (strlen)
