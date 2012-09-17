/*
 * libc/sysdeps/linux/nios/vfork.S -- `vfork' syscall for linux/nios
 *
 *  Copyright (C) 2004  Microtronix Datacom Ltd
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License.  See the file COPYING.LIB in the main
 * directory of this archive for more details.
 * 
 * Written by Wentao Xu <wentao@microtronix.com>
 */
#include <asm/unistd.h>
#include "NM_Macros.S"

#ifndef __NR_vfork
#define __NR_vfork __NR_fork /* uClinux-2.0 only has fork which is vfork */
#endif

   .text
   .align 2
   .globl errno
   .globl vfork
   .globl __libc_vfork
#if defined __HAVE_ELF__
   .type    vfork,@function
   .type    __libc_vfork,@function
#endif
vfork:
__libc_vfork:
	MOVIP	%g1, __NR_vfork
	trap	63
	
	bgen	%g1, 12
	not		%g1			/* (unsigned long) -4096 */
	cmp		%o0, %g1
	skps	cc_hi
 	  jmp	%o7
	  nop
	  
fix_errno:
	neg		%o0
	save	%sp, -16
	MOVIA	%g1, __errno_location@h
	call	%g1
	 nop
	st	[%o0], %i0		/* store errno */
	
	xor %i0, %i0
	subi %i0, 1		/* retval=-1 */
	ret
	 restore
	
		
	




