	.file	"initfini.c"
	.section	.text..SHmedia32,"ax"
	.little
	
	.section .init
	.hidden  _init
	.align 2
	.global	_init
	.type	_init, @function
	add.l	r14, r63, r15
	ld.l	r15, 12, r18
	ld.l	r15, 4, r12
	ld.l	r15, 8, r14
	ptabs	r18, tr0
	addi.l	r15, 16, r15
	blink	tr0, r63
	
	.section .fini
	.hidden  _fini
	.align 2
	.global	_fini
	.type	_fini, @function
	add.l	r14, r63, r15
	ld.l	r15, 12, r18
	ld.l	r15, 4, r12
	ld.l	r15, 8, r14
	ptabs	r18, tr0
	addi.l	r15, 16, r15
	blink	tr0, r63
	
	.ident	"GCC: (GNU) 3.3.2"
