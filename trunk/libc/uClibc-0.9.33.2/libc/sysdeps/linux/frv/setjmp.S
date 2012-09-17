	.text

/* This just does a tail-call to `__sigsetjmp (ARG, 0)'.
   We cannot do it in C because it must be a tail-call, so frame-unwinding
   in setjmp doesn't clobber the state restored by longjmp.  */

	.global _setjmp
	.type	_setjmp,@function
_setjmp:
	setlos	#0, gr9
	bra	.Lsigsetjmp_intern
	.size	_setjmp,.-_setjmp
	
/* This just does a tail-call to `__sigsetjmp (ARG, 1)'.
   We cannot do it in C because it must be a tail-call, so frame-unwinding
   in setjmp doesn't clobber the state restored by longjmp.  */

	.align 4
	.type	setjmp,@function
	.globl	setjmp
setjmp:	
	setlos	#1, gr9
	bra	.Lsigsetjmp_intern
	.size	setjmp,.-setjmp
	
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

	.global	__sigsetjmp
	.type	__sigsetjmp,@function
__sigsetjmp:
.Lsigsetjmp_intern:
	stdi	gr16, @(gr8,0)
	stdi	gr18, @(gr8,8)
	stdi 	gr20, @(gr8,16)
	stdi	gr22, @(gr8,24)
	stdi	gr24, @(gr8,32)
	stdi	gr26, @(gr8,40)
	stdi	gr28, @(gr8,48)
	stdi	gr30, @(gr8,56)
#if __FRV_GPR__ != 32
	stdi	gr48, @(gr8,64)
	stdi	gr50, @(gr8,72)
	stdi	gr52, @(gr8,80)
	stdi	gr54, @(gr8,88)
	stdi	gr56, @(gr8,96)
	stdi	gr58, @(gr8,104)
	stdi	gr60, @(gr8,112)
	stdi	gr62, @(gr8,120)
#endif

#if __FRV_FPR__ != 0
	stdfi	fr16, @(gr8,128)
	stdfi	fr18, @(gr8,136)
	stdfi 	fr20, @(gr8,144)
	stdfi	fr22, @(gr8,152)
	stdfi	fr24, @(gr8,160)
	stdfi	fr26, @(gr8,168)
	stdfi	fr28, @(gr8,176)
	stdfi	fr30, @(gr8,184)
#if __FRV_FPR__ != 32
	stdfi	fr48, @(gr8,192)
	stdfi	fr50, @(gr8,200)
	stdfi	fr52, @(gr8,208)
	stdfi	fr54, @(gr8,216)
	stdfi	fr56, @(gr8,224)
	stdfi	fr58, @(gr8,232)
	stdfi	fr60, @(gr8,240)
	stdfi	fr62, @(gr8,248)
#endif
#endif

        movsg	lr, gr4
	sti     gr4, @(gr8,256)
	sti     sp, @(gr8,260)
	sti     fp, @(gr8,264)
	
	sethi.p	#gotofffuncdeschi(__sigjmp_save), gr4
	setlo	#gotofffuncdesclo(__sigjmp_save), gr4
	ldd	@(gr15, gr4), gr14
	jmpl	@(gr14, gr0)
	.size	__sigsetjmp,.-__sigsetjmp
