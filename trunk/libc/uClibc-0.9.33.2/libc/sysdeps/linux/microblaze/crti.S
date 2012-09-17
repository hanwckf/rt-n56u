/*
 * libc/sysdeps/linux/microblaze/crti.S -- init/fini entry code for microblaze
 *                                         (baselined with gcc 4.1.2)
 *
 *  Copyright (C) 2010       Digital Design Corporation
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License.  See the file COPYING.LIB in the main
 * directory of this archive for more details.
 */

#define END_INIT
#define END_FINI
#define ALIGN
#include <libc-symbols.h>

/*@HEADER_ENDS*/

	.section .init
	.align	2
	.globl	_init
_init:
	addik	r1, r1, -32
	swi	r19, r1, 28
	addk	r19, r1, r0
	swi	r15, r1, 0

	ALIGN
	END_INIT

	.section .fini
	.align	2
	.globl	_fini
_fini:
	addik	r1, r1, -32
	swi	r19, r1, 28
	addk	r19, r1, r0
	swi	r15, r1, 0

	ALIGN
	END_FINI
