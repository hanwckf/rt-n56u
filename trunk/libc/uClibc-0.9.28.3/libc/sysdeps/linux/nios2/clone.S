/*
 * libc/sysdeps/linux/nios2/clone.S -- `clone' syscall for linux/nios2
 *
 *  Copyright (C) 2004  Microtronix Datacom Ltd
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License.  See the file COPYING.LIB in the main
 * directory of this archive for more details.
 * 
 * Written by Wentao Xu <wentao@microtronix.com>
 */
#include <asm/errno.h>
#include <asm/unistd.h>

/* int clone(int (*fn)(void *arg), void *child_stack, int flags, void *arg); */

.text
.balign 4
.type	clone,@function
.globl	clone;
clone:
.type	__clone,@function
.globl	__clone;
__clone:
	addi    sp,sp,-8
	mov     r8,r4
	stw     ra,4(sp)
	stw     r16,0(sp)

	mov     r4,r6
	movi    r2,-EINVAL
	
	/* sanity check */
	beq     r8,zero,CLONE_ERROR_LABEL
	beq     r5,zero,CLONE_ERROR_LABEL
	
	/* system call */
	movi    r2,TRAP_ID_SYSCALL
	movi    r3,__NR_clone
	trap
	
	/* child call the function */
	mov     r4,r7
	bne     r2,zero,CLONE_ERROR_LABEL
	callr   r8
	
	/* exit if it returns */
	mov     r4,r2
	movi    r3,__NR_exit
	trap
	
CLONE_ERROR_LABEL: 
	movi    r3,-4096
	sub     r16,zero,r2
	bgeu    r3,r2,CLONE_OK
	
	/* store errno */
	call    __errno_location
	stw     r16,0(r2)
	movi    r2,-1

CLONE_OK:
	ldw     ra,4(sp)
	ldw     r16,0(sp)
	addi    sp,sp,8
	ret
