	.file	"initfini.c"
#APP
	
	.section .init
#NO_APP
	.align 1
	.global __init
	.type	__init, @function
__init:
	jarl __save_r31, r10
#APP
	
	.align 1
	
	
	.section .fini
#NO_APP
	.align 1
	.global __fini
	.type	__fini, @function
__fini:
	jarl __save_r31, r10
#APP
	.align 1
	
	
	.ident	"GCC: (GNU) 3.3.2"
