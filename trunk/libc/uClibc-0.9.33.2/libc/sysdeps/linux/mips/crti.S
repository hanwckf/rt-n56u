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
	.ent	_init
	.type	_init, @function
_init:
	.frame	$sp,32,$31		# vars= 0, regs= 2/0, args= 16, extra= 8
	.mask	0x90000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.cpload	$25
	.set	reorder
	subu	$sp,$sp,32
	.cprestore 16
	sw	$31,28($sp)
	sw	$28,24($sp)
#APP
	
	.align 2
	.end _init
	
	.section .fini
#NO_APP
	.align	2
	.globl	_fini
	.ent	_fini
	.type	_fini, @function
_fini:
	.frame	$sp,32,$31		# vars= 0, regs= 2/0, args= 16, extra= 8
	.mask	0x90000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.cpload	$25
	.set	reorder
	subu	$sp,$sp,32
	.cprestore 16
	sw	$31,28($sp)
	sw	$28,24($sp)
#APP
	.align 2
	.end _fini
	
	.ident	"GCC: (GNU) 3.3.2"
#elif _MIPS_SIM == _MIPS_SIM_NABI32
	.file	1 "initfini.c"
	.section .mdebug.abiN32
	.previous
	.abicalls
#APP
	
	.section .init
#NO_APP
	.align	2
	.align	3
	.globl	_init
	.ent	_init
	.type	_init, @function
_init:
	.frame	$sp,16,$31		# vars= 0, regs= 2/0, args= 0, gp= 0
	.mask	0x90000000,-8
	.fmask	0x00000000,0
	addiu	$sp,$sp,-16
	sd	$28,0($sp)
	lui	$28,%hi(%neg(%gp_rel(_init)))
	addu	$28,$28,$25
	addiu	$28,$28,%lo(%neg(%gp_rel(_init)))
	sd	$31,8($sp)
#APP
	
	.align 3
	.end _init

	.section .fini
#NO_APP
	.align	2
	.align	3
	.globl	_fini
	.ent	_fini
	.type	_fini, @function
_fini:
	.frame	$sp,16,$31		# vars= 0, regs= 2/0, args= 0, gp= 0
	.mask	0x90000000,-8
	.fmask	0x00000000,0
	addiu	$sp,$sp,-16
	sd	$28,0($sp)
	lui	$28,%hi(%neg(%gp_rel(_fini)))
	addu	$28,$28,$25
	addiu	$28,$28,%lo(%neg(%gp_rel(_fini)))
	sd	$31,8($sp)
#APP
	.align 3
	.end _fini
	
	.ident	"GCC: (GNU) 3.4.3"
#else	/* N64 */
	.file	1 "initfini.c"
	.section .mdebug.abi64
	.previous
	.abicalls
#APP
	
	
	.section .init
#NO_APP
	.align	2
	.globl	_init
	.ent	_init
	.type	_init, @function
_init:
	.frame	$sp,32,$31		# vars= 0, regs= 2/0, args= 0, extra= 16
	.mask	0x90000000,-8
	.fmask	0x00000000,0
	dsubu	$sp,$sp,32
	sd	$31,24($sp)
	sd	$28,16($sp)
	.set	noat
	lui	$1,%hi(%neg(%gp_rel(_init)))
	addiu	$1,$1,%lo(%neg(%gp_rel(_init)))
	daddu	$gp,$1,$25
	.set	at
#APP
	
	.align 2
	.end _init
	
	.section .fini
#NO_APP
	.align	2
	.globl	_fini
	.ent	_fini
	.type	_fini, @function
_fini:
	.frame	$sp,32,$31		# vars= 0, regs= 2/0, args= 0, extra= 16
	.mask	0x90000000,-8
	.fmask	0x00000000,0
	dsubu	$sp,$sp,32
	sd	$31,24($sp)
	sd	$28,16($sp)
	.set	noat
	lui	$1,%hi(%neg(%gp_rel(_fini)))
	addiu	$1,$1,%lo(%neg(%gp_rel(_fini)))
	daddu	$gp,$1,$25
	.set	at
#APP
	.align 2
	.end _fini
	
	.ident	"GCC: (GNU) 3.3.2"
#endif	/* N64 */
