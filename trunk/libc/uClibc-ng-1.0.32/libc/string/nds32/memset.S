/*
 * Copyright (C) 2016-2017 Andes Technology, Inc.
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/*  Copyright (C) 1998, 2003 Free Software Foundation, Inc.
 *  Contributed by Philip Blundell <philb@gnu.org>
 *
 *  The GNU C Library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  The GNU C Library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with the GNU C Library; if not, write to the Free
 *  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307 USA.
 */

#include <features.h>
#include <sysdep.h>
!==========================================================
!  void *memset(void *dst, int val, int len);
!
!        dst: $r0
!        val: $r1
!        len: $r2
!        ret: $r0 - pointer to the memory area dst.
!==========================================================
.weak memset
ENTRY(memset)
	move	$r5, $r0	        ! Return value
	beqz    $r2, .Lend_memset   ! Exit when len = 0
	srli	$r3, $r2, 2	        ! r3 is how many words to copy
	andi	$r2, $r2, 3	        ! How many bytes are less than a word
	beqz	$r3, .Lbyte_set	    ! When n is less than a word

    ! set r1 from to abababab
	andi    $r1, $r1, 0x00ff        ! r1 = 000000ab
	slli    $r4, $r1, 8             ! r4 = 0000ab00
	or      $r1, $r1, $r4           ! r1 = 0000abab
	slli    $r4, $r1, 16            ! r4 = abab0000
	or      $r1, $r1, $r4           ! r1 = abababab

.Lword_set:
	addi	$r3, $r3, -1	        ! How many words left to copy
	smw.bim $r1, [$r5], $r1	    	! Copy the word to det
	bnez    $r3, .Lword_set			! Still words to set, continue looping
	beqz	$r2, .Lend_memset		! No left byte to set

.Lbyte_set:
	! Less than 4 bytes left to set
	addi	$r2, $r2, -1		! Decrease len by 1
	sbi.p	$r1, [$r5], 1		! Set data of the next byte to r1
	bnez	$r2, .Lbyte_set		! Still bytes left to set

.Lend_memset:
	ret

END(memset)
libc_hidden_def(memset)
