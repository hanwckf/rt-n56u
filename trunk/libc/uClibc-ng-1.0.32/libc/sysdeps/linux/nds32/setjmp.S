/*
 * Copyright (C) 2016 Andes Technology, Inc.
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

ENTRY(__sigsetjmp)
	move	$r2, $r0
.off_16bit
	! save registers into buffer
	smw.bim	$r6, [$r2], $r14, #0xf
.restore_16bit

#ifdef NDS32_ABI_2FP_PLUS
/* Process for FPU registers.  */
	fmfcfg	$r20	/* Keep $fpcfg in $r20.  */
	slli	$r20, $r20, #28
	srli	$r20, $r20, #30	/* Set $r20 as $fpcfg.freg.  */
	swi.bi	$r20, [$r2], #4

	/* Case switch for $r20 as $fpcfg.freg.  */
	beqz	$r20, .LCFG0		/* Branch if $fpcfg.freg = 0b00.  */
	xori	$r15, $r20, #0b10
	beqz	$r15, .LCFG2		/* Branch if $fpcfg.freg = 0b10.  */
	srli	$r20, $r20, #0b01
	beqz	$r20, .LCFG1		/* Branch if $fpcfg.freg = 0b01.  */
	/* Fall-through if $fpcfg.freg = 0b11.  */
.LCFG3:
	fsdi.bi	$fd31, [$r2], #8
	fsdi.bi	$fd30, [$r2], #8
	fsdi.bi	$fd29, [$r2], #8
	fsdi.bi	$fd28, [$r2], #8
	fsdi.bi	$fd27, [$r2], #8
	fsdi.bi	$fd26, [$r2], #8
	fsdi.bi	$fd25, [$r2], #8
	fsdi.bi	$fd24, [$r2], #8
.LCFG2:
	fsdi.bi	$fd10, [$r2], #8
	fsdi.bi	$fd9, [$r2], #8
	fsdi.bi	$fd8, [$r2], #8
.LCFG1:
	fsdi.bi	$fd7, [$r2], #8
	fsdi.bi	$fd6, [$r2], #8
	fsdi.bi	$fd5, [$r2], #8
	fsdi.bi	$fd4, [$r2], #8
.LCFG0:
	fsdi.bi	$fd3, [$r2], #8
#endif /* NDS32_ABI_2FP_PLUS */


/* Make a tail call to __sigjmp_save.  */
#ifdef PIC
	/* Initialize $r2 as $gp value.  */
	sethi   $r2, hi20(_GLOBAL_OFFSET_TABLE_-8)
	ori     $r2, $r2, lo12(_GLOBAL_OFFSET_TABLE_-4)
	mfusr   $r15, $pc
	add     $r2, $r15, $r2

	! la    $r3, __sigjmp_save@PLT
	sethi   $r3, hi20(__sigjmp_save@PLT)
	ori     $r3, $r3, lo12(__sigjmp_save@PLT)
	add     $r3, $r3, $r2

	jr      $r3
#else /* NOT PIC */
	la	$r15, C_SYMBOL_NAME(__sigjmp_save)
	jr	$r15
#endif

END(__sigsetjmp)
libc_hidden_def(__sigsetjmp)
