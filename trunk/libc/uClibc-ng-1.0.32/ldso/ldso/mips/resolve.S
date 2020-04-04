/*
 * Linux dynamic resolving code for MIPS. Fixes up the GOT entry as
 * indicated in register t8 and jumps to the resolved address. Shamelessly
 * ripped from 'sysdeps/mips/dl-machine.h' in glibc-2.2.5.
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License.  See the file "COPYING.LIB" in the main directory of this
 * archive for more details.
 *
 * Copyright (C) 1996-2001 Kazumoto Kojima <kkojima@info.kanagawa-u.ac.jp>
 * Copyright (C) 2002 Steven J. Hill <sjhill@realitydiluted.com>
 *
 */

#include <sgidefs.h>
.text
.align	2
.globl	_dl_runtime_resolve
.type	_dl_runtime_resolve,@function
.ent	_dl_runtime_resolve
_dl_runtime_resolve:
#if _MIPS_SIM == _MIPS_SIM_ABI32
	.frame	$29, 40, $31
	.set noreorder

	/* Save GP. */
	move	$3, $28

	/* Save arguments and sp value on stack. */
	subu	$29, 40

	/* Modify t9 ($25) so as to point .cpload instruction. */
	addiu	$25, 12

	/* Compute GP. */
	.set noreorder
	.cpload $25
	.set reorder

	/* Save slot call pc. */
	move	$2, $31
	.cprestore 32

	/* Store function arguments from registers to stack */
	sw	$15, 36($29)
	sw	$4, 16($29)
	sw	$5, 20($29)
	sw	$6, 24($29)
	sw	$7, 28($29)

	/* Setup functions args and call __dl_runtime_resolve */
	move	$4, $24
	move	$5, $3
	jal	__dl_runtime_resolve

	/* Restore function arguments from stack to registers */
	lw	$31, 36($29)
	lw	$4, 16($29)
	lw	$5, 20($29)
	lw	$6, 24($29)
	lw	$7, 28($29)

	/* Do a tail call to the original function */
	addiu	$29, 40
#else	/* N32 || N64 */
	.set noreorder

	/* Save GP. */
	move	$3, $28

	/* Save arguments and sp value on stack. */
	dsubu	$29, 80

	/* Compute GP. */
	.set noreorder
	.cpsetup	$25, 0, _dl_runtime_resolve
	.set reorder

	/* Store function arguments from registers to stack */
	sd	$15, 72($29)
	sd	$4, 8($29)
	sd	$5, 16($29)
	sd	$6, 24($29)
	sd	$7, 32($29)
	sd	$8, 40($29)
	sd	$9, 48($29)
	sd	$10, 56($29)
	sd	$11, 64($29)

	/* Setup functions args and call __dl_runtime_resolve */
	move	$4, $24
	move	$5, $3
	jal	__dl_runtime_resolve

	/* Restore function arguments from stack to registers */
	ld	$31, 72($29)
	ld	$4, 8($29)
	ld	$5, 16($29)
	ld	$6, 24($29)
	ld	$7, 32($29)
	ld	$8, 40($29)
	ld	$9, 48($29)
	ld	$10, 56($29)
	ld	$11, 64($29)

	/* Do a tail call to the original function */
	.cpreturn
	daddu	$29, 80
#endif	/* N32 || N64 */
	move	$25, $2
	jr	$25
.end	_dl_runtime_resolve
.previous

/* Assembler veneer called from the PLT header code when using the
   non-PIC ABI.

   Code in each PLT entry puts the caller's return address into t7 ($15),
   the PLT entry index into t8 ($24), the address of _dl_runtime_pltresolve
   into t9 ($25) and the address of .got.plt into gp ($28).  __dl_runtime_pltresolve
   needs a0 ($4) to hold the link map and a1 ($5) to hold the index into
   .rel.plt (== PLT entry index * 4).  */

	.text
	.align	2
	.globl	_dl_runtime_pltresolve
	.type	_dl_runtime_pltresolve,@function
	.ent	_dl_runtime_pltresolve
_dl_runtime_pltresolve:
	.frame	$29, 40, $31
	.set noreorder
	# Save arguments and sp value in stack.
	subu    $29, 40
	lw      $10, 4($28)
	# Modify t9 ($25) so as to point .cpload instruction.
	addiu   $25, 12
	# Compute GP.
	.cpload $25
	.set reorder

	/* Store function arguments from registers to stack */
	sw	$15, 36($29)
	sw	$4, 16($29)
	sw	$5, 20($29)
	sw	$6, 24($29)
	sw	$7, 28($29)

	/* Setup functions args and call __dl_runtime_pltresolve.  */
	move	$4, $10
	sll     $5, $24, 3
	jal	__dl_runtime_pltresolve

	/* Restore function arguments from stack to registers */
	lw	$31, 36($29)
	lw	$4, 16($29)
	lw	$5, 20($29)
	lw	$6, 24($29)
	lw	$7, 28($29)

	/* Do a tail call to the original function */
	addiu	$29, 40
	move	$25, $2
	jr	$25
	.end	_dl_runtime_pltresolve
	.previous
