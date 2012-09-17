/* Xtensa dynamic resolver.
   Parts copied from glibc/sysdeps/xtensa/dl-trampoline.S
   Copyright (C) 2007 Free Software Foundation, Inc.
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

#define MIN_FRAME_SIZE 32

#ifdef __XTENSA_EB__
#define XTENSA_IMM12_FLD_OFFSET 8
#else /* __XTENSA_EL__ */
#define XTENSA_IMM12_FLD_OFFSET 12
#endif /* __XTENSA_EL__ */

	.text
	.align	4
	.global	_dl_linux_resolve
	.type	_dl_linux_resolve, @function
_dl_linux_resolve:
	/* Fix up the high 2 bits of the return address.  */
	movi	a13, 0f
	slli	a12, a0, 2
0:	extui	a13, a13, 30, 2
	ssai	2
	src	a12, a13, a12

	/* Call the fixup function.  */
	movi	a8, _dl_linux_resolver
	callx8	a8

	/* Extract the target's frame size from the ENTRY instruction.  */
	l32i	a11, a10, 0
	extui	a11, a11, XTENSA_IMM12_FLD_OFFSET, 12
	slli	a11, a11, 3

	addi	a11, a11, -MIN_FRAME_SIZE
	sub	a11, sp, a11
	movsp	sp, a11

	/* Jump to the next instruction past the ENTRY.  */
	addi	a10, a10, 3
	jx	a10
	.size	_dl_linux_resolve, . - _dl_linux_resolve
