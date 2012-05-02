#NO_APP
	.file	"initfini.c"
#APP
	
	.section .init
#NO_APP
	.align 1
	.global _init
	.type	_init, @function
_init:
	Push $srp
	subq 4,$sp
	movem $r0,[$sp]
	move.d $pc,$r0
	sub.d .:GOTOFF,$r0
#APP
	
	.align 1
	
	
	.section .fini
#NO_APP
	.align 1
	.global _fini
	.type	_fini, @function
_fini:
	Push $srp
	subq 4,$sp
	movem $r0,[$sp]
	move.d $pc,$r0
	sub.d .:GOTOFF,$r0
#APP
	.align 1
	
	
/*@TRAILER_BEGINS*/
