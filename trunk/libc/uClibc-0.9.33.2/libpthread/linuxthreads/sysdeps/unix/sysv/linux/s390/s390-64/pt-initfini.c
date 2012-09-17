/* Special .init and .fini section support for 64 bit S/390.
   Copyright (C) 2001 Free Software Foundation, Inc.
   Contributed by Martin Schwidefsky (schwidefsky@de.ibm.com).
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

__asm__ ("\
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
\n\
	.section .init\n\
#NO_APP\n\
	.align 4\n\
.globl _init\n\
	.type	 _init,@function\n\
_init:\n\
#	leaf function           0\n\
#	automatics              0\n\
#	outgoing args           0\n\
#	need frame pointer      0\n\
#	call alloca             0\n\
#	has varargs             0\n\
#	incoming args (stack)   0\n\
#	function length         36\n\
	STMG	6,15,48(15)\n\
	LGR	1,15\n\
	AGHI	15,-160\n\
	STG	1,0(15)\n\
	LARL	12,_GLOBAL_OFFSET_TABLE_\n\
	BRASL	14,__pthread_initialize_minimal@PLT\n\
	LARL	1,__gmon_start__@GOTENT\n\
	LG	1,0(1)\n\
	LTGR	1,1\n\
	JE	.L22\n\
	BASR	14,1\n\
.L22:\n\
#APP\n\
	.align 4,0x07\n\
	END_INIT\n\
\n\
/*@_init_PROLOG_ENDS*/\n\
\n\
/*@_init_EPILOG_BEGINS*/\n\
	.align 4\n\
	.section .init\n\
#NO_APP\n\
	.align 4\n\
	LG	4,272(15)\n\
	LMG	6,15,208(15)\n\
	BR	4\n\
#APP\n\
	END_INIT\n\
\n\
/*@_init_EPILOG_ENDS*/\n\
\n\
/*@_fini_PROLOG_BEGINS*/\n\
	.section .fini\n\
#NO_APP\n\
	.align 4\n\
.globl _fini\n\
	.type	 _fini,@function\n\
_fini:\n\
#	leaf function           0\n\
#	automatics              0\n\
#	outgoing args           0\n\
#	need frame pointer      0\n\
#	call alloca             0\n\
#	has varargs             0\n\
#	incoming args (stack)   0\n\
#	function length         30\n\
	STMG	6,15,48(15)\n\
	LGR	1,15\n\
	AGHI	15,-160\n\
	STG	1,0(15)\n\
	LARL	12,_GLOBAL_OFFSET_TABLE_\n\
#APP\n\
	.align 4,0x07\n\
	END_FINI\n\
\n\
/*@_fini_PROLOG_ENDS*/\n\
\n\
/*@_fini_EPILOG_BEGINS*/\n\
	.align 4\n\
	.section .fini\n\
#NO_APP\n\
	.align 4\n\
	LG	4,272(15)\n\
	LMG	6,15,208(15)\n\
	BR	4\n\
#APP\n\
	END_FINI\n\
\n\
/*@_fini_EPILOG_ENDS*/\n\
\n\
/*@TRAILER_BEGINS*/\n\
	");
