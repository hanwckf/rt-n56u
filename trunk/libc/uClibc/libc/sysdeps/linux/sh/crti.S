	.file	"crti.S"
	.text
	
	.section .init
	.hidden  _init
	.align 1
	.global	_init
	.type	_init, @function
_init:
	mov.l	r12,@-r15
	mova	.L6,r0
	mov.l	r14,@-r15
	sts.l	pr,@-r15
	mov.l	.L6,r12
	mov	r15,r14
	add	r0,r12
	
	
	
	bra	1f
	nop
	.align 2
.L6:
	.long	_GLOBAL_OFFSET_TABLE_
1:
	
	.section .fini
	.hidden  _fini
	.align 1
	.global	_fini
	.type	_fini, @function
_fini:
	mov.l	r12,@-r15
	mova	.L11,r0
	mov.l	r14,@-r15
	sts.l	pr,@-r15
	mov.l	.L11,r12
	mov	r15,r14
	add	r0,r12
	
	
	bra	1f
	nop
	.align 2
.L11:
	.long	_GLOBAL_OFFSET_TABLE_
1:
	
	.ident	"GCC: (GNU) 3.3.2"
