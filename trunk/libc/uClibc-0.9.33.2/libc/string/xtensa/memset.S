/* Optimized memset for Xtensa.
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

/* Do not use .literal_position in the ENTRY macro.  */
#undef LITERAL_POSITION
#define LITERAL_POSITION

/* void *memset (void *dst, int c, size_t length)

   The algorithm is as follows:

   Create a word with c in all byte positions.

   If the destination is aligned, set 16B chunks with a loop, and then
   finish up with 8B, 4B, 2B, and 1B stores conditional on the length.

   If the destination is unaligned, align it by conditionally
   setting 1B and/or 2B and then go to aligned case.

   This code tries to use fall-through branches for the common
   case of an aligned destination (except for the branches to
   the alignment labels).  */


/* Byte-by-byte set.  */

	.text
	.align	4
	.literal_position
__memset_aux:

	/* Skip a byte to get 1 mod 4 alignment for LOOPNEZ
	   (0 mod 4 alignment for LBEG).  */
	.byte	0

.Lbyteset:
#if XCHAL_HAVE_LOOPS
	loopnez	a4, 2f
#else
	beqz	a4, 2f
	add	a6, a5, a4	/* a6 = ending address */
#endif
1:	s8i	a3, a5, 0
	addi	a5, a5, 1
#if !XCHAL_HAVE_LOOPS
	blt	a5, a6, 1b
#endif
2:	retw


/* Destination is unaligned.  */

	.align	4

.Ldst1mod2: /* dst is only byte aligned */

	/* Do short sizes byte-by-byte.  */
	bltui	a4, 8, .Lbyteset

	/* Set 1 byte.  */
	s8i	a3, a5, 0
	addi	a5, a5, 1
	addi	a4, a4, -1

	/* Now retest if dst is aligned.  */
	_bbci.l	a5, 1, .Ldstaligned

.Ldst2mod4: /* dst has 16-bit alignment */

	/* Do short sizes byte-by-byte.  */
	bltui	a4, 8, .Lbyteset

	/* Set 2 bytes.  */
	s16i	a3, a5, 0
	addi	a5, a5, 2
	addi	a4, a4, -2

	/* dst is now aligned; return to main algorithm */
	j	.Ldstaligned


ENTRY (memset)
	/* a2 = dst, a3 = c, a4 = length */

	/* Duplicate character into all bytes of word.  */
	extui	a3, a3, 0, 8
	slli	a7, a3, 8
	or	a3, a3, a7
	slli	a7, a3, 16
	or	a3, a3, a7

	mov	a5, a2		/* copy dst so that a2 is return value */

	/* Check if dst is unaligned.  */
	_bbsi.l	a2, 0, .Ldst1mod2
	_bbsi.l	a2, 1, .Ldst2mod4
.Ldstaligned:

	/* Get number of loop iterations with 16B per iteration.  */
	srli	a7, a4, 4

	/* Destination is word-aligned.  */
#if XCHAL_HAVE_LOOPS
	loopnez	a7, 2f
#else
	beqz	a7, 2f
	slli	a6, a7, 4
	add	a6, a6, a5	/* a6 = end of last 16B chunk */
#endif
	/* Set 16 bytes per iteration.  */
1:	s32i	a3, a5, 0
	s32i	a3, a5, 4
	s32i	a3, a5, 8
	s32i	a3, a5, 12
	addi	a5, a5, 16
#if !XCHAL_HAVE_LOOPS
	blt	a5, a6, 1b
#endif

	/* Set any leftover pieces smaller than 16B.  */
2:	bbci.l	a4, 3, 3f

	/* Set 8 bytes.  */
	s32i	a3, a5, 0
	s32i	a3, a5, 4
	addi	a5, a5, 8

3:	bbci.l	a4, 2, 4f

	/* Set 4 bytes.  */
	s32i	a3, a5, 0
	addi	a5, a5, 4

4:	bbci.l	a4, 1, 5f

	/* Set 2 bytes.  */
	s16i	a3, a5, 0
	addi	a5, a5, 2

5:	bbci.l	a4, 0, 6f

	/* Set 1 byte.  */
	s8i	a3, a5, 0
6:	retw

libc_hidden_def (memset)
