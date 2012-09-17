	.file	"initfini.c"
	
	.section .init
	.align 2
	.globl _init
	.type	_init, @function

	lwz 0,36(1)
	addi 1,1,32
	mtlr 0
	blr
	
	.section .fini
	.align 2
	.globl _fini
	.type	_fini, @function

	lwz 0,36(1)
	addi 1,1,32
	mtlr 0
	blr
