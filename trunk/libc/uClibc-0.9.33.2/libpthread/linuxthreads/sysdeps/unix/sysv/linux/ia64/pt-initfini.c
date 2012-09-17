/* Special .init and .fini section support for ia64. LinuxThreads version.
   Copyright (C) 2000, 2001, 2002, 2003 Free Software Foundation, Inc.
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

#include <stddef.h>

#ifdef HAVE_INITFINI_ARRAY

# define INIT_NEW_WAY \
    ".xdata8 \".init_array\", @fptr(__pthread_initialize_minimal)\n"
# define INIT_OLD_WAY ""
#else
# define INIT_NEW_WAY ""
# define INIT_OLD_WAY \
	"\n\
	st8 [r12] = gp, -16\n\
	br.call.sptk.many b0 = __pthread_initialize_minimal# ;;\n\
	;;\n\
	adds r12 = 16, r12\n\
	;;\n\
	ld8 gp = [r12]\n\
	;;\n"
#endif

__asm__ ("\n\
\n\
#include \"defs.h\"\n\
\n\
/*@HEADER_ENDS*/\n\
\n\
/*@_init_PROLOG_BEGINS*/\n"
	INIT_NEW_WAY
	".section .init\n\
	.align 16\n\
	.global _init#\n\
	.proc _init#\n\
_init:\n\
	.prologue\n\
	.save ar.pfs, r34\n\
	alloc r34 = ar.pfs, 0, 3, 0, 0\n\
	.vframe r32\n\
	mov r32 = r12\n\
	.save rp, r33\n\
	mov r33 = b0\n\
	.body\n\
	adds r12 = -16, r12\n\
	;;\n"
	INIT_OLD_WAY
	".endp _init#\n\
\n\
/*@_init_PROLOG_ENDS*/\n\
\n\
/*@_init_EPILOG_BEGINS*/\n\
	.section .init\n\
	.proc _init#\n\
_init:\n\
	.prologue\n\
	.save ar.pfs, r34\n\
	.vframe r32\n\
	.save rp, r33\n\
	.body\n\
	mov r12 = r32\n\
	mov ar.pfs = r34\n\
	mov b0 = r33\n\
	br.ret.sptk.many b0\n\
	.endp _init#\n\
/*@_init_EPILOG_ENDS*/\n\
\n\
/*@_fini_PROLOG_BEGINS*/\n\
	.section .fini\n\
	.align 16\n\
	.global _fini#\n\
	.proc _fini#\n\
_fini:\n\
	.prologue\n\
	.save ar.pfs, r34\n\
	alloc r34 = ar.pfs, 0, 3, 0, 0\n\
	.vframe r32\n\
	mov r32 = r12\n\
	.save rp, r33\n\
	mov r33 = b0\n\
	.body\n\
	adds r12 = -16, r12\n\
	;;\n\
	.endp _fini#\n\
\n\
/*@_fini_PROLOG_ENDS*/\n\
\n\
/*@_fini_EPILOG_BEGINS*/\n\
	.section .fini\n\
	.proc _fini#\n\
_fini:\n\
	.prologue\n\
	.save ar.pfs, r34\n\
	.vframe r32\n\
	.save rp, r33\n\
	.body\n\
	mov r12 = r32\n\
	mov ar.pfs = r34\n\
	mov b0 = r33\n\
	br.ret.sptk.many b0\n\
	.endp _fini#\n\
\n\
/*@_fini_EPILOG_ENDS*/\n\
\n\
/*@TRAILER_BEGINS*/\n\
	.weak	__gmon_start__#\n\
");
