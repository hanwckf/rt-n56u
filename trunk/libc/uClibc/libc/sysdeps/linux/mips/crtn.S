	.file	1 "initfini.c"
	.section .mdebug.abi32
	.previous
	.abicalls
#APP
	
	.section .init
#NO_APP
	.align	2
	.globl	_init
	.ent	_init
	.type	_init, @function
#NO_APP
	lw	$31,28($sp)
	#nop
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,32
	.set	macro
	.set	reorder

	.end	_init
#APP
	
	.section .fini
#NO_APP
	.align	2
	.globl	_fini
	.ent	_fini
	.type	_fini, @function
#NO_APP
	lw	$31,28($sp)
	#nop
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,32
	.set	macro
	.set	reorder

	.end	_fini
#APP
	
	.ident	"GCC: (GNU) 3.3.2"
