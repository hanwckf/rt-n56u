	.file	"initfini.c"
#APP
	
	.section .init
#NO_APP
	.align 2
	.globl _init
	.type	_init, @function
#NO_APP
	lwz 0,36(1)
	addi 1,1,32
	mtlr 0
	blr
#APP
	
	.section .fini
#NO_APP
	.align 2
	.globl _fini
	.type	_fini, @function
#NO_APP
	lwz 0,36(1)
	addi 1,1,32
	mtlr 0
	blr
#APP
	
	.ident	"GCC: (GNU) 3.3.1 20030626 (Debian prerelease)"
