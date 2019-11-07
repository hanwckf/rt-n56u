/*
 * Copyright (C) 2016 Andes Technology, Inc.
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

	.pic
#APP
	.section .init
	.align	2
	.globl	_init
	.type	_init, @function
_init:
.LFB28:
        ! Generate instructions for ABI: 2
	! pretend args size: 0, auto vars size: 0, pushed regs size: 12, outgoing args size: 0
	! frame pointer: $fp, needed: yes
	! $fp $gp $lp
	! prologue
	.off_16bit
	smw.adm $sp,[$sp],$sp,#0x8
	smw.adm $sp,[$sp],$sp,#0x6
	.restore_16bit
	sethi	$gp, hi20(_GLOBAL_OFFSET_TABLE_-8)
	ori	$gp, $gp, lo12(_GLOBAL_OFFSET_TABLE_-4)
	mfusr	$ta, $pc
	add	$gp, $ta, $gp
        addi    $sp, $sp, -4
        addi    $fp, $sp, 8
	! end of prologue
#APP
	.align	2

	.section .fini
	.align	2
	.globl	_fini
	.type	_fini, @function
_fini:
.LFB29:
        ! Generate instructions for ABI: 2
	! pretend args size: 0, auto vars size: 0, pushed regs size: 12, outgoing args size: 0
	! frame pointer: $fp, needed: yes
	! $fp $gp $lp
	! prologue
	.off_16bit
	smw.adm $sp,[$sp],$sp,#0x8
	smw.adm $sp,[$sp],$sp,#0x6
	.restore_16bit
	sethi	$gp, hi20(_GLOBAL_OFFSET_TABLE_-8)
	ori	$gp, $gp, lo12(_GLOBAL_OFFSET_TABLE_-4)
	mfusr	$ta, $pc
	add	$gp, $ta, $gp
        addi    $sp, $sp, -4
        addi    $fp, $sp, 8
	! end of prologue
#APP
	.align  2
	.ident	"GCC: (GNU) 3.4.4"
