	.file	"initfini.c"
#APP
	
	.section .init
#NO_APP
	.align 1
	.global __init
	.type	__init, @function
#NO_APP
	jr __return_r31
#APP
	
	.section .fini
#NO_APP
	.align 1
	.global __fini
	.type	__fini, @function
#NO_APP
	jr __return_r31
#APP
	
	.ident	"GCC: (GNU) 3.3.2"
