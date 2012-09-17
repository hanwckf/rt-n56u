	.file	"initfini.c"
#APP
	
	.section .init
#NO_APP
	.align	2
	.globl	_init
	.type	_init, @function
_init:
	link.w %a6,#0
#APP
	
	.align 2
	
	
	.section .fini
#NO_APP
	.align	2
	.globl	_fini
	.type	_fini, @function
_fini:
	link.w %a6,#0
#APP
	.align 2
	
	
	.ident	"GCC: (GNU) 3.3.2"
