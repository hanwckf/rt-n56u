#include "sgidefs.h"
#if _MIPS_SIM == _MIPS_SIM_ABI32
	.file	1 "initfini.c"
	.section .mdebug.abi32
	.previous
	.abicalls
#APP
	
	.section .init
#NO_APP
	.align	2
	.globl	_init
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

#APP
	
	.section .fini
#NO_APP
	.align	2
	.globl	_fini
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

#APP
	
	.ident	"GCC: (GNU) 3.3.2"
#elif _MIPS_SIM == _MIPS_SIM_NABI32
	.file	1 "initfini.c"
	.section .mdebug.abiN32
	.previous
	.abicalls
#APP
	
	.section .init
#NO_APP
	.align	3
	.globl	_init
	.type	_init, @function
#NO_APP
	ld	$31,8($sp)
	ld	$28,0($sp)
	.set	noreorder
	.set	nomacro
	j	$31
	addiu	$sp,$sp,16
	.set	macro
	.set	reorder
#APP
	
	.section .fini
#NO_APP
	.align	3
	.globl	_fini
	.type	_fini, @function
#NO_APP
	ld	$31,8($sp)
	ld	$28,0($sp)
	.set	noreorder
	.set	nomacro
	j	$31
	addiu	$sp,$sp,16
	.set	macro
	.set	reorder

#APP
	
	.ident	"GCC: (GNU) 3.4.3"
#else /* N64 */
	.file	1 "initfini.c"
	.section .mdebug.abi64
	.previous
	.abicalls
#APP
	
	
	.section .init
#NO_APP
	.align	2
	.globl	_init
	.type	_init, @function
#NO_APP
	ld	$31,24($sp)
	ld	$28,16($sp)
	#nop
	.set	noreorder
	.set	nomacro
	j	$31
	daddu	$sp,$sp,32
	.set	macro
	.set	reorder

#APP
	
	.section .fini
#NO_APP
	.align	2
	.globl	_fini
	.type	_fini, @function
#NO_APP
	ld	$31,24($sp)
	ld	$28,16($sp)
	#nop
	.set	noreorder
	.set	nomacro
	j	$31
	daddu	$sp,$sp,32
	.set	macro
	.set	reorder

#APP
	
	.ident	"GCC: (GNU) 3.3.2"
#endif	/* N64 */
