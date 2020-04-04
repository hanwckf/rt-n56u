/*
 * Copyright (C) 2016-2017 Andes Technology, Inc.
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sysdep.h>
!==========================================================
!  void *memcpy(void *dst, const void *src, int n);
!
!        dst: $r0
!        src: $r1
!        n  : $r2
!        ret: $r0 - pointer to the memory area dst.
!==========================================================
.weak memcpy
ENTRY(memcpy)
	move    $r5, $r0
	beq     $r0, $r1, .Lquit_memcpy
	beqz    $r2, .Lquit_memcpy
	srli	$r3, $r2, #5			! check if len < cache-line size 32
	beqz	$r3, .Lword_copy_entry
	andi	$r4, $r0, #0x3			! check byte-align
	beqz	$r4, .Lunalign_word_copy_entry

	addi    $r4, $r4, #-4
	abs		$r4, $r4				! check how many un-align byte to copy
	sub		$r2, $r2, $r4			! update $R2

.Lunalign_byte_copy:
	lbi.bi	$r3, [$r1], #1
	addi	$r4, $r4, #-1
	sbi.bi	$r3, [$r0], #1
	bnez	$r4, .Lunalign_byte_copy
	beqz	$r2, .Lquit_memcpy

.Lunalign_word_copy_entry:
	andi	$r3, $r0, 0x1f			! check cache-line unaligncount
	beqz	$r3, .Lcache_copy

	addi	$r3, $r3, #-32
	abs		$r3, $r3
	sub		$r2, $r2, $r3			! update $R2

.Lunalign_word_copy:
	lmw.bim	$r4, [$r1], $r4
	addi	$r3, $r3, #-4
	smw.bim	$r4, [$r0], $r4
	bnez	$r3, .Lunalign_word_copy
	beqz	$r2, .Lquit_memcpy

	addi	$r3, $r2, #-32			! to check $r2 < cache_line, than go to .Lword_copy
	bltz	$r3, .Lword_copy_entry
.Lcache_copy:
	srli	$r3, $r2, #5
	beqz	$r3, .Lword_copy_entry
	pushm	$r6, $r13
        cfi_adjust_cfa_offset(32)
        cfi_rel_offset(r6, 0)
        cfi_rel_offset(r7, 4)
        cfi_rel_offset(r8, 8)
        cfi_rel_offset(r9, 12)
        cfi_rel_offset(r10, 16)
        cfi_rel_offset(r11, 20)
        cfi_rel_offset(r12, 24)
        cfi_rel_offset(r13, 28)

.L3:
	lmw.bim	$r6, [$r1], $r13
	addi	$r3, $r3, #-1
	smw.bim	$r6, [$r0], $r13
	bnez	$r3, .L3
	popm	$r6, $r13
        cfi_adjust_cfa_offset(-32)
        cfi_restore(r6)
        cfi_restore(r7)
        cfi_restore(r8)
        cfi_restore(r9)
        cfi_restore(r10)
        cfi_restore(r11)
        cfi_restore(r12)
        cfi_restore(r13)


.Lword_copy_entry:
	andi	$r2, $r2, #31
	beqz	$r2, .Lquit_memcpy
	srli	$r3, $r2, #2
	beqz	$r3, .Lbyte_copy
.Lword_copy:
	lmw.bim	$r4, [$r1], $r4
	addi	$r3, $r3, #-1
	smw.bim	$r4, [$r0], $r4
	bnez	$r3, .Lword_copy
	andi	$r2, $r2, #3
	beqz	$r2, .Lquit_memcpy

.Lbyte_copy:
	lbi.bi	$r3, [$r1], #1
	addi	$r2, $r2, #-1
	sbi.bi	$r3, [$r0], #1
	bnez	$r2, .Lbyte_copy

.Lquit_memcpy:
	move	$r0, $r5
	ret

END(memcpy)
libc_hidden_def(memcpy)
