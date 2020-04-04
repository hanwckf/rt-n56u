	.file	"crti.S"
	.text
	
	.section .init
	.hidden  _init
	.align 1
	.global	_init
	.type	_init, @function
_init:
	mov.l	r12,@-r15
	mov.l	r14,@-r15
	sts.l	pr,@-r15
	mov	r15,r14
	bra	1f
	 nop
	.align 2
1:
	
	.section .fini
	.hidden  _fini
	.align 1
	.global	_fini
	.type	_fini, @function
_fini:
	mov.l	r12,@-r15
	mov.l	r14,@-r15
	sts.l	pr,@-r15
	mov	r15,r14
	bra	1f
	 nop
	.align 2
1:
