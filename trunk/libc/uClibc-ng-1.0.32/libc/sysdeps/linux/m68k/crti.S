#include "m68k_pic.S"

	.section .init
	.align	2
	.globl	_init
	.type	_init, @function
_init:
	link.w	%a6,#0
	INIT_GP
	
	.section .fini
	.align	2
	.globl	_fini
	.type	_fini, @function
_fini:
	link.w	%a6,#0
	INIT_GP
