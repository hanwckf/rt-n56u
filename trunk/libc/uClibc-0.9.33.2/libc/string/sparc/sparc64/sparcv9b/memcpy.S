/* Copy SIZE bytes from SRC to DEST.
   For UltraSPARC-III.
   Copyright (C) 2001, 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by David S. Miller (davem@redhat.com)

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

#include <features.h>

#define ASI_BLK_P 0xf0
#define FPRS_FEF  0x04
#define VISEntryHalf rd %fprs, %o5; wr %g0, FPRS_FEF, %fprs
#define VISExitHalf and %o5, FPRS_FEF, %o5; wr %o5, 0x0, %fprs

#ifndef XCC
#define USE_BPR
#define XCC xcc
#endif

	.register	%g2,#scratch
	.register	%g3,#scratch
	.register	%g6,#scratch

	.text
	.align	32

#ifdef __UCLIBC_SUSV3_LEGACY__
ENTRY(bcopy)
	sub		%o1, %o0, %o4
	mov		%o0, %g4
	cmp		%o4, %o2
	mov		%o1, %o0
	bgeu,pt		%XCC, 100f
	 mov		%g4, %o1
#ifndef USE_BPR
	srl		%o2, 0, %o2
#endif
	brnz,pn		%o2, 220f
	 add		%o0, %o2, %o0
	retl
	 nop
END(bcopy)
#endif

	/* Special/non-trivial issues of this code:
	 *
	 * 1) %o5 is preserved from VISEntryHalf to VISExitHalf
	 * 2) Only low 32 FPU registers are used so that only the
	 *    lower half of the FPU register set is dirtied by this
	 *    code.  This is especially important in the kernel.
	 * 3) This code never prefetches cachelines past the end
	 *    of the source buffer.
	 *
	 * The cheetah's flexible spine, oversized liver, enlarged heart,
	 * slender muscular body, and claws make it the swiftest hunter
	 * in Africa and the fastest animal on land.  Can reach speeds
	 * of up to 2.4GB per second.
	 */
	.align		32
ENTRY(memcpy)

100: /* %o0=dst, %o1=src, %o2=len */
	mov		%o0, %g5
	cmp		%o2, 0
	be,pn		%XCC, out
218:	 or		%o0, %o1, %o3
	cmp		%o2, 16
	bleu,a,pn	%XCC, small_copy
	 or		%o3, %o2, %o3

	cmp		%o2, 256
	blu,pt		%XCC, medium_copy
	 andcc		%o3, 0x7, %g0

	ba,pt		%xcc, enter
	 andcc		%o0, 0x3f, %g2

	/* Here len >= 256 and condition codes reflect execution
	 * of "andcc %o0, 0x7, %g2", done by caller.
	 */
	.align		64
enter:
	/* Is 'dst' already aligned on an 64-byte boundary? */
	be,pt		%XCC, 2f

	/* Compute abs((dst & 0x3f) - 0x40) into %g2.  This is the number
	 * of bytes to copy to make 'dst' 64-byte aligned.  We pre-
	 * subtract this from 'len'.
	 */
	 sub		%g2, 0x40, %g2
	sub		%g0, %g2, %g2
	sub		%o2, %g2, %o2

	/* Copy %g2 bytes from src to dst, one byte at a time. */
1:	ldub		[%o1 + 0x00], %o3
	add		%o1, 0x1, %o1
	add		%o0, 0x1, %o0
	subcc		%g2, 0x1, %g2

	bg,pt		%XCC, 1b
	 stb		%o3, [%o0 + -1]

2:	VISEntryHalf
	and		%o1, 0x7, %g1
	ba,pt		%xcc, begin
	 alignaddr	%o1, %g0, %o1

	.align		64
begin:
	prefetch	[%o1 + 0x000], #one_read
	prefetch	[%o1 + 0x040], #one_read
	andn		%o2, (0x40 - 1), %o4
	prefetch	[%o1 + 0x080], #one_read
	prefetch	[%o1 + 0x0c0], #one_read
	ldd		[%o1 + 0x000], %f0
	prefetch	[%o1 + 0x100], #one_read
	ldd		[%o1 + 0x008], %f2
	prefetch	[%o1 + 0x140], #one_read
	ldd		[%o1 + 0x010], %f4
	prefetch	[%o1 + 0x180], #one_read
	faligndata	%f0, %f2, %f16
	ldd		[%o1 + 0x018], %f6
	faligndata	%f2, %f4, %f18
	ldd		[%o1 + 0x020], %f8
	faligndata	%f4, %f6, %f20
	ldd		[%o1 + 0x028], %f10
	faligndata	%f6, %f8, %f22

	ldd		[%o1 + 0x030], %f12
	faligndata	%f8, %f10, %f24
	ldd		[%o1 + 0x038], %f14
	faligndata	%f10, %f12, %f26
	ldd		[%o1 + 0x040], %f0

	sub		%o4, 0x80, %o4
	add		%o1, 0x40, %o1
	ba,pt		%xcc, loop
	 srl		%o4, 6, %o3

	.align		64
loop:
	ldd		[%o1 + 0x008], %f2
	faligndata	%f12, %f14, %f28
	ldd		[%o1 + 0x010], %f4
	faligndata	%f14, %f0, %f30
	stda		%f16, [%o0] ASI_BLK_P
	ldd		[%o1 + 0x018], %f6
	faligndata	%f0, %f2, %f16

	ldd		[%o1 + 0x020], %f8
	faligndata	%f2, %f4, %f18
	ldd		[%o1 + 0x028], %f10
	faligndata	%f4, %f6, %f20
	ldd		[%o1 + 0x030], %f12
	faligndata	%f6, %f8, %f22
	ldd		[%o1 + 0x038], %f14
	faligndata	%f8, %f10, %f24

	ldd		[%o1 + 0x040], %f0
	prefetch	[%o1 + 0x180], #one_read
	faligndata	%f10, %f12, %f26
	subcc		%o3, 0x01, %o3
	add		%o1, 0x40, %o1
	bg,pt		%XCC, loop
	 add		%o0, 0x40, %o0

	/* Finally we copy the last full 64-byte block. */
loopfini:
	ldd		[%o1 + 0x008], %f2
	faligndata	%f12, %f14, %f28
	ldd		[%o1 + 0x010], %f4
	faligndata	%f14, %f0, %f30
	stda		%f16, [%o0] ASI_BLK_P
	ldd		[%o1 + 0x018], %f6
	faligndata	%f0, %f2, %f16
	ldd		[%o1 + 0x020], %f8
	faligndata	%f2, %f4, %f18
	ldd		[%o1 + 0x028], %f10
	faligndata	%f4, %f6, %f20
	ldd		[%o1 + 0x030], %f12
	faligndata	%f6, %f8, %f22
	ldd		[%o1 + 0x038], %f14
	faligndata	%f8, %f10, %f24
	cmp		%g1, 0
	be,pt		%XCC, 1f
	 add		%o0, 0x40, %o0
	ldd		[%o1 + 0x040], %f0
1:	faligndata	%f10, %f12, %f26
	faligndata	%f12, %f14, %f28
	faligndata	%f14, %f0, %f30
	stda		%f16, [%o0] ASI_BLK_P
	add		%o0, 0x40, %o0
	add		%o1, 0x40, %o1
	membar		#Sync

	/* Now we copy the (len modulo 64) bytes at the end.
	 * Note how we borrow the %f0 loaded above.
	 *
	 * Also notice how this code is careful not to perform a
	 * load past the end of the src buffer.
	 */
loopend:
	and		%o2, 0x3f, %o2
	andcc		%o2, 0x38, %g2
	be,pn		%XCC, endcruft
	 subcc		%g2, 0x8, %g2
	be,pn		%XCC, endcruft
	 cmp		%g1, 0

	be,a,pt		%XCC, 1f
	 ldd		[%o1 + 0x00], %f0

1:	ldd		[%o1 + 0x08], %f2
	add		%o1, 0x8, %o1
	sub		%o2, 0x8, %o2
	subcc		%g2, 0x8, %g2
	faligndata	%f0, %f2, %f8
	std		%f8, [%o0 + 0x00]
	be,pn		%XCC, endcruft
	 add		%o0, 0x8, %o0
	ldd		[%o1 + 0x08], %f0
	add		%o1, 0x8, %o1
	sub		%o2, 0x8, %o2
	subcc		%g2, 0x8, %g2
	faligndata	%f2, %f0, %f8
	std		%f8, [%o0 + 0x00]
	bne,pn		%XCC, 1b
	 add		%o0, 0x8, %o0

	/* If anything is left, we copy it one byte at a time.
	 * Note that %g1 is (src & 0x3) saved above before the
	 * alignaddr was performed.
	 */
endcruft:
	cmp		%o2, 0
	add		%o1, %g1, %o1
	VISExitHalf
	be,pn		%XCC, out
	 sub		%o0, %o1, %o3

	andcc		%g1, 0x7, %g0
	bne,pn		%icc, small_copy_unaligned
	 andcc		%o2, 0x8, %g0
	be,pt		%icc, 1f
	 nop
	ldx		[%o1], %o5
	stx		%o5, [%o1 + %o3]
	add		%o1, 0x8, %o1

1:	andcc		%o2, 0x4, %g0
	be,pt		%icc, 1f
	 nop
	lduw		[%o1], %o5
	stw		%o5, [%o1 + %o3]
	add		%o1, 0x4, %o1

1:	andcc		%o2, 0x2, %g0
	be,pt		%icc, 1f
	 nop
	lduh		[%o1], %o5
	sth		%o5, [%o1 + %o3]
	add		%o1, 0x2, %o1

1:	andcc		%o2, 0x1, %g0
	be,pt		%icc, out
	 nop
	ldub		[%o1], %o5
	ba,pt		%xcc, out
	 stb		%o5, [%o1 + %o3]

medium_copy: /* 16 < len <= 64 */
	bne,pn		%XCC, small_copy_unaligned
	 sub		%o0, %o1, %o3

medium_copy_aligned:
	andn		%o2, 0x7, %o4
	and		%o2, 0x7, %o2
1:	subcc		%o4, 0x8, %o4
	ldx		[%o1], %o5
	stx		%o5, [%o1 + %o3]
	bgu,pt		%XCC, 1b
	 add		%o1, 0x8, %o1
	andcc		%o2, 0x4, %g0
	be,pt		%XCC, 1f
	 nop
	sub		%o2, 0x4, %o2
	lduw		[%o1], %o5
	stw		%o5, [%o1 + %o3]
	add		%o1, 0x4, %o1
1:	cmp		%o2, 0
	be,pt		%XCC, out
	 nop
	ba,pt		%xcc, small_copy_unaligned
	 nop

small_copy: /* 0 < len <= 16 */
	andcc		%o3, 0x3, %g0
	bne,pn		%XCC, small_copy_unaligned
	 sub		%o0, %o1, %o3

small_copy_aligned:
	subcc		%o2, 4, %o2
	lduw		[%o1], %g1
	stw		%g1, [%o1 + %o3]
	bgu,pt		%XCC, small_copy_aligned
	 add		%o1, 4, %o1

out:	retl
	 mov		%g5, %o0

	.align	32
small_copy_unaligned:
	subcc		%o2, 1, %o2
	ldub		[%o1], %g1
	stb		%g1, [%o1 + %o3]
	bgu,pt		%XCC, small_copy_unaligned
	 add		%o1, 1, %o1
	retl
	 mov		%g5, %o0

END(memcpy)
libc_hidden_def(memcpy)

#define RMOVE_BIGCHUNK(src, dst, offset, t0, t1, t2, t3)	\
	ldx		[%src - offset - 0x20], %t0; 		\
	ldx		[%src - offset - 0x18], %t1; 		\
	ldx		[%src - offset - 0x10], %t2; 		\
	ldx		[%src - offset - 0x08], %t3; 		\
	stw		%t0, [%dst - offset - 0x1c]; 		\
	srlx		%t0, 32, %t0;				\
	stw		%t0, [%dst - offset - 0x20]; 		\
	stw		%t1, [%dst - offset - 0x14]; 		\
	srlx		%t1, 32, %t1;				\
	stw		%t1, [%dst - offset - 0x18]; 		\
	stw		%t2, [%dst - offset - 0x0c]; 		\
	srlx		%t2, 32, %t2;				\
	stw		%t2, [%dst - offset - 0x10]; 		\
	stw		%t3, [%dst - offset - 0x04];		\
	srlx		%t3, 32, %t3;				\
	stw		%t3, [%dst - offset - 0x08];

#define RMOVE_BIGALIGNCHUNK(src, dst, offset, t0, t1, t2, t3)	\
	ldx		[%src - offset - 0x20], %t0; 		\
	ldx		[%src - offset - 0x18], %t1; 		\
	ldx		[%src - offset - 0x10], %t2; 		\
	ldx		[%src - offset - 0x08], %t3; 		\
	stx		%t0, [%dst - offset - 0x20]; 		\
	stx		%t1, [%dst - offset - 0x18]; 		\
	stx		%t2, [%dst - offset - 0x10]; 		\
	stx		%t3, [%dst - offset - 0x08];		\
	ldx		[%src - offset - 0x40], %t0; 		\
	ldx		[%src - offset - 0x38], %t1; 		\
	ldx		[%src - offset - 0x30], %t2; 		\
	ldx		[%src - offset - 0x28], %t3; 		\
	stx		%t0, [%dst - offset - 0x40]; 		\
	stx		%t1, [%dst - offset - 0x38]; 		\
	stx		%t2, [%dst - offset - 0x30]; 		\
	stx		%t3, [%dst - offset - 0x28];

#define RMOVE_LASTCHUNK(src, dst, offset, t0, t1, t2, t3)	\
	ldx		[%src + offset + 0x00], %t0;		\
	ldx		[%src + offset + 0x08], %t1; 		\
	stw		%t0, [%dst + offset + 0x04]; 		\
	srlx		%t0, 32, %t2;				\
	stw		%t2, [%dst + offset + 0x00]; 		\
	stw		%t1, [%dst + offset + 0x0c]; 		\
	srlx		%t1, 32, %t3;				\
	stw		%t3, [%dst + offset + 0x08];

#define RMOVE_LASTALIGNCHUNK(src, dst, offset, t0, t1)		\
	ldx		[%src + offset + 0x00], %t0; 		\
	ldx		[%src + offset + 0x08], %t1; 		\
	stx		%t0, [%dst + offset + 0x00]; 		\
	stx		%t1, [%dst + offset + 0x08];

	.align		32
228:	andcc		%o2, 1, %g0			/* IEU1		Group		*/
	be,pt		%icc, 2f+4			/* CTI				*/
1:	 ldub		[%o1 - 1], %o5			/* LOAD		Group		*/
	sub		%o1, 1, %o1			/* IEU0				*/
	sub		%o0, 1, %o0			/* IEU1				*/
	subcc		%o2, 1, %o2			/* IEU1		Group		*/
	be,pn		%xcc, 229f			/* CTI				*/
	 stb		%o5, [%o0]			/* Store			*/
2:	ldub		[%o1 - 1], %o5			/* LOAD		Group		*/
	sub		%o0, 2, %o0			/* IEU0				*/
	ldub		[%o1 - 2], %g5			/* LOAD		Group		*/
	sub		%o1, 2, %o1			/* IEU0				*/
	subcc		%o2, 2, %o2			/* IEU1		Group		*/
	stb		%o5, [%o0 + 1]			/* Store			*/
	bne,pt		%xcc, 2b			/* CTI				*/
	 stb		%g5, [%o0]			/* Store			*/
229:	retl
	 mov		%g4, %o0

	.align		32
ENTRY(memmove)
	mov		%o0, %g5
#ifndef USE_BPR
	srl		%o2, 0, %o2			/* IEU1		Group		*/
#endif
	brz,pn		%o2, out			/* CTI		Group		*/
	 sub		%o0, %o1, %o4			/* IEU0				*/
	cmp		%o4, %o2			/* IEU1		Group		*/
	bgeu,pt		%XCC, 218b			/* CTI				*/
	 mov		%o0, %g4			/* IEU0				*/
	add		%o0, %o2, %o0			/* IEU0		Group		*/
220:	add		%o1, %o2, %o1			/* IEU1				*/
	cmp		%o2, 15				/* IEU1		Group		*/
	bleu,pn		%xcc, 228b			/* CTI				*/
	 andcc		%o0, 7, %g2			/* IEU1		Group		*/
	sub		%o0, %o1, %g5			/* IEU0				*/
	andcc		%g5, 3, %o5			/* IEU1		Group		*/
	bne,pn		%xcc, 232f			/* CTI				*/
	 andcc		%o1, 3, %g0			/* IEU1		Group		*/
	be,a,pt		%xcc, 236f			/* CTI				*/
	 andcc		%o1, 4, %g0			/* IEU1		Group		*/
	andcc		%o1, 1, %g0			/* IEU1		Group		*/
	be,pn		%xcc, 4f			/* CTI				*/
	 andcc		%o1, 2, %g0			/* IEU1		Group		*/
	ldub		[%o1 - 1], %g2			/* Load		Group		*/
	sub		%o1, 1, %o1			/* IEU0				*/
	sub		%o0, 1, %o0			/* IEU1				*/
	sub		%o2, 1, %o2			/* IEU0		Group		*/
	be,pn		%xcc, 5f			/* CTI		Group		*/
	 stb		%g2, [%o0]			/* Store			*/
4:	lduh		[%o1 - 2], %g2			/* Load		Group		*/
	sub		%o1, 2, %o1			/* IEU0				*/
	sub		%o0, 2, %o0			/* IEU1				*/
	sub		%o2, 2, %o2			/* IEU0				*/
	sth		%g2, [%o0]			/* Store	Group + bubble	*/
5:	andcc		%o1, 4, %g0			/* IEU1				*/
236:	be,a,pn		%xcc, 2f			/* CTI				*/
	 andcc		%o2, -128, %g6			/* IEU1		Group		*/
	lduw		[%o1 - 4], %g5			/* Load		Group		*/
	sub		%o1, 4, %o1			/* IEU0				*/
	sub		%o0, 4, %o0			/* IEU1				*/
	sub		%o2, 4, %o2			/* IEU0		Group		*/
	stw		%g5, [%o0]			/* Store			*/
	andcc		%o2, -128, %g6			/* IEU1		Group		*/
2:	be,pn		%xcc, 235f			/* CTI				*/
	 andcc		%o0, 4, %g0			/* IEU1		Group		*/
	be,pn		%xcc, 282f + 4			/* CTI		Group		*/
5:	RMOVE_BIGCHUNK(o1, o0, 0x00, g1, g3, g5, o5)
	RMOVE_BIGCHUNK(o1, o0, 0x20, g1, g3, g5, o5)
	RMOVE_BIGCHUNK(o1, o0, 0x40, g1, g3, g5, o5)
	RMOVE_BIGCHUNK(o1, o0, 0x60, g1, g3, g5, o5)
	subcc		%g6, 128, %g6			/* IEU1		Group		*/
	sub		%o1, 128, %o1			/* IEU0				*/
	bne,pt		%xcc, 5b			/* CTI				*/
	 sub		%o0, 128, %o0			/* IEU0		Group		*/
235:	andcc		%o2, 0x70, %g6			/* IEU1		Group		*/
41:	be,pn		%xcc, 280f			/* CTI				*/
	 andcc		%o2, 8, %g0			/* IEU1		Group		*/
							/* Clk1 8-(			*/
							/* Clk2 8-(			*/
							/* Clk3 8-(			*/
							/* Clk4 8-(			*/
279:	rd		%pc, %o5			/* PDU		Group		*/
	sll		%g6, 1, %g5			/* IEU0		Group		*/
	sub		%o1, %g6, %o1			/* IEU1				*/
	sub		%o5, %g5, %o5			/* IEU0  	Group		*/
	jmpl		%o5 + %lo(280f - 279b), %g0	/* CTI		Group brk forced*/
	 sub		%o0, %g6, %o0			/* IEU0		Group		*/
	RMOVE_LASTCHUNK(o1, o0, 0x60, g2, g3, g5, o5)
	RMOVE_LASTCHUNK(o1, o0, 0x50, g2, g3, g5, o5)
	RMOVE_LASTCHUNK(o1, o0, 0x40, g2, g3, g5, o5)
	RMOVE_LASTCHUNK(o1, o0, 0x30, g2, g3, g5, o5)
	RMOVE_LASTCHUNK(o1, o0, 0x20, g2, g3, g5, o5)
	RMOVE_LASTCHUNK(o1, o0, 0x10, g2, g3, g5, o5)
	RMOVE_LASTCHUNK(o1, o0, 0x00, g2, g3, g5, o5)
280:	be,pt		%xcc, 281f			/* CTI				*/
	 andcc		%o2, 4, %g0			/* IEU1				*/
	ldx		[%o1 - 8], %g2			/* Load		Group		*/
	sub		%o0, 8, %o0			/* IEU0				*/
	stw		%g2, [%o0 + 4]			/* Store	Group		*/
	sub		%o1, 8, %o1			/* IEU1				*/
	srlx		%g2, 32, %g2			/* IEU0		Group		*/
	stw		%g2, [%o0]			/* Store			*/
281:	be,pt		%xcc, 1f			/* CTI				*/
	 andcc		%o2, 2, %g0			/* IEU1		Group		*/
	lduw		[%o1 - 4], %g2			/* Load		Group		*/
	sub		%o1, 4, %o1			/* IEU0				*/
	stw		%g2, [%o0 - 4]			/* Store	Group		*/
	sub		%o0, 4, %o0			/* IEU0				*/
1:	be,pt		%xcc, 1f			/* CTI				*/
	 andcc		%o2, 1, %g0			/* IEU1		Group		*/
	lduh		[%o1 - 2], %g2			/* Load		Group		*/
	sub		%o1, 2, %o1			/* IEU0				*/
	sth		%g2, [%o0 - 2]			/* Store	Group		*/
	sub		%o0, 2, %o0			/* IEU0				*/
1:	be,pt		%xcc, 211f			/* CTI				*/
	 nop						/* IEU1				*/
	ldub		[%o1 - 1], %g2			/* Load		Group		*/
	stb		%g2, [%o0 - 1]			/* Store	Group + bubble	*/
211:	retl
	 mov		%g4, %o0

282:	RMOVE_BIGALIGNCHUNK(o1, o0, 0x00, g1, g3, g5, o5)
	RMOVE_BIGALIGNCHUNK(o1, o0, 0x40, g1, g3, g5, o5)
	subcc		%g6, 128, %g6			/* IEU1		Group		*/
	sub		%o1, 128, %o1			/* IEU0				*/
	bne,pt		%xcc, 282b			/* CTI				*/
	 sub		%o0, 128, %o0			/* IEU0		Group		*/
	andcc		%o2, 0x70, %g6			/* IEU1				*/
	be,pn		%xcc, 284f			/* CTI				*/
	 andcc		%o2, 8, %g0			/* IEU1		Group		*/
							/* Clk1 8-(			*/
							/* Clk2 8-(			*/
							/* Clk3 8-(			*/
							/* Clk4 8-(			*/
283:	rd		%pc, %o5			/* PDU		Group		*/
	sub		%o1, %g6, %o1			/* IEU0		Group		*/
	sub		%o5, %g6, %o5			/* IEU1				*/
	jmpl		%o5 + %lo(284f - 283b), %g0	/* CTI		Group brk forced*/
	 sub		%o0, %g6, %o0			/* IEU0		Group		*/
	RMOVE_LASTALIGNCHUNK(o1, o0, 0x60, g2, g3)
	RMOVE_LASTALIGNCHUNK(o1, o0, 0x50, g2, g3)
	RMOVE_LASTALIGNCHUNK(o1, o0, 0x40, g2, g3)
	RMOVE_LASTALIGNCHUNK(o1, o0, 0x30, g2, g3)
	RMOVE_LASTALIGNCHUNK(o1, o0, 0x20, g2, g3)
	RMOVE_LASTALIGNCHUNK(o1, o0, 0x10, g2, g3)
	RMOVE_LASTALIGNCHUNK(o1, o0, 0x00, g2, g3)
284:	be,pt		%xcc, 285f			/* CTI		Group		*/
	 andcc		%o2, 4, %g0			/* IEU1				*/
	ldx		[%o1 - 8], %g2			/* Load		Group		*/
	sub		%o0, 8, %o0			/* IEU0				*/
	sub		%o1, 8, %o1			/* IEU0		Group		*/
	stx		%g2, [%o0]			/* Store			*/
285:	be,pt		%xcc, 1f			/* CTI				*/
	 andcc		%o2, 2, %g0			/* IEU1		Group		*/
	lduw		[%o1 - 4], %g2			/* Load		Group		*/
	sub		%o0, 4, %o0			/* IEU0				*/
	sub		%o1, 4, %o1			/* IEU0		Group		*/
	stw		%g2, [%o0]			/* Store			*/
1:	be,pt		%xcc, 1f			/* CTI				*/
	 andcc		%o2, 1, %g0			/* IEU1		Group		*/
	lduh		[%o1 - 2], %g2			/* Load		Group		*/
	sub		%o0, 2, %o0			/* IEU0				*/
	sub		%o1, 2, %o1			/* IEU0		Group		*/
	sth		%g2, [%o0]			/* Store			*/
1:	be,pt		%xcc, 1f			/* CTI				*/
	 nop						/* IEU0		Group		*/
	ldub		[%o1 - 1], %g2			/* Load		Group		*/
	stb		%g2, [%o0 - 1]			/* Store	Group + bubble	*/
1:	retl
	 mov		%g4, %o0

232:	brz,pt		%g2, 2f				/* CTI		Group		*/
	 sub		%o2, %g2, %o2			/* IEU0		Group		*/
1:	ldub		[%o1 - 1], %g5			/* Load		Group		*/
	sub		%o1, 1, %o1			/* IEU0				*/
	sub		%o0, 1, %o0			/* IEU1				*/
	subcc		%g2, 1, %g2			/* IEU1		Group		*/
	bne,pt		%xcc, 1b			/* CTI				*/
	 stb		%g5, [%o0]			/* Store			*/
2:	andn		%o2, 7, %g5 			/* IEU0		Group		*/
	and		%o2, 7, %o2			/* IEU1				*/
	fmovd		%f0, %f2			/* FPU				*/
	alignaddr	%o1, %g0, %g1			/* GRU		Group		*/
	ldd		[%g1], %f4			/* Load		Group		*/
1:	ldd		[%g1 - 8], %f6			/* Load		Group		*/
	sub		%g1, 8, %g1			/* IEU0		Group		*/
	subcc		%g5, 8, %g5			/* IEU1				*/
	faligndata	%f6, %f4, %f0			/* GRU		Group		*/
	std		%f0, [%o0 - 8]			/* Store			*/
	sub		%o1, 8, %o1			/* IEU0		Group		*/
	be,pn		%xcc, 233f			/* CTI				*/
	 sub		%o0, 8, %o0			/* IEU1				*/
	ldd		[%g1 - 8], %f4			/* Load		Group		*/
	sub		%g1, 8, %g1			/* IEU0				*/
	subcc		%g5, 8, %g5			/* IEU1				*/
	faligndata	%f4, %f6, %f0			/* GRU		Group		*/
	std		%f0, [%o0 - 8]			/* Store			*/
	sub		%o1, 8, %o1			/* IEU0				*/
	bne,pn		%xcc, 1b			/* CTI		Group		*/
	 sub		%o0, 8, %o0			/* IEU0				*/
233:	brz,pn		%o2, 234f			/* CTI		Group		*/
	 nop						/* IEU0				*/
237:	ldub		[%o1 - 1], %g5			/* LOAD				*/
	sub		%o1, 1, %o1			/* IEU0				*/
	sub		%o0, 1, %o0			/* IEU1				*/
	subcc		%o2, 1, %o2			/* IEU1				*/
	bne,pt		%xcc, 237b			/* CTI				*/
	 stb		%g5, [%o0]			/* Store	Group		*/
234:	wr		%g0, FPRS_FEF, %fprs
	retl
	 mov		%g4, %o0
END(memmove)
libc_hidden_def(memmove)

#ifdef USE_BPR
weak_alias(memcpy,__align_cpy_1)
weak_alias(memcpy,__align_cpy_2)
weak_alias(memcpy,__align_cpy_4)
weak_alias(memcpy,__align_cpy_8)
weak_alias(memcpy,__align_cpy_16)
#endif
