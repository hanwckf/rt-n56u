/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

# setjmp/longjmp for Frv.  The jmpbuf looks like this:
#	
# Register	jmpbuf offset
# R16-R31	0x0-0x03c
# R48-R63	0x40-0x7c
# FR16-FR31     0x80-0xbc
# FR48-FR63     0xc0-0xfc
# LR            0x100
# SP            0x104
# FP            0x108
#
# R8 contains the pointer to jmpbuf

	.text
	.global	__longjmp
	.type	__longjmp,@function
__longjmp:
	lddi	@(gr8,0), gr16
	lddi	@(gr8,8), gr18
	lddi 	@(gr8,16), gr20
	lddi	@(gr8,24), gr22
	lddi	@(gr8,32), gr24
	lddi	@(gr8,40), gr26
	lddi	@(gr8,48), gr28
	lddi	@(gr8,56), gr30
#if __FRV_GPR__ != 32
	lddi	@(gr8,64), gr48
	lddi	@(gr8,72), gr50
	lddi	@(gr8,80), gr52
	lddi	@(gr8,88), gr54
	lddi	@(gr8,96), gr56
	lddi	@(gr8,104), gr58
	lddi	@(gr8,112), gr60
	lddi	@(gr8,120), gr62
#endif

#if __FRV_FPR__ != 0
	lddfi	@(gr8,128), fr16
	lddfi	@(gr8,136), fr18
	lddfi 	@(gr8,144), fr20
	lddfi	@(gr8,152), fr22
	lddfi	@(gr8,160), fr24
	lddfi	@(gr8,168), fr26
	lddfi	@(gr8,176), fr28
	lddfi	@(gr8,184), fr30
#if __FRV_FPR__ != 32
	lddfi	@(gr8,192), fr48
	lddfi	@(gr8,200), fr50
	lddfi	@(gr8,208), fr52
	lddfi	@(gr8,216), fr54
	lddfi	@(gr8,224), fr56
	lddfi	@(gr8,232), fr58
	lddfi	@(gr8,240), fr60
	lddfi	@(gr8,248), fr62
#endif
#endif

        ldi     @(gr8,256), gr4
	movgs   gr4,lr

        ldi     @(gr8,260), sp
        ldi     @(gr8,264), fp
	
# Value to return is in r9.  If zero, return 1
	cmp	gr9, gr0, icc0
	setlos	#1, gr8
	ckne	icc0, cc4
	cmov	gr9, gr8, cc4, 1
	ret
.Lend2:
	.size	__longjmp,.Lend2-__longjmp

libc_hidden_def(__longjmp)
