/* Special .init and .fini section support for SH. Linuxthread version.
   Copyright (C) 2000, 2001, 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   In addition to the permissions in the GNU Lesser General Public
   License, the Free Software Foundation gives you unlimited
   permission to link the compiled version of this file with other
   programs, and to distribute those programs without any restriction
   coming from the use of this file.  (The Lesser General Public
   License restrictions do apply in other respects; for example, they
   cover modification of the file, and distribution when not linked
   into another program.)

   The GNU C Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* This file is compiled into assembly code which is then munged by a sed
   script into two files: crti.s and crtn.s.

   * crti.s puts a function prologue at the beginning of the
   .init and .fini sections and defines global symbols for
   those addresses, so they can be called as functions.

   * crtn.s puts the corresponding function epilogues
   in the .init and .fini sections. */

__asm__ ("\n\
\n\
#include \"defs.h\"\n\
\n\
/*@HEADER_ENDS*/\n\
\n\
/*@TESTS_BEGIN*/\n\
\n\
/*@TESTS_END*/\n\
\n\
/*@_init_PROLOG_BEGINS*/\n\
	.section .init\n\
	.align 5\n\
	.global	_init\n\
	.type	_init,@function\n\
_init:\n\
	mov.l	r12,@-r15\n\
	mov.l	r14,@-r15\n\
	sts.l	pr,@-r15\n\
	mova	.L22,r0\n\
	mov.l	.L22,r12\n\
	add	r0,r12\n\
	mova	.L24,r0\n\
	mov.l	.L24,r1\n\
	add	r0,r1\n\
	jsr	@r1\n\
	 nop\n\
	mova	.L23,r0\n\
	mov.l	.L23,r1\n\
	add	r0,r1\n\
	jsr	@r1\n\
	 mov	r15,r14\n\
	bra	1f\n\
	 nop\n\
	.align 2\n\
.L22:\n\
	.long	_GLOBAL_OFFSET_TABLE_\n\
.L23:\n\
	.long	__gmon_start__@PLT\n\
.L24:\n\
	.long	__pthread_initialize_minimal@PLT\n\
1:\n\
	ALIGN\n\
	END_INIT\n\
\n\
/*@_init_PROLOG_ENDS*/\n\
\n\
/*@_init_EPILOG_BEGINS*/\n\
	.section .init\n\
	mov	r14,r15\n\
	lds.l	@r15+,pr\n\
	mov.l	@r15+,r14\n\
	rts	\n\
	mov.l	@r15+,r12\n\
	END_INIT\n\
	.section .text\n\
	.align 5\n\
	.weak	__gmon_start__\n\
	.type	__gmon_start__,@function\n\
__gmon_start__:\n\
	mov.l	r14,@-r15\n\
	mov	r15,r14\n\
	mov	r14,r15\n\
	rts	\n\
	mov.l	@r15+,r14\n\
	\n\
/*@_init_EPILOG_ENDS*/\n\
\n\
/*@_fini_PROLOG_BEGINS*/\n\
	.section .fini\n\
	.align 5\n\
	.global	_fini\n\
	.type	_fini,@function\n\
_fini:\n\
	mov.l	r12,@-r15\n\
	mov.l	r14,@-r15\n\
	sts.l	pr,@-r15\n\
	mova	.L27,r0\n\
	mov.l	.L27,r12\n\
	add	r0,r12\n\
	mov	r15,r14\n\
	ALIGN\n\
	END_FINI\n\
	bra	1f\n\
	 nop\n\
	.align	2\n\
.L27:\n\
	.long	_GLOBAL_OFFSET_TABLE_\n\
1:\n\
/*@_fini_PROLOG_ENDS*/\n\
\n\
/*@_fini_EPILOG_BEGINS*/\n\
	.section .fini\n\
	mov	r14,r15\n\
	lds.l	@r15+,pr\n\
	mov.l	@r15+,r14\n\
	rts	\n\
	mov.l	@r15+,r12\n\
\n\
	END_FINI\n\
	\n\
/*@_fini_EPILOG_ENDS*/\n\
\n\
/*@TRAILER_BEGINS*/\n\
");
