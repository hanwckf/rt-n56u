/*
 * Copyright (C) 2016-2017 Andes Technology, Inc.
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/*
  setjmp/longjmp for nds32.
  r0 - r5 are for paramter passing - no need to save
  r6 - r14 are callee saved - needs to save
  r15 is temp register for assembler - no need to save
  r16 - r25 are caller saved - no need to save
  r26 - r27 are temp registers for OS - no need to save
  r28 is fp - need to save
  r29 is gp - need to save
  r30 is ra - need to save
  r31 is sp - need to save
  so we need to save r6 - r14 and r28 - r31
  The jmpbuf looks like this:
  r6
  r7
  r8
  r9
  r10
  r11
  r12
  r13
  r14
  fp
  gp
  ra
  sp
#ifdef NDS32_ABI_2FP_PLUS
  ($fpcfg.freg)
  (callee-saved FPU regs)
#endif
  reserved(for 8-byte align if needed)
*/

#include <sysdep.h>
#define _SETJMP_H
#define _ASM
#include <bits/setjmp.h>
	.section .text

/* __longjmp (env[0].__jmpbuf, val ?: 1);  */
ENTRY(__longjmp)
	! restore registers
	lmw.bim  $r6, [$r0], $r14, #0xf

#ifdef NDS32_ABI_2FP_PLUS
	lwi.bi	$r20, [$r0], #4	/* Load $fpcfg.freg to $r20.  */

	/* Case switch for $r20 as $fpcfg.freg.  */
	beqz    $r20, .LCFG0         /* Branch if $fpcfg.freg = 0b00.  */
	xori    $r15, $r20, #0b10
	beqz    $r15, .LCFG2         /* Branch if $fpcfg.freg = 0b10.  */
	srli    $r20, $r20, #0b01
	beqz    $r20, .LCFG1         /* Branch if $fpcfg.freg = 0b01.  */
	/* Fall-through if $fpcfg.freg = 0b11.  */
.LCFG3:
	fldi.bi	$fd31, [$r0], #8
	fldi.bi	$fd30, [$r0], #8
	fldi.bi	$fd29, [$r0], #8
	fldi.bi	$fd28, [$r0], #8
	fldi.bi	$fd27, [$r0], #8
	fldi.bi	$fd26, [$r0], #8
	fldi.bi	$fd25, [$r0], #8
	fldi.bi	$fd24, [$r0], #8
.LCFG2:
	fldi.bi	$fd10, [$r0], #8
	fldi.bi	$fd9, [$r0], #8
	fldi.bi	$fd8, [$r0], #8
.LCFG1:
	fldi.bi	$fd7, [$r0], #8
	fldi.bi	$fd6, [$r0], #8
	fldi.bi	$fd5, [$r0], #8
	fldi.bi	$fd4, [$r0], #8
.LCFG0:
	fldi.bi	$fd3, [$r0], #8
#endif /* NDS32_ABI_2FP_PLUS */


	! return error code; make sure error code is not 0
	bnez  $r1, .Ldone
	movi  $r1, #1
.Ldone:
	addi  $r0, $r1, #0
	ret
END(__longjmp)
libc_hidden_def(__longjmp)
