	.section .init
	.align 2
	.globl _init
	ldq $26,0($30)
	lda $30,16($30)
	ret $31,($26),1

	.section .fini
	.align 2
	.globl _fini
	ldq $26,0($30)
	lda $30,16($30)
	ret $31,($26),1
