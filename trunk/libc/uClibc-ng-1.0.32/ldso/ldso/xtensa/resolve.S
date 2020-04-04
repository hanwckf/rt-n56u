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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#define MIN_FRAME_SIZE 32

#ifdef __XTENSA_EB__
#define XTENSA_IMM12_FLD_OFFSET 8
#else /* __XTENSA_EL__ */
#define XTENSA_IMM12_FLD_OFFSET 12
#endif /* __XTENSA_EL__ */

	.text
	.align	4
	.literal_position
	.global	_dl_linux_resolve
	.type	_dl_linux_resolve, @function
_dl_linux_resolve:
#if defined(__XTENSA_WINDOWED_ABI__)
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
#elif defined(__XTENSA_CALL0_ABI__)
	/* Reserve stack space and save incoming arguments.  */
	addi	a1, a1, -32
	s32i	a0, a1, 0
	s32i	a2, a1, 8
	s32i	a3, a1, 12
	s32i	a4, a1, 16
	s32i	a5, a1, 20
	s32i	a6, a1, 24
	s32i	a7, a1, 28

	/* Move arguments for the _dl_linux_resolver to proper registers.  */
	mov	a2, a10
	mov	a3, a11
	/* Call the fixup function.  */
	movi	a0, _dl_linux_resolver
	callx0	a0
	mov	a10, a2

	/* Restore incoming arguments from stack and deallocate reservation.  */
	l32i	a0, a1, 0
	l32i	a2, a1, 8
	l32i	a3, a1, 12
	l32i	a4, a1, 16
	l32i	a5, a1, 20
	l32i	a6, a1, 24
	l32i	a7, a1, 28
	addi	a1, a1, 32

	/* Jump to the target function.  */
	jx	a10
#else
#error Unsupported Xtensa ABI
#endif
	.size	_dl_linux_resolve, . - _dl_linux_resolve
