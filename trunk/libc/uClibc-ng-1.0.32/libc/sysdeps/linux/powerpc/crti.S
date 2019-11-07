	.file	"initfini.c"
	
	.section .init
	.align 2
	.globl _init
	.type	_init, @function
_init:
	stwu 1,-32(1)
	mflr 0
	stw 0,36(1)
	
	.align 2
	
	
	.section .fini
	.align 2
	.globl _fini
	.type	_fini, @function
_fini:
	stwu 1,-32(1)
	mflr 0
	stw 0,36(1)

	.align 2
