/* dummy _init and _fini */

	.section .init
	.globl	_init
	.align	2
	.type	_init,@function

_init:


	.section .fini
	.global	_fini
	.align	2
	.type	_fini,@function

_fini: 

