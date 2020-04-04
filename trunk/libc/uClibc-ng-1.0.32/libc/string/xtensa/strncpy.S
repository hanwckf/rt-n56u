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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>
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

/* Do not use .literal_position in the ENTRY macro.  */
#undef LITERAL_POSITION
#define LITERAL_POSITION

	.text
	.align	4
	.literal_position
__strncpy_aux:

.Lsrc1mod2: /* src address is odd */
	l8ui	a8, a3, 0	/* get byte 0 */
	addi	a3, a3, 1	/* advance src pointer */
	s8i	a8, a10, 0	/* store byte 0 */
	addi	a4, a4, -1	/* decrement n */
	beqz    a4, .Lret       /* if n is zero */
	addi	a10, a10, 1	/* advance dst pointer */
	beqz	a8, .Lfill	/* if byte 0 is zero */
	bbci.l	a3, 1, .Lsrcaligned /* if src is now word-aligned */

.Lsrc2mod4: /* src address is 2 mod 4 */
	l8ui	a8, a3, 0	/* get byte 0 */
	addi	a4, a4, -1	/* decrement n */
	s8i	a8, a10, 0	/* store byte 0 */
	beqz    a4, .Lret       /* if n is zero */
	addi	a10, a10, 1	/* advance dst pointer */
	beqz	a8, .Lfill	/* if byte 0 is zero */
	l8ui	a8, a3, 1	/* get byte 0 */
	addi	a3, a3, 2	/* advance src pointer */
	s8i	a8, a10, 0	/* store byte 0 */
	addi	a4, a4, -1	/* decrement n */
	beqz    a4, .Lret       /* if n is zero */
	addi	a10, a10, 1	/* advance dst pointer */
	bnez	a8, .Lsrcaligned
	j	.Lfill

.Lret:
	abi_ret


ENTRY (strncpy)
	/* a2 = dst, a3 = src */

	mov	a10, a2		/* leave dst in return value register */
	beqz    a4, .Lret       /* if n is zero */

	movi	a11, MASK0
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


/* Fill the dst with zeros -- n is at least 1.  */

.Lfill:
	movi	a9, 0
	bbsi.l	a10, 0, .Lfill1mod2
	bbsi.l	a10, 1, .Lfill2mod4
.Lfillaligned:
	blti	a4, 4, .Lfillcleanup

	/* Loop filling complete words with zero.  */
#if XCHAL_HAVE_LOOPS

	srai	a8, a4, 2
	loop	a8, 1f
	s32i	a9, a10, 0
	addi	a10, a10, 4

1:	slli	a8, a8, 2
	sub	a4, a4, a8

#else /* !XCHAL_HAVE_LOOPS */

1:	s32i	a9, a10, 0
	addi	a10, a10, 4
	addi	a4, a4, -4
	bgei    a4, 4, 1b

#endif /* !XCHAL_HAVE_LOOPS */

	beqz	a4, 2f

.Lfillcleanup:
	/* Fill leftover (1 to 3) bytes with zero.  */
	s8i	a9, a10, 0	/* store byte 0 */
	addi	a4, a4, -1	/* decrement n */
	addi	a10, a10, 1
	bnez    a4, .Lfillcleanup

2:	abi_ret

.Lfill1mod2: /* dst address is odd */
	s8i	a9, a10, 0	/* store byte 0 */
	addi	a4, a4, -1	/* decrement n */
	beqz    a4, 2b		/* if n is zero */
	addi    a10, a10, 1	/* advance dst pointer */
	bbci.l	a10, 1, .Lfillaligned /* if dst is now word-aligned */

.Lfill2mod4: /* dst address is 2 mod 4 */
	s8i	a9, a10, 0	/* store byte 0 */
	addi	a4, a4, -1	/* decrement n */
	beqz    a4, 2b		/* if n is zero */
	s8i	a9, a10, 1	/* store byte 1 */
	addi	a4, a4, -1	/* decrement n */
	beqz    a4, 2b		/* if n is zero */
	addi    a10, a10, 2	/* advance dst pointer */
	j	.Lfillaligned


/* dst is word-aligned; src is word-aligned; n is at least 1.  */

	.align	4
	/* (2 mod 4) alignment for loop instruction */
.Laligned:
#if XCHAL_HAVE_LOOPS
	_movi.n	a8, 0		/* set up for the maximum loop count */
	loop	a8, 1f		/* loop forever (almost anyway) */
	blti	a4, 5, .Ldstunaligned /* n is near limit; do one at a time */
	l32i	a8, a3, 0	/* get word from src */
	addi	a3, a3, 4	/* advance src pointer */
	bnone	a8, a11, .Lz0	/* if byte 0 is zero */
	bnone	a8, a5, .Lz1	/* if byte 1 is zero */
	bnone	a8, a6, .Lz2	/* if byte 2 is zero */
	s32i	a8, a10, 0	/* store word to dst */
	addi	a4, a4, -4	/* decrement n */
	addi	a10, a10, 4	/* advance dst pointer */
	bnone	a8, a7, .Lfill	/* if byte 3 is zero */
1:

#else /* !XCHAL_HAVE_LOOPS */

1:	blti	a4, 5, .Ldstunaligned /* n is near limit; do one at a time */
	l32i	a8, a3, 0	/* get word from src */
	addi	a3, a3, 4	/* advance src pointer */
	bnone	a8, a11, .Lz0	/* if byte 0 is zero */
	bnone	a8, a5, .Lz1	/* if byte 1 is zero */
	bnone	a8, a6, .Lz2	/* if byte 2 is zero */
	s32i	a8, a10, 0	/* store word to dst */
	addi	a4, a4, -4	/* decrement n */
	addi	a10, a10, 4	/* advance dst pointer */
	bany	a8, a7, 1b	/* no zeroes */
#endif /* !XCHAL_HAVE_LOOPS */

	j	.Lfill

.Lz0:	/* Byte 0 is zero.  */
#ifdef __XTENSA_EB__
	movi	a8, 0
#endif
	s8i	a8, a10, 0
	addi	a4, a4, -1	/* decrement n */
	addi	a10, a10, 1	/* advance dst pointer */
	j	.Lfill

.Lz1:	/* Byte 1 is zero.  */
#ifdef __XTENSA_EB__
        extui   a8, a8, 16, 16
#endif
	s16i	a8, a10, 0
	addi	a4, a4, -2	/* decrement n */
	addi	a10, a10, 2	/* advance dst pointer */
	j	.Lfill

.Lz2:	/* Byte 2 is zero.  */
#ifdef __XTENSA_EB__
        extui   a8, a8, 16, 16
#endif
	s16i	a8, a10, 0
	movi	a8, 0
	s8i	a8, a10, 2
	addi	a4, a4, -3	/* decrement n */
	addi	a10, a10, 3	/* advance dst pointer */
	j	.Lfill

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
	addi	a4, a4, -1
	beqz	a4, 3f
	addi	a10, a10, 1
#if XCHAL_HAVE_LOOPS
	beqz	a8, 2f
#else
	bnez	a8, 1b
#endif
2:	j	.Lfill

3:	abi_ret

libc_hidden_def (strncpy)
