	.file	"initfini.c"
	
	.section .init
	.align 4
	.global _init
	.type	_init, %function
	.proc	020
	ret
	restore
	
	.section .fini
	.align 4
	.global _fini
	.type	_fini, %function
	.proc	020
	ret
	restore
