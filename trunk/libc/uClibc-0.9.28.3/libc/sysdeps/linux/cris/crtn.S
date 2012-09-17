#NO_APP
	.file	"initfini.c"
#APP
	
	.section .init
#NO_APP
	.align 1
	.global _init
	.type	_init, @function
#NO_APP
	movem [$sp+],$r0
	Jump [$sp+]
#APP
	
	.section .fini
#NO_APP
	.align 1
	.global _fini
	.type	_fini, @function
#NO_APP
	movem [$sp+],$r0
	Jump [$sp+]
#APP
	
/*@TRAILER_BEGINS*/
