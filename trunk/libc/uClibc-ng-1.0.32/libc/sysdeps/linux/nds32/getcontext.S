/*
 * Copyright (C) 2016-2017 Andes Technology, Inc.
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/* Copyright (C) 20[B01-2013 Free Software Foundation, Inc.
   Contributed by Martin Schwidefsky (schwidefsky@de.ibm.com).

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>

#include "ucontext_i.h"

/*  __getcontext (const ucontext_t *ucp)

  Saves the machine context in UCP such that when it is activated,
  it appears as if __getcontext() returned again.

  This implementation is intended to be used for *synchronous* context
  switches only.  Therefore, it does not have to save anything
  other than the PRESERVED state.  */

ENTRY(__getcontext)
        swi     $lp, [$r0 + UCONTEXT_PC]
        addi    $r15, $r0, UCONTEXT_GREGS
        xor     $r1, $r1, $r1
	smw.bim	$r1, [$r15], $r1
	smw.bim	$r1, [$r15], $r14
        addi    $r15, $r15, 4
	smw.bim	$r16, [$r15], $r25, #0xf
	move    $r4, $r0

	/* sigprocmask (SIG_BLOCK, NULL, &sc->sc_mask).  */
	move    $r0, SIG_BLOCK
	move	$r1, 0
	addi	$r2, $r4, UCONTEXT_SIGMASK
	move	$r3, _NSIG8
	syscall SYS_ify(rt_sigprocmask)
        bnez    $r0, 1f


#ifdef __NDS32_ABI_2FP_PLUS__
	addi	$r2, $r4, UCONTEXT_FDREGS
/* Process for FPU registers.  */
	fmfcfg	$r20	/* Keep $fpcfg in $r20.  */
	slli	$r20, $r20, #28
	srli	$r20, $r20, #30	/* Set $r20 as $fpcfg.freg.  */

	/* Case switch for $r20 as $fpcfg.freg.  */
	beqz	$r20, .LCFG0		/* Branch if $fpcfg.freg = 0b00.  */
	xori	$r15, $r20, #0b10
	beqz	$r15, .LCFG2		/* Branch if $fpcfg.freg = 0b10.  */
	srli	$r20, $r20, #0b01
	beqz	$r20, .LCFG1		/* Branch if $fpcfg.freg = 0b01.  */
	/* Fall-through if $fpcfg.freg = 0b11.  */
.LCFG3:
	fsdi $fd31, [$r2 + 248]
	fsdi $fd30, [$r2 + 240]
	fsdi $fd29, [$r2 + 232]
	fsdi $fd28, [$r2 + 224]
	fsdi $fd27, [$r2 + 216]
	fsdi $fd26, [$r2 + 208]
	fsdi $fd25, [$r2 + 200]
	fsdi $fd24, [$r2 + 192]
.LCFG2:
	fsdi $fd10, [$r2 + 80]
	fsdi $fd9, [$r2 + 72]
	fsdi $fd8, [$r2 + 64]
.LCFG1:
	fsdi $fd7, [$r2 + 56]
	fsdi $fd6, [$r2 + 48]
	fsdi $fd5, [$r2 + 40]
	fsdi $fd4, [$r2 + 32]
.LCFG0:
	fsdi $fd3, [$r2 + 24]
	/*save fpcsr*/
	fmfcsr $r1
	swi $r1, [$r2 + 0x100]
#endif /* __NDS32_ABI_2FP_PLUS__ */

	/* Set __getcontext return value to 0.  */
        xor      $r0, $r0, $r0
        /* Return first_return: 1 */
        addi     $r1, $r0, 1
        ret

1:
	move $r0, -1
	ret
END(__getcontext)

weak_alias (__getcontext, getcontext)

