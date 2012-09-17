	.set noat
	.set noreorder
	.set nomacro
	.set	macro
	
	.section .init
	.set	nomacro
	.align 2
	.globl _init
	.ent _init
_init:
	.frame $30,0,$26,0
	.mask 0x4000000,0
	ldah $29,0($27)		!gpdisp!3
	lda $29,0($29)		!gpdisp!3
$_init..ng:
	lda $30,-16($30)
	stq $26,0($30)
	.prologue 1
	.set	macro
	
	.align 2
	.end _init
	
	.section .fini
	.set	nomacro
	.align 2
	.globl _fini
	.ent _fini
_fini:
	.frame $30,0,$26,0
	.mask 0x4000000,0
	ldah $29,0($27)		!gpdisp!6
	lda $29,0($29)		!gpdisp!6
$_fini..ng:
	lda $30,-16($30)
	stq $26,0($30)
	.prologue 1
	.set	macro
	.align 2
	.end _fini
	
	.ident	"GCC: (GNU) 3.3.2"
