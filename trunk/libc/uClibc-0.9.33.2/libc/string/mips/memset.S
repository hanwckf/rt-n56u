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

/* void *memset(void *s, int c, size_t n).  */

#ifdef __mips64

#include <sys/asm.h>

#if __BYTE_ORDER == __BIG_ENDIAN
# define SDHI	sdl		/* high part is left in big-endian	*/
#else
# define SDHI	sdr		/* high part is right in little-endian	*/
#endif

ENTRY (memset)
	.set	noreorder

	slti	ta1, a2, 16		# Less than 16?
	bne	ta1, zero, L(last16)
	move	v0, a0			# Setup exit value before too late

	beq	a1, zero, L(ueven)	# If zero pattern, no need to extend
	andi	a1, 0xff		# Avoid problems with bogus arguments
	dsll	ta0, a1, 8
	or	a1, ta0
	dsll	ta0, a1, 16
	or	a1, ta0			# a1 is now pattern in full word
	dsll	ta0, a1, 32
	or	a1, ta0			# a1 is now pattern in double word

L(ueven):
	PTR_SUBU ta0, zero, a0		# Unaligned address?
	andi	ta0, 0x7
	beq	ta0, zero, L(chkw)
	PTR_SUBU a2, ta0
	SDHI	a1, 0(a0)		# Yes, handle first unaligned part
	PTR_ADDU a0, ta0		# Now both a0 and a2 are updated

L(chkw):
	andi	ta0, a2, 0xf		# Enough left for one loop iteration?
	beq	ta0, a2, L(chkl)
	PTR_SUBU a3, a2, ta0
	PTR_ADDU a3, a0			# a3 is last loop address +1
	move	a2, ta0			# a2 is now # of bytes left after loop
L(loopw):
	PTR_ADDIU a0, 16		# Handle 2 dwords pr. iteration
	sd	a1, -16(a0)
	bne	a0, a3, L(loopw)
	sd	a1,  -8(a0)

L(chkl):
	andi	ta0, a2, 0x8		# Check if there is at least a double
	beq	ta0, zero, L(last16)	#  word remaining after the loop
	PTR_SUBU a2, ta0
	sd	a1, 0(a0)		# Yes...
	PTR_ADDIU a0, 8

L(last16):
	blez	a2, L(exit)		# Handle last 16 bytes (if cnt>0)
	PTR_ADDU a3, a2, a0		# a3 is last address +1
L(lst16l):
	PTR_ADDIU a0, 1
	bne	a0, a3, L(lst16l)
	sb	a1, -1(a0)
L(exit):
	j	ra			# Bye, bye
	nop

	.set	reorder
END (memset)

#else /* !__mips64 */

#if __BYTE_ORDER == __BIG_ENDIAN
# define SWHI	swl		/* high part is left in big-endian	*/
#else
# define SWHI	swr		/* high part is right in little-endian	*/
#endif

ENTRY (memset)
	.set	noreorder

	slti	t1, a2, 8		# Less than 8?
	bne	t1, zero, L(last8)
	move	v0, a0			# Setup exit value before too late

	beq	a1, zero, L(ueven)	# If zero pattern, no need to extend
	andi	a1, 0xff		# Avoid problems with bogus arguments
	sll	t0, a1, 8
	or	a1, t0
	sll	t0, a1, 16
	or	a1, t0			# a1 is now pattern in full word

L(ueven):	
	subu	t0, zero, a0		# Unaligned address?
	andi	t0, 0x3
	beq	t0, zero, L(chkw)
	subu	a2, t0
	SWHI	a1, 0(a0)		# Yes, handle first unaligned part
	addu	a0, t0			# Now both a0 and a2 are updated

L(chkw):	
	andi	t0, a2, 0x7		# Enough left for one loop iteration?
	beq	t0, a2, L(chkl)
	subu	a3, a2, t0
	addu	a3, a0			# a3 is last loop address +1
	move	a2, t0			# a2 is now # of bytes left after loop
L(loopw):	
	addiu	a0, 8			# Handle 2 words pr. iteration
	sw	a1, -8(a0)
	bne	a0, a3, L(loopw)
	sw	a1, -4(a0)

L(chkl):	
	andi	t0, a2, 0x4		# Check if there is at least a full
	beq	t0, zero, L(last8)	#  word remaining after the loop
	subu	a2, t0
	sw	a1, 0(a0)		# Yes...
	addiu	a0, 4

L(last8):	
	blez	a2, L(exit)		# Handle last 8 bytes (if cnt>0)
	addu	a3, a2, a0		# a3 is last address +1
L(lst8l):	
	addiu	a0, 1
	bne	a0, a3, L(lst8l)
	sb	a1, -1(a0)
L(exit):	
	j	ra			# Bye, bye
	nop

	.set	reorder
END (memset)

#endif /* !__mips64 */

libc_hidden_def(memset)
