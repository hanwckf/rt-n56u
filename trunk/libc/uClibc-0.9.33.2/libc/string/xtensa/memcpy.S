/* Optimized memcpy for Xtensa.
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

	.macro	src_b	r, w0, w1
#ifdef __XTENSA_EB__
	src	\r, \w0, \w1
#else
	src	\r, \w1, \w0
#endif
	.endm

	.macro	ssa8	r
#ifdef __XTENSA_EB__
	ssa8b	\r
#else
	ssa8l	\r
#endif
	.endm

/* If the Xtensa Unaligned Load Exception option is not used, this
   code can run a few cycles faster by relying on the low address bits
   being ignored.  However, if the code is then run with an Xtensa ISS
   client that checks for unaligned accesses, it will produce a lot of
   warning messages.  Set this flag to disable the use of unaligned
   accesses and keep the ISS happy.  */

#define UNALIGNED_ADDRESSES_CHECKED 1

/* Do not use .literal_position in the ENTRY macro.  */
#undef LITERAL_POSITION
#define LITERAL_POSITION


/* void *memcpy (void *dst, const void *src, size_t len)

   The algorithm is as follows:

   If the destination is unaligned, align it by conditionally
   copying 1- and/or 2-byte pieces.

   If the source is aligned, copy 16 bytes with a loop, and then finish up
   with 8, 4, 2, and 1-byte copies conditional on the length.

   Else (if source is unaligned), do the same, but use SRC to align the
   source data.

   This code tries to use fall-through branches for the common
   case of aligned source and destination and multiple of 4 (or 8) length.  */


/* Byte by byte copy.  */

	.text
	.align	4
	.literal_position
__memcpy_aux:

	/* Skip a byte to get 1 mod 4 alignment for LOOPNEZ
	   (0 mod 4 alignment for LBEG).  */
	.byte	0

.Lbytecopy:
#if XCHAL_HAVE_LOOPS
	loopnez	a4, 2f
#else
	beqz	a4, 2f
	add	a7, a3, a4	/* a7 = end address for source */
#endif
1:	l8ui	a6, a3, 0
	addi	a3, a3, 1
	s8i	a6, a5, 0
	addi	a5, a5, 1
#if !XCHAL_HAVE_LOOPS
	blt	a3, a7, 1b
#endif
2:	retw


/* Destination is unaligned.  */

	.align	4
.Ldst1mod2: /* dst is only byte aligned */

	/* Do short copies byte-by-byte.  */
	_bltui	a4, 7, .Lbytecopy

	/* Copy 1 byte.  */
	l8ui	a6, a3, 0
	addi	a3, a3, 1
	addi	a4, a4, -1
	s8i	a6, a5, 0
	addi	a5, a5, 1

	/* Return to main algorithm if dst is now aligned.  */
	_bbci.l	a5, 1, .Ldstaligned

.Ldst2mod4: /* dst has 16-bit alignment */

	/* Do short copies byte-by-byte.  */
	_bltui	a4, 6, .Lbytecopy

	/* Copy 2 bytes.  */
	l8ui	a6, a3, 0
	l8ui	a7, a3, 1
	addi	a3, a3, 2
	addi	a4, a4, -2
	s8i	a6, a5, 0
	s8i	a7, a5, 1
	addi	a5, a5, 2

	/* dst is now aligned; return to main algorithm.  */
	j	.Ldstaligned


ENTRY (memcpy)
	/* a2 = dst, a3 = src, a4 = len */

	mov	a5, a2		/* copy dst so that a2 is return value */
	_bbsi.l	a2, 0, .Ldst1mod2
	_bbsi.l	a2, 1, .Ldst2mod4
.Ldstaligned:

	/* Get number of loop iterations with 16B per iteration.  */
	srli	a7, a4, 4

	/* Check if source is aligned.  */
	movi	a8, 3
	_bany	a3, a8, .Lsrcunaligned

	/* Destination and source are word-aligned, use word copy.  */
#if XCHAL_HAVE_LOOPS
	loopnez	a7, 2f
#else
	beqz	a7, 2f
	slli	a8, a7, 4
	add	a8, a8, a3	/* a8 = end of last 16B source chunk */
#endif
1:	l32i	a6, a3, 0
	l32i	a7, a3, 4
	s32i	a6, a5, 0
	l32i	a6, a3, 8
	s32i	a7, a5, 4
	l32i	a7, a3, 12
	s32i	a6, a5, 8
	addi	a3, a3, 16
	s32i	a7, a5, 12
	addi	a5, a5, 16
#if !XCHAL_HAVE_LOOPS
	blt	a3, a8, 1b
#endif

	/* Copy any leftover pieces smaller than 16B.  */
2:	bbci.l	a4, 3, 3f

	/* Copy 8 bytes.  */
	l32i	a6, a3, 0
	l32i	a7, a3, 4
	addi	a3, a3, 8
	s32i	a6, a5, 0
	s32i	a7, a5, 4
	addi	a5, a5, 8

3:	bbsi.l	a4, 2, 4f
	bbsi.l	a4, 1, 5f
	bbsi.l	a4, 0, 6f
	retw

	/* Copy 4 bytes.  */
4:	l32i	a6, a3, 0
	addi	a3, a3, 4
	s32i	a6, a5, 0
	addi	a5, a5, 4
	bbsi.l	a4, 1, 5f
	bbsi.l	a4, 0, 6f
	retw

	/* Copy 2 bytes.  */
5:	l16ui	a6, a3, 0
	addi	a3, a3, 2
	s16i	a6, a5, 0
	addi	a5, a5, 2
	bbsi.l	a4, 0, 6f
	retw

	/* Copy 1 byte.  */
6:	l8ui	a6, a3, 0
	s8i	a6, a5, 0

.Ldone:
	retw


/* Destination is aligned; source is unaligned.  */

	.align	4
.Lsrcunaligned:
	/* Avoid loading anything for zero-length copies.  */
	_beqz	a4, .Ldone

	/* Copy 16 bytes per iteration for word-aligned dst and
	   unaligned src.  */
	ssa8	a3		/* set shift amount from byte offset */
#if UNALIGNED_ADDRESSES_CHECKED
	and	a11, a3, a8	/* save unalignment offset for below */
	sub	a3, a3, a11	/* align a3 */
#endif
	l32i	a6, a3, 0	/* load first word */
#if XCHAL_HAVE_LOOPS
	loopnez	a7, 2f
#else
	beqz	a7, 2f
	slli	a10, a7, 4
	add	a10, a10, a3	/* a10 = end of last 16B source chunk */
#endif
1:	l32i	a7, a3, 4
	l32i	a8, a3, 8
	src_b	a6, a6, a7
	s32i	a6, a5, 0
	l32i	a9, a3, 12
	src_b	a7, a7, a8
	s32i	a7, a5, 4
	l32i	a6, a3, 16
	src_b	a8, a8, a9
	s32i	a8, a5, 8
	addi	a3, a3, 16
	src_b	a9, a9, a6
	s32i	a9, a5, 12
	addi	a5, a5, 16
#if !XCHAL_HAVE_LOOPS
	blt	a3, a10, 1b
#endif

2:	bbci.l	a4, 3, 3f

	/* Copy 8 bytes.  */
	l32i	a7, a3, 4
	l32i	a8, a3, 8
	src_b	a6, a6, a7
	s32i	a6, a5, 0
	addi	a3, a3, 8
	src_b	a7, a7, a8
	s32i	a7, a5, 4
	addi	a5, a5, 8
	mov	a6, a8

3:	bbci.l	a4, 2, 4f

	/* Copy 4 bytes.  */
	l32i	a7, a3, 4
	addi	a3, a3, 4
	src_b	a6, a6, a7
	s32i	a6, a5, 0
	addi	a5, a5, 4
	mov	a6, a7
4:
#if UNALIGNED_ADDRESSES_CHECKED
	add	a3, a3, a11	/* readjust a3 with correct misalignment */
#endif
	bbsi.l	a4, 1, 5f
	bbsi.l	a4, 0, 6f
	retw

	/* Copy 2 bytes.  */
5:	l8ui	a6, a3, 0
	l8ui	a7, a3, 1
	addi	a3, a3, 2
	s8i	a6, a5, 0
	s8i	a7, a5, 1
	addi	a5, a5, 2
	bbsi.l	a4, 0, 6f
	retw

	/* Copy 1 byte.  */
6:	l8ui	a6, a3, 0
	s8i	a6, a5, 0
	retw

libc_hidden_def (memcpy)
