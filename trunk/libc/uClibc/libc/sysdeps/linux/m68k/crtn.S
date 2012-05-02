	.file	"initfini.c"
#APP
	
	.section .init
#NO_APP
	.align	2
	.globl	_init
	.type	_init, @function
#NO_APP
	unlk %a6
	rts
#APP
	
	.section .fini
#NO_APP
	.align	2
	.globl	_fini
	.type	_fini, @function
#NO_APP
	unlk %a6
	rts
#APP
	
	.ident	"GCC: (GNU) 3.3.2"
