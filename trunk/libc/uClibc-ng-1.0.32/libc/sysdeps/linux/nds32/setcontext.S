/*
 * Copyright (C) 2016-2017 Andes Technology, Inc.
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sysdep.h>

#include "ucontext_i.h"

ENTRY(__setcontext)
        move    $r4, $r0

#ifdef __NDS32_ABI_2FP_PLUS__
	addi	$r0,  $r4, UCONTEXT_FDREGS

	/* Case switch for $r20 as $fpcfg.freg.  */
	beqz    $r20, .LCFG0         /* Branch if $fpcfg.freg = 0b00.  */
	xori    $r15, $r20, #0b10
	beqz    $r15, .LCFG2         /* Branch if $fpcfg.freg = 0b10.  */
	srli    $r20, $r20, #0b01
	beqz    $r20, .LCFG1         /* Branch if $fpcfg.freg = 0b01.  */
	/* Fall-through if $fpcfg.freg = 0b11.  */
.LCFG3:
	fldi $fd31, [$r0 + 248]
	fldi $fd30, [$r0 + 240]
	fldi $fd29, [$r0 + 232]
	fldi $fd28, [$r0 + 224]
	fldi $fd27, [$r0 + 216]
	fldi $fd26, [$r0 + 208]
	fldi $fd25, [$r0 + 200]
	fldi $fd24, [$r0 + 192]
.LCFG2:
	fldi $fd10, [$r0 + 80]
	fldi $fd9, [$r0 + 72]
	fldi $fd8, [$r0 + 64]
.LCFG1:
	fldi $fd7, [$r0 + 56]
	fldi $fd6, [$r0 + 48]
	fldi $fd5, [$r0 + 40]
	fldi $fd4, [$r0 + 32]
.LCFG0:
	fldi $fd3, [$r0 + 24]
	/*save fpcsr*/
	lwi $r1, [$r0 + 0x100]
	fmtcsr $r1
#endif /* __NDS32_ABI_2FP_PLUS__ */

	/* sigprocmask (SIG_BLOCK, &sc->sc_mask, NULL).  */
	move    $r0, SIG_SETMASK
	addi	$r1, $r4, UCONTEXT_SIGMASK
	move	$r2, 0
	move	$r3, _NSIG8
	syscall SYS_ify(rt_sigprocmask)
        bnez    $r0, 1f

        move    $r0, $r4
        addi    $r15, $r4, UCONTEXT_GREGS + 4
	lmw.bim	$r1, [$r15], $r14
        addi    $r15, $r15, 4
	lmw.bim	$r16, [$r15], $r25, #0xf
        lwi     $r15, [$r0 + UCONTEXT_PC]
        push    $r1
	cfi_adjust_cfa_offset (4)
        move    $r1, $r0
        lwi     $r0, [$r1 + UCONTEXT_GREGS]
        pop     $r1
	cfi_adjust_cfa_offset (-4)
        jr      $r15
1:
	move $r0, -1
	ret
END(__setcontext)

weak_alias (__setcontext, setcontext)
libc_hidden_def(__setcontext)

ENTRY (__startcontext)
        move $r0, $r6
	beqz $r0, 1f
	jal HIDDEN_JUMPTARGET(__setcontext)
1:	
	move $r0, 0
	j HIDDEN_JUMPTARGET(exit)
END (__startcontext)

