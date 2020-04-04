/*
 * Architecture specific code used by dl-startup.c
 * Copyright (C) 2016 Waldemar Brodkorb <wbx@uclibc-ng.org>
 * Ported from GNU libc
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/* Copyright (C) 1995-2016 Free Software Foundation, Inc.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <features.h>

__asm__("\
.text									\n\
.globl _start								\n\
.type _start, %function							\n\
.globl _dl_start_user							\n\
.type _dl_start_user, %function						\n\
_start:									\n\
	mov	x0, sp							\n\
	bl	_dl_start						\n\
	// returns user entry point in x0				\n\
	mov	x21, x0							\n\
_dl_start_user:								\n\
	// get the original arg count					\n\
	ldr	x1, [sp]						\n\
	// get the argv address						\n\
	add	x2, sp, #(1<<3)						\n\
	// get _dl_skip_args to see if we were				\n\
	// invoked as an executable					\n\
	adrp	x4, _dl_skip_args					\n\
        ldr	w4, [x4, #:lo12:_dl_skip_args]				\n\
	// do we need to adjust argc/argv				\n\
        cmp	w4, 0							\n\
	beq	.L_done_stack_adjust					\n\
	// subtract _dl_skip_args from original arg count		\n\
	sub	x1, x1, x4						\n\
	// store adjusted argc back to stack				\n\
	str	x1, [sp]						\n\
	// find the first unskipped argument				\n\
	mov	x3, x2							\n\
	add	x4, x2, x4, lsl #3					\n\
	// shuffle envp down						\n\
1:	ldr	x5, [x4], #(1<<3)					\n\
	str	x5, [x3], #(1<<3)					\n\
	cmp	x5, #0							\n\
	bne	1b							\n\
	// shuffle auxv down						\n\
1:	ldp	x0, x5, [x4, #(2<<3)]!					\n\
	stp	x0, x5, [x3], #(2<<3)					\n\
	cmp	x0, #0							\n\
	bne	1b							\n\
.L_done_stack_adjust:							\n\
	// compute envp							\n\
	add	x3, x2, x1, lsl #3					\n\
	add	x3, x3, #(1<<3)						\n\
	// load the finalizer function					\n\
	adrp	x0, _dl_fini						\n\
	add	x0, x0, #:lo12:_dl_fini					\n\
	// jump to the user_s entry point				\n\
	br      x21							\n\
");

/* Get a pointer to the argv array.  On many platforms this can be just
 * the address of the first argument, on other platforms we need to
 * do something a little more subtle here.  */
#define GET_ARGV(ARGVP, ARGS) ARGVP = (((unsigned long*)ARGS)+1)

/* Handle relocation of the symbols in the dynamic loader. */
static __always_inline
void PERFORM_BOOTSTRAP_RELOC(ELF_RELOC *rpnt, ElfW(Addr) *reloc_addr,
	ElfW(Addr) symbol_addr, ElfW(Addr) load_addr, ElfW(Addr) *sym)
{
	switch (ELF_R_TYPE(rpnt->r_info)) {
		case R_AARCH64_NONE:
			break;
		case R_AARCH64_ABS64:
		case R_AARCH64_GLOB_DAT:
		case R_AARCH64_JUMP_SLOT:
			*reloc_addr = symbol_addr + rpnt->r_addend;
			break;
		default:
			_dl_exit(1);
	}
}
