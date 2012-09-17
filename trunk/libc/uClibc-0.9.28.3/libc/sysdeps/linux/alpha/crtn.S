	.set noat
	.set noreorder
	.set nomacro
	.set	macro
	
	.section .init
	.set	nomacro
	.align 2
	.globl _init
	.ent _init
	.set	nomacro
	ldq $26,0($30)
	lda $30,16($30)
	ret $31,($26),1
	.end _init
	.set	macro
	
	.section .fini
	.set	nomacro
	.align 2
	.globl _fini
	.ent _fini
	.set	nomacro
	ldq $26,0($30)
	lda $30,16($30)
	ret $31,($26),1
	.end _fini
	.set	macro
	
	.ident	"GCC: (GNU) 3.3.2"
