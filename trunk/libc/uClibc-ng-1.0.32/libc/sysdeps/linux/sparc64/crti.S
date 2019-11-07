	.file	"initfini.c"
	
	.section .init
	.align 4
	.global _init
	.type	_init, %function
	.proc	020
_init:
	!#PROLOGUE# 0
	save	%sp, -176, %sp
	
	.align 4
	
	
	.section .fini
	.align 4
	.global _fini
	.type	_fini, %function
	.proc	020
_fini:
	!#PROLOGUE# 0
	save	%sp, -176, %sp
	.align 4
