/*
 * libc/sysdeps/linux/microblaze/vfork.S -- `vfork' syscall for linux/microblaze
 *
 * Copyright (C) 2003  John Williams <jwilliams@itee.uq.edu.au>
 * Copyright (C) 2001  NEC Corporation
 * Copyright (C) 2001  Miles Bader <miles@gnu.org>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License.  See the file COPYING.LIB in the main
 * directory of this archive for more details.
 *
 * Written by Miles Bader <miles@gnu.org>
 * Microblaze port by John Williams
 */

#define _ERRNO_H	1
#include <bits/errno.h>
#define _SYSCALL_H
#include <bits/sysnum.h>

#include <libc-symbols.h>

/* Clone the calling process, but without copying the whole address space.
   The calling process is suspended until the new process exits or is
   replaced by a call to `execve'.  Return -1 for errors, 0 to the new process,
   and the process ID of the new process to the old process.  */

.global C_SYMBOL_NAME(errno)

	.globl __vfork
	.align 4
__vfork:
	addi	r12, r0, SYS_vfork
	brki	r14, 0x08;
	addi	r4, r3, 125		/* minimum err value */
	blti	r4, 1f			/* is r3 < -125? */
	bri	2f			/* normal return */
1:	sub 	r3, r3, r0		/* r3 = -r3 */
#ifdef __PIC__
	mfs	r3,rpc
	addik	r3,r3,_GLOBAL_OFFSET_TABLE_+8
	lwi	r3,r3,C_SYMBOL_NAME(errno)@GOT
	sw	r3, r0, r3
#else
	swi	r3, r0, C_SYMBOL_NAME(errno);
#endif
					/* state restore etc */
2:	rtsd	r15, 8			/* error return */
	nop
       .size   __vfork, .-__vfork

weak_alias(__vfork,vfork)
libc_hidden_weak(vfork)
