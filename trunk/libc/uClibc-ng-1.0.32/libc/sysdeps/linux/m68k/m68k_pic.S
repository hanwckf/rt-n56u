/*
 * Copyright (C) 2006 CodeSourcery Inc
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 *
 * This file defines some m68k assembly macros for handling the differences
 * between PIC and non-PIC.
 */
#include <features.h>

	/* When assembling code for shared flat libraries, this is nonzero
	 * if %a5 points the current library's GOT.  */
	.equ	have_current_got, 0

	/* Perform the equivalent of "<op> <target>", where <target> is
	 * a text address.  <tmp> is available as a temporary address
	 * register.  */
	.macro	DO_TEXT op,target,tmp
#if defined __UCLIBC_FORMAT_SHARED_FLAT__
	.ifne	have_current_got
	move.l \target@GOT(%a5),\tmp
	.else
	move.l _current_shared_library_a5_offset_(%a5),\tmp
	move.l \target@GOT(\tmp),\tmp
	.endif
	\op (\tmp)
#elif defined __PIC__
	lea \target-.-8,\tmp
	\op (%pc,\tmp)
#else
	\op \target
#endif
	.endm

	/* Do "pea <target>" when <target> is a text address.
	 * <tmp> is available as a temporary register.  */
	.macro PEA_TEXT target,tmp
	DO_TEXT pea,\target,\tmp
	.endm

	/* Likewise jsr.  */
	.macro CALL target,tmp
	DO_TEXT jsr,\target,\tmp
	.endm

	/* Likewise jmp.  */
	.macro JUMP target,tmp
	DO_TEXT jmp,\target,\tmp
	.endm

	/* Initialize the global pointer, if functions need to do that.  */
	.macro INIT_GP
#if defined __UCLIBC_FORMAT_SHARED_FLAT__
	move.l	%a5,-(%sp)
	move.l _current_shared_library_a5_offset_(%a5),%a5
#endif
	.endm

	/* Undo the effects of INIT_GP.  */
	.macro FINI_GP
#if defined __UCLIBC_FORMAT_SHARED_FLAT__
	move.l	(%sp)+,%a5
#endif
	.endm
