	.file	"crtn.S"
	.text
	
	.section .init
	.hidden  _init
	.align 1
	.global	_init
	.type	_init, @function
	mov	r14,r15
	lds.l	@r15+,pr
	mov.l	@r15+,r14
	rts	
	mov.l	@r15+,r12
.L8:
	.align 2
.L6:
.L7:
	
	.section .fini
	.hidden  _fini
	.align 1
	.global	_fini
	.type	_fini, @function
	mov	r14,r15
	lds.l	@r15+,pr
	mov.l	@r15+,r14
	rts	
	mov.l	@r15+,r12
.L13:
	.align 2
.L11:
.L12:
	
	.ident	"GCC: (GNU) 3.3.2"
