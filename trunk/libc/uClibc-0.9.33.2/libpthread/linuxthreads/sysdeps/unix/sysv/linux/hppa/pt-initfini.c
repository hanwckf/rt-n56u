/* Special .init and .fini section support for HPPA.  Linuxthreads version.
   Copyright (C) 2001, 2003 Free Software Foundation, Inc.
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

/* If we use the standard C version, the linkage table pointer won't
   be properly preserved due to the splitting up of function prologues
   and epilogues.  Therefore we write these in assembly to make sure
   they do the right thing.  */

__asm__ (
"#include \"defs.h\"\n"
"\n"
"/*@HEADER_ENDS*/\n"
"\n"
"/*@_init_PROLOG_BEGINS*/\n"
"	.section .init\n"
"	.align 4\n"
"	.globl _init\n"
"	.type _init,@function\n"
"_init:\n"
"	stw	%rp,-20(%sp)\n"
"	stwm	%r4,64(%sp)\n"
"	stw	%r19,-32(%sp)\n"
"	bl	__pthread_initialize_minimal,%rp\n"
"	copy	%r19,%r4	/* delay slot */\n"
"	copy	%r4,%r19\n"
"/*@_init_PROLOG_ENDS*/\n"
"\n"
"/*@_init_EPILOG_BEGINS*/\n"
"/* Here is the tail end of _init.  */\n"
"	.section .init\n"
"	ldw	-84(%sp),%rp\n"
"	copy	%r4,%r19\n"
"	bv	%r0(%rp)\n"
"_end_init:\n"
"	ldwm	-64(%sp),%r4\n"
"\n"
"/* Our very own unwind info, because the assembler can't handle\n"
"   functions split into two or more pieces.  */\n"
"	.section .PARISC.unwind,\"a\",@progbits\n"
"	.extern _init\n"
"	.word	_init, _end_init\n"
"	.byte	0x08, 0x01, 0x00, 0x08, 0x00, 0x00, 0x00, 0x08\n"
"\n"
"/*@_init_EPILOG_ENDS*/\n"
"\n"
"/*@_fini_PROLOG_BEGINS*/\n"
"	.section .fini\n"
"	.align 4\n"
"	.globl _fini\n"
"	.type _fini,@function\n"
"_fini:\n"
"	stw	%rp,-20(%sp)\n"
"	stwm	%r4,64(%sp)\n"
"	stw	%r19,-32(%sp)\n"
"	copy	%r19,%r4\n"
"/*@_fini_PROLOG_ENDS*/\n"
"\n"
"/*@_fini_EPILOG_BEGINS*/\n"
"	.section .fini\n"
"	ldw	-84(%sp),%rp\n"
"	copy	%r4,%r19\n"
"	bv	%r0(%rp)\n"
"_end_fini:\n"
"	ldwm	-64(%sp),%r4\n"
"\n"
"	.section .PARISC.unwind,\"a\",@progbits\n"
"	.extern _fini\n"
"	.word	_fini, _end_fini\n"
"	.byte	0x08, 0x01, 0x00, 0x08, 0x00, 0x00, 0x00, 0x08\n"
"\n"
"/*@_fini_EPILOG_ENDS*/\n"
"\n"
"/*@TRAILER_BEGINS*/\n"
);
