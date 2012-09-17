/*
 * libc/sysdeps/linux/microblaze/crtn.S -- init/fini exit code for microblaze
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

	.section .init
	.align	2
	.globl	_init
	.ent	_init

	lwi	r15, r1, 0
	lwi	r19, r1, 28
	rtsd	r15, 8
	addik	r1, r1, 32	# Delay slot

	.end	_init
$Lfe2:

	.section .fini
	.align	2
	.globl	_fini
	.ent	_fini

	lwi	r15, r1, 0
	lwi	r19, r1, 28
	rtsd	r15, 8
	addik	r1, r1, 32	# Delay slot

	.end	_fini
$Lfe3:

/*@TRAILER_BEGINS*/
