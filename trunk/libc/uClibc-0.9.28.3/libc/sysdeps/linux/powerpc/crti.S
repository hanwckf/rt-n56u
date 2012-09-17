	.file	"initfini.c"
#APP
	
	.section .init
#NO_APP
	.align 2
	.globl _init
	.type	_init, @function
_init:
	stwu 1,-32(1)
	mflr 0
	stw 0,36(1)
#APP
	
	.align 2
	
	
	.section .fini
#NO_APP
	.align 2
	.globl _fini
	.type	_fini, @function
_fini:
	stwu 1,-32(1)
	mflr 0
	stw 0,36(1)
#APP
	.align 2
	
	
	.ident	"GCC: (GNU) 3.3.1 20030626 (Debian prerelease)"
