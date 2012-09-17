/* Optimized strcpy for Xtensa.
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
ENTRY (strcpy)
	/* a2 = dst, a3 = src */

	mov	a10, a2		/* leave dst in return value register */
	movi	a4, MASK0
	movi	a5, MASK1
	movi	a6, MASK2
	movi	a7, MASK3
	bbsi.l	a3, 0, .Lsrc1mod2
	bbsi.l	a3, 1, .Lsrc2mod4
.Lsrcaligned:

	/* Check if the destination is aligned.  */
	movi	a8, 3
	bnone	a10, a8, .Laligned

	j	.Ldstunaligned

.Lsrc1mod2: /* src address is odd */
	l8ui	a8, a3, 0	/* get byte 0 */
	addi	a3, a3, 1	/* advance src pointer */
	s8i	a8, a10, 0	/* store byte 0 */
	beqz	a8, 1f		/* if byte 0 is zero */
	addi	a10, a10, 1	/* advance dst pointer */
	bbci.l	a3, 1, .Lsrcaligned /* if src is now word-aligned */

.Lsrc2mod4: /* src address is 2 mod 4 */
	l8ui	a8, a3, 0	/* get byte 0 */
	/* 1-cycle interlock */
	s8i	a8, a10, 0	/* store byte 0 */
	beqz	a8, 1f		/* if byte 0 is zero */
	l8ui	a8, a3, 1	/* get byte 0 */
	addi	a3, a3, 2	/* advance src pointer */
	s8i	a8, a10, 1	/* store byte 0 */
	addi	a10, a10, 2	/* advance dst pointer */
	bnez	a8, .Lsrcaligned
1:	retw


/* dst is word-aligned; src is word-aligned.  */

	.align	4
#if XCHAL_HAVE_LOOPS
	/* (2 mod 4) alignment for loop instruction */
.Laligned:
	_movi.n	a8, 0		/* set up for the maximum loop count */
	loop	a8, .Lz3	/* loop forever (almost anyway) */
	l32i	a8, a3, 0	/* get word from src */
	addi	a3, a3, 4	/* advance src pointer */
	bnone	a8, a4, .Lz0	/* if byte 0 is zero */
	bnone	a8, a5, .Lz1	/* if byte 1 is zero */
	bnone	a8, a6, .Lz2	/* if byte 2 is zero */
	s32i	a8, a10, 0	/* store word to dst */
	bnone	a8, a7, .Lz3	/* if byte 3 is zero */
	addi	a10, a10, 4	/* advance dst pointer */

#else /* !XCHAL_HAVE_LOOPS */

1:	addi	a10, a10, 4	/* advance dst pointer */
.Laligned:
	l32i	a8, a3, 0	/* get word from src */
	addi	a3, a3, 4	/* advance src pointer */
	bnone	a8, a4, .Lz0	/* if byte 0 is zero */
	bnone	a8, a5, .Lz1	/* if byte 1 is zero */
	bnone	a8, a6, .Lz2	/* if byte 2 is zero */
	s32i	a8, a10, 0	/* store word to dst */
	bany	a8, a7, 1b	/* if byte 3 is zero */
#endif /* !XCHAL_HAVE_LOOPS */

.Lz3:	/* Byte 3 is zero.  */
	retw

.Lz0:	/* Byte 0 is zero.  */
#ifdef __XTENSA_EB__
	movi	a8, 0
#endif
	s8i	a8, a10, 0
	retw

.Lz1:	/* Byte 1 is zero.  */
#ifdef __XTENSA_EB__
        extui   a8, a8, 16, 16
#endif
	s16i	a8, a10, 0
	retw

.Lz2:	/* Byte 2 is zero.  */
#ifdef __XTENSA_EB__
        extui   a8, a8, 16, 16
#endif
	s16i	a8, a10, 0
	movi	a8, 0
	s8i	a8, a10, 2
	retw

	.align	4
	/* (2 mod 4) alignment for loop instruction */
.Ldstunaligned:

#if XCHAL_HAVE_LOOPS
	_movi.n	a8, 0		/* set up for the maximum loop count */
	loop	a8, 2f		/* loop forever (almost anyway) */
#endif
1:	l8ui	a8, a3, 0
	addi	a3, a3, 1
	s8i	a8, a10, 0
	addi	a10, a10, 1
#if XCHAL_HAVE_LOOPS
	beqz	a8, 2f
#else
	bnez	a8, 1b
#endif
2:	retw

libc_hidden_def (strcpy)
