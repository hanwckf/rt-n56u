/*
 * Copyright (C) 2016 Andes Technology, Inc.
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/* Startup code compliant to the ELF NDS32 ABI */

#include <sys/regdef.h>
#include <features.h>
#define BP_SYM(name) name

/* We need to call:
   __uClibc_main (int (*main) (int, char **, char **), int argc,
		      char **argv, void (*init) (void), void (*fini) (void),
		      void (*rtld_fini) (void), void *stack_end)
*/

.text
	.globl	_start
	.type	_start,@function
	.type	_init,@function
	.type	_fini,@function
#ifndef __UCLIBC_CTOR_DTOR__
	.weak	_init
	.weak	_fini
#endif
	.type	    main,@function
	.type	    __uClibc_main,@function
#ifdef SHARED
.pic
1:
	ret
#endif

_start:
	movi    $fp,	0		! clear FP
	lwi	$r1,	[$sp + 0]	! r1 = argc
	addi	$r2,	$sp,	4	! r2 = argv

	/* align sp to 8-byte boundary */
	movi	$r0,	-8
	and	$sp,	$sp,	$r0

	addi	$r6,	$sp,	0	! r6 = stack top

#ifdef SHARED
	/* set gp register */
	mfusr 	$r15, 	$PC
	sethi	$gp,	HI20(_GLOBAL_OFFSET_TABLE_+4)
	ori	$gp,	$gp, LO12(_GLOBAL_OFFSET_TABLE_ + 8)
	add	$gp,	$r15,	$gp

	la		$r3,	_init@GOTOFF
	la		$r4,	_fini@GOTOFF
	la		$r0,	main@GOT

	/* push everything to stack, r5 is rtld_fini and r7 is garbage */
	pushm	$r0,	$r7

	/* now start it up */
	bal	__uClibc_main@PLT

	/* should never get here */
	bal	abort@PLT
#else
	la      $gp, _SDA_BASE_ ! init GP for small data access

	la		$r3,	_init
	la		$r4,	_fini
	la		$r0,	main

	/* push everything to stack, r5 is rtld_fini and r7 is garbage */
	pushm	$r0,	$r7

	/* now start it up */
	bal	__uClibc_main

	/* should never get here */
	bal	abort
#endif
	ret

/* Define a symbol for the first piece of initialized data.  */
	.data
	.globl __data_start
__data_start:
	.long	0
	.weak data_start
	data_start = __data_start
