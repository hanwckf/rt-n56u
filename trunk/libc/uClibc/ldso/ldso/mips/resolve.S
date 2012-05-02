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

.text
.align	2
.globl	_dl_runtime_resolve
.type	_dl_runtime_resolve,@function
.ent	_dl_runtime_resolve
_dl_runtime_resolve:
	.frame	$29, 40, $31
	.set noreorder

	# Save GP.
	move	$3, $28

	# Save arguments and sp value on stack.
	subu	$29, 40

	# Modify t9 ($25) so as to point .cpload instruction.
	addiu	$25, 12

	# Compute GP.
	.set noreorder
	.cpload $25
	.set reorder

	# Save slot call pc.
	move	$2, $31
	.cprestore 32

	# Store function arguments from registers to stack
	sw	$15, 36($29)
	sw	$4, 16($29)
	sw	$5, 20($29)
	sw	$6, 24($29)
	sw	$7, 28($29)

	# Setup functions args and call __dl_runtime_resolve
	move	$4, $24
	move	$5, $3
	jal	__dl_runtime_resolve

	# Restore function arguments from stack to registers
	lw	$31, 36($29)
	lw	$4, 16($29)
	lw	$5, 20($29)
	lw	$6, 24($29)
	lw	$7, 28($29)

	# Do a tail call to the original function
	addiu	$29, 40
	move	$25, $2
	jr	$25
.end	_dl_runtime_resolve
.previous

