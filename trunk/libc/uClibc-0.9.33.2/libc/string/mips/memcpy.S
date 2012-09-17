/* Copyright (C) 2002, 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Hartvig Ekner <hartvige@mips.com>, 2002.

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
/*#include <sysdep.h>*/
#include <endian.h>
#include "sysdep.h"

/* void *memcpy(void *s1, const void *s2, size_t n);  */

#ifdef __mips64

#include <sys/asm.h>

#if __BYTE_ORDER == __BIG_ENDIAN
#  define LDHI	ldl		/* high part is left in big-endian	*/
#  define SDHI	sdl		/* high part is left in big-endian	*/
#  define LDLO	ldr		/* low part is right in big-endian	*/
#  define SDLO	sdr		/* low part is right in big-endian	*/
#else
#  define LDHI	ldr		/* high part is right in little-endian	*/
#  define SDHI	sdr		/* high part is right in little-endian	*/
#  define LDLO	ldl		/* low part is left in little-endian	*/
#  define SDLO	sdl		/* low part is left in little-endian	*/
#endif

ENTRY (memcpy)
	.set	noreorder

	slti	t0, a2, 16		# Less than 16?
	bne	t0, zero, L(last16)
	move	v0, a0			# Setup exit value before too late

	xor	t0, a1, a0		# Find a0/a1 displacement
	andi	t0, 0x7
	bne	t0, zero, L(shift)	# Go handle the unaligned case
	PTR_SUBU t1, zero, a1
	andi	t1, 0x7			# a0/a1 are aligned, but are we
	beq	t1, zero, L(chk8w)	#  starting in the middle of a word?
	PTR_SUBU a2, t1
	LDHI	t0, 0(a1)		# Yes we are... take care of that
	PTR_ADDU a1, t1
	SDHI	t0, 0(a0)
	PTR_ADDU a0, t1

L(chk8w):
	andi	t0, a2, 0x3f		# 64 or more bytes left?
	beq	t0, a2, L(chk1w)
	PTR_SUBU a3, a2, t0		# Yes
	PTR_ADDU a3, a1			# a3 = end address of loop
	move	a2, t0			# a2 = what will be left after loop
L(lop8w):	
	ld	t0,  0(a1)		# Loop taking 8 words at a time
	ld	t1,  8(a1)
	ld	t2, 16(a1)
	ld	t3, 24(a1)
	ld	ta0, 32(a1)
	ld	ta1, 40(a1)
	ld	ta2, 48(a1)
	ld	ta3, 56(a1)
	PTR_ADDIU a0, 64
	PTR_ADDIU a1, 64
	sd	t0, -64(a0)
	sd	t1, -56(a0)
	sd	t2, -48(a0)
	sd	t3, -40(a0)
	sd	ta0, -32(a0)
	sd	ta1, -24(a0)
	sd	ta2, -16(a0)
	bne	a1, a3, L(lop8w)
	sd	ta3,  -8(a0)

L(chk1w):
	andi	t0, a2, 0x7		# 8 or more bytes left?
	beq	t0, a2, L(last16)
	PTR_SUBU a3, a2, t0		# Yes, handle them one dword at a time
	PTR_ADDU a3, a1			# a3 again end address
	move	a2, t0
L(lop1w):
	ld	t0, 0(a1)
	PTR_ADDIU a0, 8
	PTR_ADDIU a1, 8
	bne	a1, a3, L(lop1w)
	sd	t0, -8(a0)

L(last16):
	blez	a2, L(lst16e)		# Handle last 16 bytes, one at a time
	PTR_ADDU a3, a2, a1
L(lst16l):
	lb	t0, 0(a1)
	PTR_ADDIU a0, 1
	PTR_ADDIU a1, 1
	bne	a1, a3, L(lst16l)
	sb	t0, -1(a0)
L(lst16e):
	jr	ra			# Bye, bye
	nop

L(shift):
	PTR_SUBU a3, zero, a0		# Src and Dest unaligned 
	andi	a3, 0x7			#  (unoptimized case...)
	beq	a3, zero, L(shft1)
	PTR_SUBU a2, a3			# a2 = bytes left
	LDHI	t0, 0(a1)		# Take care of first odd part
	LDLO	t0, 7(a1)
	PTR_ADDU a1, a3
	SDHI	t0, 0(a0)
	PTR_ADDU a0, a3
L(shft1):
	andi	t0, a2, 0x7
	PTR_SUBU a3, a2, t0
	PTR_ADDU a3, a1
L(shfth):
	LDHI	t1, 0(a1)		# Limp through, dword by dword
	LDLO	t1, 7(a1)
	PTR_ADDIU a0, 8
	PTR_ADDIU a1, 8
	bne	a1, a3, L(shfth)
	sd	t1, -8(a0)
	b	L(last16)		# Handle anything which may be left
	move	a2, t0

	.set	reorder
END (memcpy)

#else /* !__mips64 */

#if __BYTE_ORDER == __BIG_ENDIAN
#  define LWHI	lwl		/* high part is left in big-endian	*/
#  define SWHI	swl		/* high part is left in big-endian	*/
#  define LWLO	lwr		/* low part is right in big-endian	*/
#  define SWLO	swr		/* low part is right in big-endian	*/
#else
#  define LWHI	lwr		/* high part is right in little-endian	*/
#  define SWHI	swr		/* high part is right in little-endian	*/
#  define LWLO	lwl		/* low part is left in little-endian	*/
#  define SWLO	swl		/* low part is left in little-endian	*/
#endif

ENTRY (memcpy)
	.set	noreorder

	slti	t0, a2, 8		# Less than 8?
	bne	t0, zero, L(last8)
	move	v0, a0			# Setup exit value before too late

	xor	t0, a1, a0		# Find a0/a1 displacement
	andi	t0, 0x3
	bne	t0, zero, L(shift)	# Go handle the unaligned case
	subu	t1, zero, a1
	andi	t1, 0x3			# a0/a1 are aligned, but are we
	beq	t1, zero, L(chk8w)	#  starting in the middle of a word?
	subu	a2, t1
	LWHI	t0, 0(a1)		# Yes we are... take care of that
	addu	a1, t1
	SWHI	t0, 0(a0)
	addu	a0, t1

L(chk8w):	
	andi	t0, a2, 0x1f		# 32 or more bytes left?
	beq	t0, a2, L(chk1w)
	subu	a3, a2, t0		# Yes
	addu	a3, a1			# a3 = end address of loop
	move	a2, t0			# a2 = what will be left after loop
L(lop8w):	
	lw	t0,  0(a1)		# Loop taking 8 words at a time
	lw	t1,  4(a1)
	lw	t2,  8(a1)
	lw	t3, 12(a1)
	lw	t4, 16(a1)
	lw	t5, 20(a1)
	lw	t6, 24(a1)
	lw	t7, 28(a1)
	addiu	a0, 32
	addiu	a1, 32
	sw	t0, -32(a0)
	sw	t1, -28(a0)
	sw	t2, -24(a0)
	sw	t3, -20(a0)
	sw	t4, -16(a0)
	sw	t5, -12(a0)
	sw	t6,  -8(a0)
	bne	a1, a3, L(lop8w)
	sw	t7,  -4(a0)

L(chk1w):	
	andi	t0, a2, 0x3		# 4 or more bytes left?
	beq	t0, a2, L(last8)
	subu	a3, a2, t0		# Yes, handle them one word at a time
	addu	a3, a1			# a3 again end address
	move	a2, t0
L(lop1w):	
	lw	t0, 0(a1)
	addiu	a0, 4
	addiu	a1, 4
	bne	a1, a3, L(lop1w)
	sw	t0, -4(a0)

L(last8):	
	blez	a2, L(lst8e)		# Handle last 8 bytes, one at a time
	addu	a3, a2, a1
L(lst8l):	
	lb	t0, 0(a1)
	addiu	a0, 1
	addiu	a1, 1
	bne	a1, a3, L(lst8l)
	sb	t0, -1(a0)
L(lst8e):	
	jr	ra			# Bye, bye
	nop

L(shift):	
	subu	a3, zero, a0		# Src and Dest unaligned 
	andi	a3, 0x3			#  (unoptimized case...)
	beq	a3, zero, L(shft1)
	subu	a2, a3			# a2 = bytes left
	LWHI	t0, 0(a1)		# Take care of first odd part
	LWLO	t0, 3(a1)
	addu	a1, a3
	SWHI	t0, 0(a0)
	addu	a0, a3
L(shft1):	
	andi	t0, a2, 0x3
	subu	a3, a2, t0
	addu	a3, a1
L(shfth):	
	LWHI	t1, 0(a1)		# Limp through, word by word
	LWLO	t1, 3(a1)
	addiu	a0, 4
	addiu	a1, 4
	bne	a1, a3, L(shfth)
	sw	t1, -4(a0)
	b	L(last8)		# Handle anything which may be left
	move	a2, t0

	.set	reorder
END (memcpy)

#endif /* !__mips64 */

libc_hidden_def(memcpy)
